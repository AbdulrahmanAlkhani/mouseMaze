#include "solver.h"
#include "queue.h"

// Global maze data
unsigned int maze[MAZE_SIZE][MAZE_SIZE] = {0};
int distances[MAZE_SIZE][MAZE_SIZE] = {-1}; // -1 means not yet visited
Coordinate position;
Heading heading;

// Wall detection flags
int iswallFront = 0;
int iswallLeft = 0;
int iswallRight = 0;

// State flag
int reached_center = 0;

void initialize()
{
    // Set maze borders
    for (int i = 1; i < MAZE_SIZE - 1; ++i)
    {
        maze[0][i] = _0001;
        maze[i][0] = _0010;
        maze[i][MAZE_SIZE - 1] = _1000;
        maze[MAZE_SIZE - 1][i] = _0100;
    }

    maze[0][0] = _0011;
    maze[0][MAZE_SIZE - 1] = _1001;
    maze[MAZE_SIZE - 1][0] = _0110;
    maze[MAZE_SIZE - 1][MAZE_SIZE - 1] = _1100;

    // Reset distance map
    resetDistances();

    // Initialize starting position and heading
    position.x = 0;
    position.y = 0;
    heading = NORTH;
}

/*
Updates the maze's walls based on what the mouse can currently see
*/
void updateMaze(int iswallFront, int iswallLeft, int iswallRight)
{
    int x = position.x;
    int y = position.y;
    unsigned int walls = _0000;

    switch (heading)
    {
    case NORTH:
        if (iswallFront)
        {
            walls |= _1000;
            if (y + 1 < MAZE_SIZE)
                maze[x][y + 1] |= _0010;
        }
        if (iswallLeft)
        {
            walls |= _0001;
            if (x - 1 >= 0)
                maze[x - 1][y] |= _0100;
        }
        if (iswallRight)
        {
            walls |= _0100;
            if (x + 1 < MAZE_SIZE)
                maze[x + 1][y] |= _0001;
        }
        break;

    case EAST:
        if (iswallFront)
        {
            walls |= _0100;
            if (x + 1 < MAZE_SIZE)
                maze[x + 1][y] |= _0001;
        }
        if (iswallLeft)
        {
            walls |= _1000;
            if (y + 1 < MAZE_SIZE)
                maze[x][y + 1] |= _0010;
        }
        if (iswallRight)
        {
            walls |= _0010;
            if (y - 1 >= 0)
                maze[x][y - 1] |= _1000;
        }
        break;

    case SOUTH:
        if (iswallFront)
        {
            walls |= _0010;
            if (y - 1 >= 0)
                maze[x][y - 1] |= _1000;
        }
        if (iswallLeft)
        {
            walls |= _0100;
            if (x + 1 < MAZE_SIZE)
                maze[x + 1][y] |= _0001;
        }
        if (iswallRight)
        {
            walls |= _0001;
            if (x - 1 >= 0)
                maze[x - 1][y] |= _0100;
        }
        break;

    case WEST:
        if (iswallFront)
        {
            walls |= _0001;
            if (x - 1 >= 0)
                maze[x - 1][y] |= _0100;
        }
        if (iswallLeft)
        {
            walls |= _0010;
            if (y - 1 >= 0)
                maze[x][y - 1] |= _1000;
        }
        if (iswallRight)
        {
            walls |= _1000;
            if (y + 1 < MAZE_SIZE)
                maze[x][y + 1] |= _0010;
        }
        break;
    }

    maze[x][y] |= walls;
}

int xyToSquare(int x, int y)
{
    return x + MAZE_SIZE * y;
}

Coordinate squareToCoord(int square)
{
    Coordinate coord;
    coord.x = square % MAZE_SIZE;
    coord.y = square / MAZE_SIZE;
    return coord;
}

void resetDistances()
{
    for (int x = 0; x < MAZE_SIZE; ++x)
    {
        for (int y = 0; y < MAZE_SIZE; ++y)
        {
            distances[x][y] = -1;
        }
    }

    // Set goal cells
    if (!reached_center)
    {
        if (MAZE_SIZE % 2 == 0)
        {
            distances[MAZE_SIZE / 2][MAZE_SIZE / 2] = 0;
            distances[MAZE_SIZE / 2 - 1][MAZE_SIZE / 2] = 0;
            distances[MAZE_SIZE / 2][MAZE_SIZE / 2 - 1] = 0;
            distances[MAZE_SIZE / 2 - 1][MAZE_SIZE / 2 - 1] = 0;
        }
        else
        {
            distances[MAZE_SIZE / 2][MAZE_SIZE / 2] = 0;
        }
    }
    else
    {
        distances[0][0] = 0;
    }
}

int isWallInDirection(int x, int y, Heading direction)
{
    switch (direction)
    {
    case NORTH:
        return maze[x][y] >= 8;
    case EAST:
        return maze[x][y] % 8 >= 4;
    case SOUTH:
        return maze[x][y] % 4 >= 2;
    case WEST:
        return maze[x][y] % 2 == 1;
    default:
        return 0;
    }
}

void updateDistances()
{
    resetDistances();
    queue squares = queue_create();

    for (int x = 0; x < MAZE_SIZE; ++x)
    {
        for (int y = 0; y < MAZE_SIZE; ++y)
        {
            if (distances[x][y] == 0)
                queue_push(squares, xyToSquare(x, y));
        }
    }

    while (!queue_is_empty(squares))
    {
        Coordinate square = squareToCoord(queue_pop(squares));
        int x = square.x;
        int y = square.y;

        if (!isWallInDirection(x, y, NORTH) && distances[x][y + 1] == -1)
        {
            distances[x][y + 1] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x, y + 1));
        }
        if (!isWallInDirection(x, y, EAST) && distances[x + 1][y] == -1)
        {
            distances[x + 1][y] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x + 1, y));
        }
        if (!isWallInDirection(x, y, SOUTH) && distances[x][y - 1] == -1)
        {
            distances[x][y - 1] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x, y - 1));
        }
        if (!isWallInDirection(x, y, WEST) && distances[x - 1][y] == -1)
        {
            distances[x - 1][y] = distances[x][y] + 1;
            queue_push(squares, xyToSquare(x - 1, y));
        }
    }
}

void updateHeading(Action nextAction)
{
    if (nextAction == FORWARD || nextAction == IDLE)
        return;

    if (nextAction == LEFT)
    {
        switch (heading)
        {
        case NORTH:
            heading = WEST;
            break;
        case EAST:
            heading = NORTH;
            break;
        case SOUTH:
            heading = EAST;
            break;
        case WEST:
            heading = SOUTH;
            break;
        }
    }
    else if (nextAction == RIGHT)
    {
        switch (heading)
        {
        case NORTH:
            heading = EAST;
            break;
        case EAST:
            heading = SOUTH;
            break;
        case SOUTH:
            heading = WEST;
            break;
        case WEST:
            heading = NORTH;
            break;
        }
    }
}

void updatePosition(Action nextAction)
{
    if (nextAction != FORWARD)
        return;

    switch (heading)
    {
    case NORTH:
        position.y += 1;
        break;
    case SOUTH:
        position.y -= 1;
        break;
    case EAST:
        position.x += 1;
        break;
    case WEST:
        position.x -= 1;
        break;
    }
}

Action solver()
{
    if (!reached_center && distances[position.x][position.y] == 0)
        reached_center = 1;
    else if (reached_center && distances[position.x][position.y] == 0)
        reached_center = 0;

    updateMaze(iswallFront, iswallLeft, iswallRight);
    updateDistances();

    Action action = floodFill();

    updateHeading(action);
    updatePosition(action);
    return action;
}

Action floodFill()
{
    unsigned int least_distance = 300;
    Action optimal_move = IDLE;

    if (heading == NORTH)
    {
        if (!isWallInDirection(position.x, position.y, NORTH) && distances[position.x][position.y + 1] < least_distance)
        {
            least_distance = distances[position.x][position.y + 1];
            optimal_move = FORWARD;
        }
        if (!isWallInDirection(position.x, position.y, EAST) && distances[position.x + 1][position.y] < least_distance)
        {
            least_distance = distances[position.x + 1][position.y];
            optimal_move = RIGHT;
        }
        if (!isWallInDirection(position.x, position.y, WEST) && distances[position.x - 1][position.y] < least_distance)
        {
            least_distance = distances[position.x - 1][position.y];
            optimal_move = LEFT;
        }
    }
    else if (heading == EAST)
    {
        if (!isWallInDirection(position.x, position.y, EAST) && distances[position.x + 1][position.y] < least_distance)
        {
            least_distance = distances[position.x + 1][position.y];
            optimal_move = FORWARD;
        }
        if (!isWallInDirection(position.x, position.y, SOUTH) && distances[position.x][position.y - 1] < least_distance)
        {
            least_distance = distances[position.x][position.y - 1];
            optimal_move = RIGHT;
        }
        if (!isWallInDirection(position.x, position.y, NORTH) && distances[position.x][position.y + 1] < least_distance)
        {
            least_distance = distances[position.x][position.y + 1];
            optimal_move = LEFT;
        }
    }
    else if (heading == SOUTH)
    {
        if (!isWallInDirection(position.x, position.y, SOUTH) && distances[position.x][position.y - 1] < least_distance)
        {
            least_distance = distances[position.x][position.y - 1];
            optimal_move = FORWARD;
        }
        if (!isWallInDirection(position.x, position.y, WEST) && distances[position.x - 1][position.y] < least_distance)
        {
            least_distance = distances[position.x - 1][position.y];
            optimal_move = RIGHT;
        }
        if (!isWallInDirection(position.x, position.y, EAST) && distances[position.x + 1][position.y] < least_distance)
        {
            least_distance = distances[position.x + 1][position.y];
            optimal_move = LEFT;
        }
    }
    else if (heading == WEST)
    {
        if (!isWallInDirection(position.x, position.y, WEST) && distances[position.x - 1][position.y] < least_distance)
        {
            least_distance = distances[position.x - 1][position.y];
            optimal_move = FORWARD;
        }
        if (!isWallInDirection(position.x, position.y, NORTH) && distances[position.x][position.y + 1] < least_distance)
        {
            least_distance = distances[position.x][position.y + 1];
            optimal_move = RIGHT;
        }
        if (!isWallInDirection(position.x, position.y, SOUTH) && distances[position.x][position.y - 1] < least_distance)
        {
            least_distance = distances[position.x][position.y - 1];
            optimal_move = LEFT;
        }
    }

    if (least_distance == 300)
        optimal_move = BACK;

    return optimal_move;
}

Action leftWallFollower()
{
    if (iswallFront)
    {
        if (iswallLeft)
            return RIGHT;
        return LEFT;
    }
    return FORWARD;
}
