#include "MoveDeciders.h"
#include "Time.h"
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

typedef int (*HeuristicFunction)(const Map &map);

class GameTree
{
    public:
        GameTree();
        ~GameTree();

        Direction decideMove(Map &map, int depth, HeuristicFunction fun);

    private:
        struct Node;
        bool buildTreeOneLevel(Node *node, Map &map, Player player);
        int negamax_normal(Node *node, Map &map, int depth,
                int alpha, int beta,
                int sign, HeuristicFunction fun, Direction *dir);
        int negamax_quiescence(Node *node, Map &map, int depth,
                int alpha, int beta,
                int sign, HeuristicFunction fun);
        bool quiet(Node *node, Map &map, int sign, HeuristicFunction fun);
        bool quiet_quiescence(Node *node, Map &map, int sign, HeuristicFunction fun);

        struct Node
        {
            Node *children[4];
            Direction direction;

            Node(Direction dir);
            void promoteChild(int i);
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
    direction(dir)
{
    for (int i = 0; i < 4; ++i)
        children[i] = NULL;
}

inline void GameTree::Node::promoteChild(int i)
{
    switch (i) {
        case 0:
            return;
        case 1:
            std::swap(children[0], children[1]);
            return;
        case 2:
            std::swap(children[1], children[2]);
            std::swap(children[0], children[1]);
            return;
        case 3:
            std::swap(children[2], children[3]);
            std::swap(children[1], children[2]);
            std::swap(children[0], children[1]);
            return;
    }
}

Direction GameTree::decideMove(Map &map, int depth, HeuristicFunction fun)
{
    Direction bestDir = NORTH;

    int alpha = negamax_normal(root, map, depth, -INF, INF, 1, fun, &bestDir);

    fprintf(stderr, "depth: %d, dir: %s, alpha: %d\n", depth, dirToString(bestDir), alpha);

    if (alpha == -INF) {
        fprintf(stderr, "best alpha is -Infinity, we lose no matter what...\n");
        throw std::runtime_error("no possible moves, or all moves result in a loss");
    }

    return bestDir;
}

// returns true if node is NOT a terminal node (ie the tree was built)
bool GameTree::buildTreeOneLevel(Node *node, Map &map, Player player)
{
    if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
        return false;

    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (map.isWall(dir, player))
            continue;

        nodesAlloc.push_back(Node(dir));

        node->children[cnt] = &nodesAlloc.back();

        ++cnt;
    }

    return cnt > 0;
}

bool GameTree::quiet(Node *node, Map &map, int sign, HeuristicFunction fun)
{
    // if we are isolated, the evaluation is likely to be stable (as long as we count the reachable moves correctly)
    if (isOpponentIsolated(map))
        return true;

    int a1 = sign * fun(map);
    int a2 = negamax_quiescence(node, map, 1, -INF, INF, sign, fun);

    if (abs(a1 - a2) <= 4)
        return true;

    return false;
}

bool GameTree::quiet_quiescence(Node *node, Map &map, int sign, HeuristicFunction fun)
{
    if (isOpponentIsolated(map))
        return true;

    return false;
}

int GameTree::negamax_quiescence(Node *node, Map &map, int depth,
        int alpha, int beta, int sign, HeuristicFunction fun)
{
    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    if (node->children[0] == NULL) {
        if (!buildTreeOneLevel(node, map, signToPlayer(sign)))
            return sign * fun(map);
    }

    if (depth == 0 || quiet_quiescence(node, map, sign, fun)) {
        return sign * fun(map);
    }

    // singularity enhancement - if only one possible move, don't let it count
    // towards the depth
    int newdepth = depth - 1;
    if (!node->children[1])
        newdepth = depth;

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        Direction dir = node->children[i]->direction;

        map.move(dir, signToPlayer(sign));
        int a = -negamax_quiescence(node->children[i], map, newdepth,
                -beta, -alpha, -sign, fun);
        map.unmove(dir, signToPlayer(sign));

        if (a > alpha) {
            node->promoteChild(i);
            alpha = a;
        }

        if (alpha >= beta)
            break;
    }

    return alpha;
}

int GameTree::negamax_normal(Node *node, Map &map, int depth,
        int alpha, int beta,
        int sign, HeuristicFunction fun, Direction *bestDir)
{
    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    if (node->children[0] == NULL) {
        if (!buildTreeOneLevel(node, map, signToPlayer(sign)))
            return sign * fun(map);
    }

    if (depth == 0) {
        if (quiet(node, map, sign, fun))
            return sign * fun(map);
        else
            return negamax_quiescence(node, map, 6, alpha, beta, sign, fun);
    }

    // singularity enhancement - if only one possible move, don't let it count
    // towards the depth
    int newdepth = depth - 1;
    if (!node->children[1])
        newdepth = depth;

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        Direction dir = node->children[i]->direction;

        map.move(dir, signToPlayer(sign));
        int a = -negamax_normal(node->children[i], map, newdepth,
                -beta, -alpha, -sign, fun, NULL);
        map.unmove(dir, signToPlayer(sign));

        if (a > alpha) {
            if (bestDir)
                *bestDir = dir;

            node->promoteChild(i);
            alpha = a;
        }

        if (alpha >= beta)
            break;
    }

    return alpha;
}

static void fillBoardVoronoi(std::vector<int> &board, int depth,
        std::vector<std::pair<int, int> > &posVisits)
{
    std::vector<std::pair<int, int> > posVisitsOut;
    posVisitsOut.reserve(posVisits.capacity());

    for (std::vector<std::pair<int, int> >::const_iterator it = posVisits.begin();
            it != posVisits.end(); ++it) {
        int x = it->first, y = it->second;
        if (board[x*height + y + 1] > depth) {
            board[x*height + y + 1] = depth;
            posVisitsOut.push_back(std::pair<int, int>(x, y + 1));
        }
        if (board[x*height + y - 1] > depth) {
            board[x*height + y - 1] = depth;
            posVisitsOut.push_back(std::pair<int, int>(x, y - 1));
        }
        if (board[(x + 1)*height + y] > depth) {
            board[(x + 1)*height + y] = depth;
            posVisitsOut.push_back(std::pair<int, int>(x + 1, y));
        }
        if (board[(x - 1)*height + y] > depth) {
            board[(x - 1)*height + y] = depth;
            posVisitsOut.push_back(std::pair<int, int>(x - 1, y));
        }
    }

    posVisits.swap(posVisitsOut);
}

static void setupVoronoiBoards(const Map &map,
        std::vector<int> &boardPlayer,
        std::vector<int> &boardEnemy)
{
    boardPlayer.resize(width*height);

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            boardPlayer[i*height + j] = map.isWall(i, j) ? INT_MIN : INT_MAX;
        }
    }

    boardPlayer[map.myX()*height + map.myY()] = INT_MIN;
    boardPlayer[map.enemyX()*height + map.enemyY()] = INT_MIN;

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

    std::vector<std::pair<int, int> > posVisits;
    posVisits.reserve(width*2 + height*2);
    posVisits.push_back(std::pair<int, int>(map.myX(), map.myY()));
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardPlayer, depth, posVisits);
    }

    posVisits.clear();
    posVisits.push_back(std::pair<int, int>(map.enemyX(), map.enemyY()));
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardEnemy, depth, posVisits);
    }

    return countVoronoiBoards(boardPlayer, boardEnemy);
}

static bool fillBoardDistanceToOpponent(std::vector<bool> &board,
        std::vector<std::pair<int, int> > &posVisits,
        int oppX, int oppY)
{
    std::vector<std::pair<int, int> > posVisitsOut;
    posVisitsOut.reserve(posVisits.capacity());

    for (std::vector<std::pair<int, int> >::const_iterator it = posVisits.begin();
            it != posVisits.end(); ++it) {
        int x = it->first, y = it->second;

        if (x == oppX && y == oppY)
            return true;

        if (!board[x*height + y + 1]) {
            board[x*height + y + 1] = true;
            posVisitsOut.push_back(std::make_pair(x, y + 1));
        }
        if (!board[x*height + y - 1]) {
            board[x*height + y - 1] = true;
            posVisitsOut.push_back(std::make_pair(x, y - 1));
        }
        if (!board[(x + 1)*height + y]) {
            board[(x + 1)*height + y] = true;
            posVisitsOut.push_back(std::make_pair(x + 1, y));
        }
        if (!board[(x - 1)*height + y]) {
            board[(x - 1)*height + y] = true;
            posVisitsOut.push_back(std::make_pair(x - 1, y));
        }
    }

    posVisits.swap(posVisitsOut);
    return false;
}

static int distanceToOpponent(const Map &map)
{
    std::vector<bool> board(map.getBoard());
    board[map.myX()*height + map.myY()] = false;
    board[map.enemyX()*height + map.enemyY()] = false;

    std::vector<std::pair<int, int> > posVisits;
    posVisits.reserve(width*2 + height*2);
    posVisits.push_back(std::pair<int, int>(map.myX(), map.myY()));
    for (int depth = 0; !posVisits.empty(); ++depth) {
        if (fillBoardDistanceToOpponent(board, posVisits, map.enemyX(),
                    map.enemyY())) {
            return depth;
        }
    }
    return -1;
}

// note must be tested to have 1 xmove and 1 ymove before entry
static bool isCornerCorridor(const std::vector<bool> &board, int x, int y)
{
    int x2, y2;

    if (!board[(x - 1)*height + y])
        x2 = x - 1;
    else
        x2 = x + 1;

    if (!board[x*height + y - 1])
        y2 = y - 1;
    else 
        y2 = y + 1;

    return board[x2*height + y2];
}

static int countCorridorSquares(const Map &map)
{
    std::vector<bool> board(map.getBoard());
    fillUnreachableSquares(board, map.myX(), map.myY());

    int cnt = 0;

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (board[i*height + j])
                continue;

            int xmoves = 0, ymoves = 0;
            if (!board[(i - 1)*height + j])
                ++xmoves;
            if (!board[(i + 1)*height + j])
                ++xmoves;
            if (!board[i*height + (j - 1)])
                ++ymoves;
            if (!board[i*height + (j + 1)])
                ++ymoves;

            if (xmoves + ymoves <= 1 ||
                    (ymoves == 2 && xmoves == 0) ||
                    (ymoves == 0 && xmoves == 2) ||
                    (xmoves == 1 && ymoves == 1 && isCornerCorridor(board, i, j))) {
                ++cnt;
            }
        }
    }
    return cnt;
}

static int fitness(const Map &map)
{
    if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
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
        int corridors = countCorridorSquares(map);
        return voronoi - corridors;
    }
}

Direction decideMoveMinimax(Map map)
{
    GameTree tree;

    Direction dir = NORTH;
    try {
        for (int depth = 2; depth < 100; depth += 2) {
            dir = tree.decideMove(map, depth, &fitness);
            fprintf(stderr, "Depth %d ==> %s\n", depth, dirToString(dir));
        }
    } catch (...) {
    }

    return dir;
}

