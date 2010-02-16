#include "MoveDeciders.h"
#include "Time.h"
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <utility>
#include <stdexcept>

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

    if (player == SELF &&
            node->map.myX() == node->map.enemyX() &&
            node->map.myY() == node->map.enemyY())
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
    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    if (depth == 0 || node->children[0] == NULL) {
        return color * fun(node->map, node->direction, colorToPlayer(color));
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
#if 0
        static std::deque<std::pair<Player, Direction> > choices;
        choices.push_back(std::make_pair(colorToPlayer(color), node->children[i]->direction));
#endif

        int newAlpha = -negamax(node->children[i], depth - 1, -beta, -alpha,
                -color, NULL, fun);

#if 0
        for (std::deque<std::pair<Player, Direction> >::const_iterator it = choices.begin();
                it != choices.end(); ++it) {
            fprintf(stderr, "%s %s, ", playerToString(it->first), dirToString(it->second));
        }
        fprintf(stderr, "new alpha: %d, curr alpha: %d, curr beta: %d\n", newAlpha, alpha, beta);
        choices.pop_back();
#endif

        if (newAlpha > alpha) {
            if (dir)
                *dir = node->children[i]->direction;

            std::swap(node->children[0], node->children[i]);
            alpha = newAlpha;
        }

        if (alpha >= beta)
            break;
    }

    return alpha;
}

static int distanceFromEnemy(const Map &map)
{
    int diffX = map.myX() - map.enemyX();
    int diffY = map.myY() - map.enemyY();

    int distance = map.width()*map.width() + map.height()*map.height() -
        diffX*diffX - diffY*diffY;

    return distance;
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
    if (map.myX() == map.enemyX() && map.myY() == map.enemyY())
        return -1; // draw

    int cntPlayer = countReachableSquares(map, SELF);
    int cntEnemy = countReachableSquares(map, ENEMY);

    if (cntPlayer == 0 && cntEnemy > 0)
        return -INT_MAX; // player lost
    else if (cntPlayer > 0 && cntEnemy == 0)
        return INT_MAX; // player won
    else if (cntPlayer == 0 && cntEnemy == 0)
        return -1; // draw

    int reachableMovesAdvantage = cntPlayer - cntEnemy;
    return reachableMovesAdvantage;
}

Direction decideMoveMinimax(const Map &map)
{
    GameTree tree(map, 0);

    Direction dir = NORTH;
    try {
        for (int depth = 1; ; ++depth) {
            tree.extendTree(map, depth);
            dir = tree.decideMove(depth, &fitness);
            fprintf(stderr, "Depth %d ==> %s\n", depth, dirToString(dir));
        }
    } catch (...) {
    }

    return dir;
}

