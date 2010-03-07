#include "MoveDeciders.h"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <utility>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <cassert>

static const int INF = INT_MAX;

static int heuristic(const Map &map);

class GameTree
{
    public:
        GameTree();
        ~GameTree();

        Direction decideMove(Map &map, int depth);

    private:
        struct Node;
        bool buildTreeTwoLevels(Node *node, const Map &map);
        int negamax(Node *node, Map &map, int depth,
                int alpha, int beta, int sign, Direction *dir);

        struct Node
        {
            Node *children[4];
            Direction dir;

            Node(Direction dir);
            void promoteMove(int i);
        };

        Node *root;

        std::deque<Node> nodesAlloc;
};

GameTree::GameTree()
{
    nodesAlloc.push_back(Node(NORTH));

    root = &nodesAlloc.back();
}

GameTree::~GameTree()
{
}

static inline Player signToPlayer(int sign)
{
    if (sign == 1)
        return SELF;
    else
        return ENEMY;
}

inline GameTree::Node::Node(Direction dir) :
    dir(dir)
{
    for (int i = 0; i < 4; ++i) {
        children[i] = NULL;
    }
}

inline void GameTree::Node::promoteMove(int i)
{
    for (; i > 0; --i) {
        std::swap(children[i], children[i - 1]);
    }
}

Direction GameTree::decideMove(Map &map, int depth)
{
    Direction bestDir = NORTH;

    int alpha = negamax(root, map, depth, -INF, INF, 1, &bestDir);

    fprintf(stderr, "depth: %d, dir: %s, alpha: %d\n", depth, dirToString(bestDir), alpha);

    if (alpha == -INF) {
        fprintf(stderr, "best alpha is -Infinity, we lose no matter what...\n");
        throw std::runtime_error("no possible moves, or all moves result in a loss");
    }

    return bestDir;
}

// returns true if node is NOT a terminal node (ie the tree was built)
bool GameTree::buildTreeTwoLevels(Node *node, const Map &map)
{
    if (map.my_pos() == map.enemy_pos())
        return false;

    if (time_expired)
        throw std::runtime_error("time expired for move decision");

    if (map.cntMoves(SELF) == 0 || map.cntMoves(ENEMY) == 0)
        return false;

    int i = 0, j;
    for (Direction myDir = DIR_MIN; myDir <= DIR_MAX;
            myDir = static_cast<Direction>(myDir + 1)) {
        if (map.isWall(myDir, SELF))
            continue;

        nodesAlloc.push_back(Node(myDir));
        node->children[i] = &nodesAlloc.back();

        j = 0;
        for (Direction enemyDir = DIR_MIN; enemyDir <= DIR_MAX;
            enemyDir = static_cast<Direction>(enemyDir + 1)) {
            if (map.isWall(enemyDir, ENEMY))
                continue;

            nodesAlloc.push_back(Node(enemyDir));
            node->children[i]->children[j] = &nodesAlloc.back();

            ++j;
        }

        ++i;
    }

    return true;
}

int GameTree::negamax(Node *node, Map &map, int depth,
        int alpha, int beta, int sign, Direction *bestDir)
{
    if (time_expired)
        throw std::runtime_error("time expired for move decision");

    if (node->children[0] == NULL) {
        if (!buildTreeTwoLevels(node, map)) {
            return sign * heuristic(map);
        }
    }

    if (depth == 0)
        return sign * heuristic(map);

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        Direction dir = node->children[i]->dir;

        map.move(dir, signToPlayer(sign));
        int a = -negamax(node->children[i], map, depth - 1, -beta, -alpha, -sign, NULL);
        map.unmove(dir, signToPlayer(sign));

        if (a > alpha) {
            alpha = a;

            if (bestDir)
                *bestDir = dir;

            node->promoteMove(i);
        }

        if (alpha >= beta) // beta cutoff
            break;
    }

    return alpha;
}

static void fillBoardVoronoi(std::vector<int> &board, int depth,
        std::vector<position> &posVisits)
{
    std::vector<position> posVisitsOut;
    posVisitsOut.reserve(posVisits.capacity());

    for (std::vector<position>::const_iterator it = posVisits.begin();
            it != posVisits.end(); ++it) {
        if (board[index(it->north())] > depth) {
            board[index(it->north())] = depth;
            posVisitsOut.push_back(it->north());
        }
        if (board[index(it->south())] > depth) {
            board[index(it->south())] = depth;
            posVisitsOut.push_back(it->south());
        }
        if (board[index(it->west())] > depth) {
            board[index(it->west())] = depth;
            posVisitsOut.push_back(it->west());
        }
        if (board[index(it->east())] > depth) {
            board[index(it->east())] = depth;
            posVisitsOut.push_back(it->east());
        }
    }

    posVisits.swap(posVisitsOut);
}

static void setupVoronoiBoards(const Map &map,
        std::vector<int> &boardPlayer,
        std::vector<int> &boardEnemy)
{
    boardPlayer.resize(width*height);

    position pos;
    for (pos.x = 0; pos.x < width; ++pos.x) {
        for (pos.y = 0; pos.y < height; ++pos.y) {
            boardPlayer[index(pos)] = map.isWall(pos) ? INT_MIN : INT_MAX;
        }
    }

    boardPlayer[index(map.my_pos())] = INT_MIN;
    boardPlayer[index(map.enemy_pos())] = INT_MIN;

    boardEnemy = boardPlayer;
}

static int countVoronoiBoards(const std::vector<int> &boardPlayer,
        const std::vector<int> &boardEnemy)
{
    int cnt = 0;
    for (int i = 0; i < width*height; ++i) {
        int playerDepth = boardPlayer[i];
        int enemyDepth = boardEnemy[i];
        if (playerDepth < enemyDepth)
            ++cnt;
        else if (playerDepth > enemyDepth)
            --cnt;
    }
    return cnt;
}

// count of squares player can reach first minus squares enemy can reach first
static int voronoiTerritory(const Map &map)
{
    std::vector<int> boardPlayer, boardEnemy;

    setupVoronoiBoards(map, boardPlayer, boardEnemy);

    std::vector<position> posVisits;
    posVisits.reserve(width*2 + height*2);
    posVisits.push_back(map.my_pos());
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardPlayer, depth, posVisits);
    }

    posVisits.clear();
    posVisits.push_back(map.enemy_pos());
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardEnemy, depth, posVisits);
    }

    return countVoronoiBoards(boardPlayer, boardEnemy);
}

static bool fillBoardDistanceToOpponent(std::vector<bool> &board,
        std::vector<position> &posVisits, position opp_pos)
{
    std::vector<position> posVisitsOut;
    posVisitsOut.reserve(posVisits.capacity());

    for (std::vector<position>::const_iterator it = posVisits.begin();
            it != posVisits.end(); ++it) {
        if (*it == opp_pos)
            return true;

        if (!board[index(it->north())]) {
            board[index(it->north())] = true;
            posVisitsOut.push_back(it->north());
        }
        if (!board[index(it->south())]) {
            board[index(it->south())] = true;
            posVisitsOut.push_back(it->south());
        }
        if (!board[index(it->west())]) {
            board[index(it->west())] = true;
            posVisitsOut.push_back(it->west());
        }
        if (!board[index(it->east())]) {
            board[index(it->east())] = true;
            posVisitsOut.push_back(it->east());
        }
    }

    posVisits.swap(posVisitsOut);
    return false;
}

static int distanceToOpponent(const Map &map)
{
    std::vector<bool> board(map.getBoard());
    board[index(map.my_pos())] = false;
    board[index(map.enemy_pos())] = false;

    std::vector<position> posVisits;
    posVisits.reserve(width*2 + height*2);
    posVisits.push_back(map.my_pos());
    for (int depth = 0; !posVisits.empty(); ++depth) {
        if (fillBoardDistanceToOpponent(board, posVisits, map.enemy_pos())) {
            return depth;
        }
    }
    return -1;
}

static int countCorridorSquares(const Map &map)
{
    std::vector<bool> board(map.getBoard());
    fillUnreachableSquares(board, map.my_pos());

    int cnt = 0;

    position pos;
    for (pos.x = 0; pos.x < width; ++pos.x) {
        for (pos.y = 0; pos.y < height; ++pos.y) {
            if (board[index(pos)])
                continue;

            if (isCorridorSquare(board, pos))
                ++cnt;
        }
    }
    return cnt;
}

static int heuristic(const Map &map)
{
    if (map.my_pos() == map.enemy_pos())
        return 0; // draw

    int playerMoves = map.cntMoves(SELF);
    int enemyMoves = map.cntMoves(ENEMY);

    if (!playerMoves && enemyMoves)
        return -INF; // player lost
    if (playerMoves && !enemyMoves)
        return INF; // player won
    if (!playerMoves && !enemyMoves)
        return 0; // draw


    if (isOpponentIsolated(map)) {
        int cntPlayer = countReachableSquares(map, SELF);
        int cntEnemy = countReachableSquares(map, ENEMY);
        return cntPlayer - cntEnemy;
    } else {
        int voronoi = voronoiTerritory(map);
        return voronoi;
    }
}

Direction decideMoveMinimax(Map map)
{
    GameTree tree;

    Direction dir = NORTH;
    try {
        for (int depth = 2; depth < 100; depth += 2) { // fixme
            dir = tree.decideMove(map, depth);
            fprintf(stderr, "Depth %d ==> %s\n", depth, dirToString(dir));
        }
    } catch (...) {
    }

    return dir;
}

