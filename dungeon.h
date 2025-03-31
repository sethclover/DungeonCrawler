#pragma once

#define MAX_WIDTH 80
#define MAX_HEIGHT 21
#define MAX_HARDNESS 255

#define ATTEMPTS 1000

#define CORNER '+'
#define VERT_WALL '|'
#define HORZ_WALL '-'
#define FLOOR '.'
#define CORRIDOR '#'
#define STAIR_UP '<'
#define STAIR_DOWN '>'
#define ROCK ' ' 
#define UNREACHABLE 9999

typedef struct Room {
    int width;
    int height;
    int x;
    int y;
} Room;

typedef struct Tile {
    char type;
    int hardness;
    int tunnelingDist;
    int nonTunnelingDist;
} Tile;

typedef struct Position {
    int x;
    int y;
} Pos;

typedef struct Monster {
    int intelligent;
    int telepathic;
    int tunneling;
    int erratic;
    int speed;
    Pos pos;
    Pos lastSeen;
} Mon;

extern Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
extern int roomCount;
extern Room *rooms;
extern Pos player;
extern Pos *upStairs;
extern int upStairsCount;
extern Pos *downStairs;
extern int downStairsCount;
extern Mon *monsterAt[MAX_HEIGHT][MAX_WIDTH];
extern Mon **monsters;

void initDungeon();
void initRoom(Room *roomsLoaded);
void spawnPlayer();
void printHardness();
void printTunnelingDistances();
void printNonTunnelingDistances();
int spawnMonsterWithMonType(char monType);
int spawnMonsters(int numMonsters, int playerX, int playerY);
int generateStructures(int numMonsters);
void freeAll();
