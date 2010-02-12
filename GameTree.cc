#include "MoveDeciders.h"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <deque>

static Direction disambiguateBestMoves(const std::deque<Direction> &moves,
        const Map &map);

typedef int (*HeuristicFunction)(const Map &map, Direction dir, Player p);

class GameTree
{
    public:
        GameTree(const Map &map, int plies);
        ~GameTree();

        Direction decideMove(int depth, HeuristicFunction fun);

        void extendTree(const Map &map, int plies);

    private:
        struct Node;
        void buildTree(Node *node, int plies, Player player);
        void extendTree(Node *node, int plies, Player player);
        int negamax(Node *node, int depth, int alpha, int beta, int color,
                Direction *dir, HeuristicFunction fun);
        void destroySubtree(Node *node);

        struct Node
        {
            Node *children[4];

            Direction direction;
            Map map;
        };

        Node root;
};

GameTree::GameTree(const Map &map, int plies)
{
    root.direction = NORTH;
    root.map = map;

    buildTree(&root, plies, SELF);
}

GameTree::~GameTree()
{
    destroySubtree(&root);
}

static Player colorToPlayer(int color)
{
    if (color == 1)
        return SELF;
    else
        return ENEMY;
}

Direction GameTree::decideMove(int depth, HeuristicFunction fun)
{
    Direction ret = NORTH;
    int alpha = negamax(&root, depth, -INT_MAX, INT_MAX, 1, &ret, fun);
    fprintf(stderr, "Negamax returned alpha: %d\n", alpha);
    return ret;
}

void GameTree::buildTree(Node *node, int plies, Player player)
{
    for (int i = 0; i < 4; ++i)
        node->children[i] = NULL;

    if (plies <= 0)
        return;

    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (node->map.isWall(dir, player))
            continue;

        node->children[cnt] = new Node;
        node->children[cnt]->direction = dir;
        node->children[cnt]->map = node->map;
        node->children[cnt]->map.move(dir, player, player == SELF);

        buildTree(node->children[cnt], plies - 1, otherPlayer(player));

        ++cnt;
    }
}

void GameTree::extendTree(const Map &map, int plies)
{
    extendTree(&root, plies, SELF);
}

void GameTree::extendTree(Node *node, int plies, Player player)
{
    if (plies <= 0)
        return;

    if (!node->children[0]) {
        buildTree(node, plies, player);
        return;
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        extendTree(node->children[i], plies - 1, otherPlayer(player));
    }
}


void GameTree::destroySubtree(Node *node)
{
    for (int i = 0; i < 4 && node->children[i]; ++i) {
        destroySubtree(node->children[i]);
    }

    if (node != &root)
        delete node;
}

int GameTree::negamax(Node *node, int depth, int alpha, int beta, int color,
        Direction *dir, HeuristicFunction fun)
{
    if (depth == 0 || node->children[0] == NULL) {
        return color * fun(node->map, node->direction, colorToPlayer(color));
    }

    std::deque<Direction> bestMoves;

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        int newAlpha = -negamax(node->children[i], depth - 1, -beta, -alpha,
                -color, NULL, fun);

        /*
        for (int j = 0; j < 20 - depth; ++j)
            fprintf(stderr, "  ");
        fprintf(stderr, "depth: %d, %s %s, alpha: %d, newalpha: %d, beta: %d\n", depth, playerToString(colorToPlayer(color)), dirToString(node->children[i]->direction), alpha, newAlpha, beta);
        */

        if (newAlpha > alpha) {
            if (dir) {
                bestMoves.clear();
                bestMoves.push_back(node->children[i]->direction);
            }

            std::swap(node->children[0], node->children[i]); // for subsequent runs more pruning
            alpha = newAlpha;
        } else if (newAlpha == alpha) {
            if (dir) {
                bestMoves.push_back(node->children[i]->direction);
            }
        }

        if (alpha >= beta)
            break;
    }

    if (dir) {
        *dir = disambiguateBestMoves(bestMoves, node->map);
    }

    return alpha;
}

static int isPlayerCloseToEnemy(const Map &map)
{
    int diffX = map.myX() - map.enemyX();
    int diffY = map.myY() - map.enemyY();

    return map.width()*map.width() + map.height()*map.height() -
        diffX*diffX - diffY*diffY;
}

static bool isPlayerWallHugging(Map map)
{
    int x = map.myX();
    int y = map.myY();

    int cnt = 0;

    if (map.isWall(x - 1, y))
        ++cnt;
    if (map.isWall(x + 1, y))
        ++cnt;
    if (map.isWall(x, y - 1))
        ++cnt;
    if (map.isWall(x, y + 1))
        ++cnt;

    return cnt >= 2;
}

static int fitness(const Map &map, Direction dir, Player currentPlayer)
{
    int cntPlayer = countReachableSquares(map, SELF);
    int cntEnemy = countReachableSquares(map, ENEMY);

    if (cntPlayer == 0 && cntEnemy > 0)
        return -INT_MAX; // player lost
    else if (cntPlayer > 0 && cntEnemy == 0)
        return INT_MAX; // player won
    else if (cntPlayer == 0 && cntEnemy == 0)
        return 0; // draw
    else if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
        return 0; // draw

    int reachableMovesAdvantage = cntPlayer - cntEnemy;
    return reachableMovesAdvantage;
}

static Direction disambiguateBestMoves(const std::deque<Direction> &moves,
        const Map &map)
{
    Direction bestDir = NORTH;
    int bestFit = -INT_MAX;

    for (std::deque<Direction>::const_iterator it = moves.begin();
            it != moves.end(); ++it) {
        Map newMap(map);
        newMap.move(*it, SELF);

        int towardsEnemy = isPlayerCloseToEnemy(newMap);
        int wallHug = isPlayerWallHugging(newMap);

        int fit = towardsEnemy + wallHug*100;

        if (fit > bestFit) {
            bestFit = fit;
            bestDir = *it;
        }
    }

    return bestDir;
}

static double tvtod(const timeval &tv)
{
    return double(tv.tv_sec) + double(tv.tv_usec)/1e6;
}

Direction decideMoveMinimax(const Map &map)
{
    timeval tv;
    gettimeofday(&tv, NULL);
    double tstart = tvtod(tv);

    GameTree tree(map, 0);

    double tincr = 0, ttot = 0.0;
    Direction dir = NORTH;
    for (int depth = 1; ttot + tincr*3.0 < 0.95; ++depth) {
        gettimeofday(&tv, NULL);
        double tincrstart = tvtod(tv);

        tree.extendTree(map, depth);
        dir = tree.decideMove(depth, &fitness);

        gettimeofday(&tv, NULL);
        tincr = tvtod(tv) - tincrstart;
        ttot = tvtod(tv) - tstart;

        fprintf(stderr, "at depth %d, finding move %s took %f incr sec, %f total sec\n", depth, dirToString(dir), tincr, ttot);
    }

    return dir;
}

