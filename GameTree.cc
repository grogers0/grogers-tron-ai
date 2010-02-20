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

static const double INF = std::numeric_limits<double>::infinity();

typedef double (*HeuristicFunction)(const Map &map);

class GameTree
{
    public:
        GameTree();
        ~GameTree();

        Direction decideMove(Map &map, int depth, HeuristicFunction fun);

        void extendTree(Map &map, int plies);

    private:
        struct Node;
        void buildTree(Node *node, Map &map, int plies, Player player);
        void extendTree(Node *node, Map &map, int plies, Player player);
        double negamax(Node *node, Map &map, int depth,
                double alpha, double beta,
                int sign, HeuristicFunction fun, Direction *dir);

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

    double alpha = negamax(root, map, depth, -INF, INF, 1, fun, &bestDir);

    if (alpha == -INF) {
        fprintf(stderr, "best alpha is -Infinity, we lose no matter what...\n");
        throw std::runtime_error("no possible moves, or all moves result in a loss");
    }

    return bestDir;
}

void GameTree::buildTree(Node *node, Map &map, int plies, Player player)
{
    if (plies <= 0)
        return;

    if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
        return;

    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (map.isWall(dir, player))
            continue;

        nodesAlloc.push_back(Node(dir));

        node->children[cnt] = &nodesAlloc.back();

        map.move(dir, player);
        buildTree(node->children[cnt], map, plies - 1, otherPlayer(player));
        map.unmove(dir, player);

        ++cnt;
    }
}

void GameTree::extendTree(Map &map, int plies)
{
    extendTree(root, map, plies, SELF);
}

void GameTree::extendTree(Node *node, Map &map, int plies, Player player)
{
    if (plies <= 0)
        return;

    if (!node->children[0]) {
        buildTree(node, map, plies, player);
        return;
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        map.move(node->children[i]->direction, player);
        extendTree(node->children[i], map, plies - 1, otherPlayer(player));
        map.unmove(node->children[i]->direction, player);
    }
}


#if 0
static std::deque<std::pair<Player, Direction> > choices;
#endif

double GameTree::negamax(Node *node, Map &map, int depth,
        double alpha, double beta,
        int sign, HeuristicFunction fun, Direction *bestDir)
{
    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    if (depth == 0 || node->children[0] == NULL) {
        return sign * fun(map);
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        Direction dir = node->children[i]->direction;
#if 0
        choices.push_back(std::make_pair(signToPlayer(sign), node->children[i]->direction));
#endif

        map.move(dir, signToPlayer(sign));
        double a = -negamax(node->children[i], map, depth - 1,
                -beta, -alpha, -sign, fun, NULL);
        map.unmove(dir, signToPlayer(sign));

#if 0
        for (std::deque<std::pair<Player, Direction> >::const_iterator it = choices.begin();
                it != choices.end(); ++it) {
            fprintf(stderr, "%s %s, ", playerToString(it->first), dirToString(it->second));
        }
        fprintf(stderr, "new alpha: %g, curr alpha: %g, curr beta: %g\n", a, alpha, beta);
        choices.pop_back();
#endif

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

void fillBoardVoronoi(std::vector<signed char> &board, int depth,
        std::vector<std::pair<unsigned char, unsigned char> > &posVisits,
        int height)
{
    std::vector<std::pair<unsigned char, unsigned char> > posVisitsOut;
    posVisitsOut.reserve(posVisits.capacity());

    for (std::vector<std::pair<unsigned char, unsigned char> >::const_iterator it = posVisits.begin();
            it != posVisits.end(); ++it) {
        int x = it->first, y = it->second;
        if (board[x*height + y + 1] > depth) {
            board[x*height + y + 1] = depth;
            posVisitsOut.push_back(std::pair<unsigned char, unsigned char>(x, y + 1));
        }
        if (board[x*height + y - 1] > depth) {
            board[x*height + y - 1] = depth;
            posVisitsOut.push_back(std::pair<unsigned char, unsigned char>(x, y - 1));
        }
        if (board[(x + 1)*height + y] > depth) {
            board[(x + 1)*height + y] = depth;
            posVisitsOut.push_back(std::pair<unsigned char, unsigned char>(x + 1, y));
        }
        if (board[(x - 1)*height + y] > depth) {
            board[(x - 1)*height + y] = depth;
            posVisitsOut.push_back(std::pair<unsigned char, unsigned char>(x - 1, y));
        }
    }

    posVisits.swap(posVisitsOut);
}

// count of squares player can reach first minus squares enemy can reach first
static int voronoiTerritory(const Map &map)
{
    int width = map.width();
    int height = map.height();

    std::vector<signed char> boardPlayer(width*height);

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            boardPlayer[i*height + j] = map.isWall(i, j) ? SCHAR_MIN : SCHAR_MAX;
        }
    }

    boardPlayer[map.myX()*height + map.myY()] = SCHAR_MIN;
    boardPlayer[map.enemyX()*height + map.enemyY()] = SCHAR_MIN;

    std::vector<signed char> boardEnemy(boardPlayer);

    std::vector<std::pair<unsigned char, unsigned char> > posVisits;
    posVisits.reserve(width*2 + height*2);
    posVisits.push_back(std::pair<unsigned char, unsigned char>(map.myX(), map.myY()));
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardPlayer, depth, posVisits, height);
    }

    posVisits.clear();
    posVisits.push_back(std::pair<unsigned char, unsigned char>(map.enemyX(), map.enemyY()));
    for (int depth = 1; !posVisits.empty(); ++depth) {
        fillBoardVoronoi(boardEnemy, depth, posVisits, height);
    }

    int cnt = 0;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            if (boardPlayer[i*height + j] < boardEnemy[i*height + j])
                ++cnt;
            else if (boardPlayer[i*height + j] > boardEnemy[i*height + j])
                --cnt;
        }
    }

    return cnt;
}

static double fitness(const Map &map)
{
    if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
        return 0.0; // draw

    int playerMoves = map.cntMoves(SELF);
    int enemyMoves = map.cntMoves(ENEMY);

    if (!playerMoves && enemyMoves)
        return -INF; // player lost
    if (playerMoves && !enemyMoves)
        return INF; // player won
    if (!playerMoves && !enemyMoves)
        return 0.0; // draw


    if (isOpponentIsolated(map)) {
        int cntPlayer = countReachableSquares(map, SELF);
        int cntEnemy = countReachableSquares(map, ENEMY);
        return cntPlayer - cntEnemy;
    } else {
        return voronoiTerritory(map);
    }
}

Direction decideMoveMinimax(Map map)
{
    GameTree tree;

    Direction dir = NORTH;
    try {
        for (int depth = 1; ; ++depth) {
            tree.extendTree(map, depth);
            dir = tree.decideMove(map, depth, &fitness);
            fprintf(stderr, "Depth %d ==> %s\n", depth, dirToString(dir));
        }
    } catch (...) {
    }

    return dir;
}

