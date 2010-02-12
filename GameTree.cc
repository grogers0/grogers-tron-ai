#include "MoveDeciders.h"
#include <climits>
#include <cstdio>
#include <sys/time.h>

const int MAX_DEPTH = 9;

typedef int (*HeuristicFunction)(const Map &map, Direction dir);

class GameTree
{
    public:
        GameTree(const Map &map, int plies);
        ~GameTree();

        Direction decideMove(int depth, HeuristicFunction fun);

    private:
        struct Node;
        void buildTree(Node *node, int plies, Player player);
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
    negamax(&root, depth, -INT_MAX, INT_MAX, 1, &ret, fun);
    return ret;
}

void GameTree::buildTree(Node *node, int plies, Player player)
{
    for (int i = 0; i < 4; ++i)
        node->children[i] = NULL;

    if (plies < 0)
        return;

    int cnt = 0;
    for (Direction dir = DIR_MIN; dir <= DIR_MAX;
            dir = static_cast<Direction>(dir + 1)) {
        if (node->map.isWall(dir, player))
            continue;

        node->children[cnt] = new Node;
        node->children[cnt]->direction = dir;
        node->children[cnt]->map = node->map;
        node->children[cnt]->map.move(dir, player);

        buildTree(node->children[cnt], plies - 1, otherPlayer(player));

        ++cnt;
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
        return color * fun(node->map, node->direction);
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
        int newAlpha = -negamax(node->children[i], depth - 1, -beta, -alpha,
                -color, NULL, fun);
/*
        for (int j = 0; j < MAX_DEPTH - depth; ++j)
            fprintf(stderr, "    ");
        fprintf(stderr, "depth: %d, %s %s, alpha: %d, newalpha: %d, beta: %d\n", depth, playerToString(colorToPlayer(color)), dirToString(node->children[i]->direction), alpha, newAlpha, beta);
        */

        if (newAlpha > alpha) {
            if (dir)
                *dir = node->children[i]->direction;
            alpha = newAlpha;
        }

        if (alpha >= beta)
            break;
    }

    return alpha;
}

// todo - make this so that when far away, the effect is small, but when closer
// the effect is greater
static int isDirectionTowardsOtherPlayer(const Map &map, Direction dir)
{
    int diffX, diffY;

    diffX = map.myX() - map.enemyX();
    diffY = map.myY() - map.enemyY();

    if (diffX > 0 && dir == WEST)
        return true;
    if (diffX < 0 && dir == EAST)
        return true;

    if (diffY > 0 && dir == NORTH)
        return true;
    if (diffY < 0 && dir == SOUTH)
        return true;

    return false;
}

static bool isPlayerHuggingWall(const Map &map)
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

static int fitness(const Map &map, Direction dir)
{
    int cntPlayer = countReachableSquares(map, SELF);
    int cntEnemy = countReachableSquares(map, ENEMY);

    if (cntPlayer == 0 && cntEnemy > 0)
        return -INT_MAX; // player lost
    else if (cntPlayer > 0 && cntEnemy == 0)
        return INT_MAX; // player won
    else if (cntPlayer == 0 && cntEnemy == 0)
        return 0; // draw

    int reachableMovesAdvantage = cntPlayer - cntEnemy;
    //int towardsEnemy = isDirectionTowardsOtherPlayer(map, dir);
    //int huggingWall = isPlayerHuggingWall(map);

    return reachableMovesAdvantage; // + 2*towardsEnemy + huggingWall;
}

Direction decideMoveMinimax(const Map &map)
{
    timeval ts, te;
    gettimeofday(&ts, NULL);

    int depth = MAX_DEPTH;
    GameTree tree(map, depth);

    Direction dir = tree.decideMove(depth, &fitness);

    gettimeofday(&te, NULL);

    timeval tdiff;
    timersub(&te, &ts, &tdiff);

    fprintf(stderr, "finding move %s took %d sec, %d usec\n", dirToString(dir), tdiff.tv_sec, tdiff.tv_usec);

    return dir;
}

