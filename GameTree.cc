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

static double distanceFromEnemy(const Map &map);
static int movesFromEnemy(const Map &map);
static bool isPlayerWallHugging(const Map &map);

static const double INF = std::numeric_limits<double>::infinity();

typedef double (*HeuristicFunction)(const Map &map);

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
        double negamax(Node *node, int depth, double alpha, double beta,
                int sign, HeuristicFunction fun);
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

static Player signToPlayer(int sign)
{
    if (sign == 1)
        return SELF;
    else
        return ENEMY;
}

Direction GameTree::decideMove(int depth, HeuristicFunction fun)
{
    std::deque<Direction> bestDirs;
    double bestAlpha = -INF;

    for (int i = 0; i < 4 && root.children[i]; ++i) {
        double alpha = -negamax(root.children[i], depth - 1, -INF, INF, -1, fun);
        fprintf(stderr, "%s alpha: %f\n", dirToString(root.children[i]->direction), alpha);

        if (alpha > bestAlpha) {
            bestDirs.clear();
            bestDirs.push_back(root.children[i]->direction);
            bestAlpha = alpha;
        } else if (alpha == bestAlpha) {
            bestDirs.push_back(root.children[i]->direction);
        }
    }

    if (bestAlpha == -INF) {
        fprintf(stderr, "best alpha is -Infinity, we lose...\n");
        throw std::runtime_error("no possible moves, or all moves result in a loss");
    }

    fprintf(stderr, "disambiguating between best moves:");
    for (std::deque<Direction>::const_iterator it = bestDirs.begin();
            it != bestDirs.end(); ++it) {
        fprintf(stderr, " %s", dirToString(*it));
    }
    fprintf(stderr, "\n");

    bestAlpha = -INF;
    Direction ret = NORTH;
    for (std::deque<Direction>::const_iterator it = bestDirs.begin();
            it != bestDirs.end(); ++it) {
        Map newMap(root.map);
        newMap.move(*it, SELF);

        double alpha = fun(newMap);
        if (alpha > bestAlpha) {
            ret = *it;
            bestAlpha = alpha;
        }
    }

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

#if 0
static std::deque<std::pair<Player, Direction> > choices;
#endif

double GameTree::negamax(Node *node, int depth, double alpha, double beta,
        int sign, HeuristicFunction fun)
{
    if (Time::now() > deadline)
        throw std::runtime_error("time expired for move decision");

    if (depth == 0 || node->children[0] == NULL) {
        return sign * fun(node->map);
    }

    for (int i = 0; i < 4 && node->children[i]; ++i) {
#if 0
        choices.push_back(std::make_pair(signToPlayer(sign), node->children[i]->direction));
#endif

        double newAlpha = -negamax(node->children[i], depth - 1, -beta, -alpha,
                -sign, fun);

#if 0
        for (std::deque<std::pair<Player, Direction> >::const_iterator it = choices.begin();
                it != choices.end(); ++it) {
            fprintf(stderr, "%s %s, ", playerToString(it->first), dirToString(it->second));
        }
        fprintf(stderr, "new alpha: %f, curr alpha: %f, curr beta: %f\n", newAlpha, alpha, beta);
        choices.pop_back();
#endif

        if (newAlpha > alpha) {
            std::swap(node->children[0], node->children[i]);
            alpha = newAlpha;
        }

        if (alpha >= beta)
            break;
    }

    return alpha;
}

static double distanceFromEnemy(const Map &map)
{
    double diffX = map.myX() - map.enemyX();
    double diffY = map.myY() - map.enemyY();

    return sqrt(diffX*diffX + diffY*diffY);
}



static int movesFromEnemy(const Map &map)
{
    //assert(!isOpponentIsolated(map));

    std::vector<short> board(map.width()*map.height());
    for (int i = 0; i < map.width(); ++i) {
        for (int j = 0; j < map.height(); ++j) {
            board[i*map.height() + j] = map.isWall(i, j) ? SHRT_MIN : SHRT_MAX;
        }
    }

    board[map.myX()*map.height() + map.myY()] = 0;
    board[map.enemyX()*map.height() + map.enemyY()] = SHRT_MAX;

    for (int depth = 0; ; ++depth) {
        for (int i = 0; i < map.width(); ++i) {
            for (int j = 0; j < map.height(); ++j) {
                if (board[i*map.height() + j] == depth) {
                    if (i == map.enemyX() && j == map.enemyY())
                        return depth;

                    if (board[i*map.height() + j + 1] > depth + 1)
                        board[i*map.height() + j + 1] = depth + 1;
                    if (board[i*map.height() + j - 1] > depth + 1)
                        board[i*map.height() + j - 1] = depth + 1;
                    if (board[(i + 1)*map.height() + j] > depth + 1)
                        board[(i + 1)*map.height() + j] = depth + 1;
                    if (board[(i - 1)*map.height() + j] > depth + 1)
                        board[(i - 1)*map.height() + j] = depth + 1;
                }
            }
        }
    }

}

static bool isPlayerWallHugging(const Map &map)
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

    bool playerHasMoves = map.anyMoves(SELF);
    bool enemyHasMoves = map.anyMoves(ENEMY);

    if (!playerHasMoves && enemyHasMoves)
        return -INF; // player lost
    if (playerHasMoves && !enemyHasMoves)
        return INF; // player won
    if (!playerHasMoves && !enemyHasMoves)
        return 0.0; // draw

    int voronoi = voronoiTerritory(map);

    /*if (isOpponentIsolated(map)) {
        int cntPlayer = countReachableSquares(map, SELF);
        int cntEnemy = countReachableSquares(map, ENEMY);
        if (voronoi != cntPlayer - cntEnemy) {
            fprintf(stderr, "BUG: cnt player: %d, cnt enemy: %d, voronoi: %d\n", cntPlayer, cntEnemy, voronoi);
        }
    }*/

    return voronoi;
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

