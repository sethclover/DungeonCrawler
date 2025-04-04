#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "dungeon.hpp"
#include "pathFinding.hpp"
#include "perlin.hpp"

Tile dungeon[MAX_HEIGHT][MAX_WIDTH];
int roomCount;
Room *rooms;
Player player;
Pos *upStairs;
int upStairsCount;
Pos *downStairs;
int downStairsCount;

Monster *monsterAt[MAX_HEIGHT][MAX_WIDTH] = {nullptr};
Monster *monsters;

Monster::Monster(Pos pos) {
    intelligent = rand() % 2;
    tunneling = rand() % 2;
    telepathic = rand() % 2;
    erratic = rand() % 2;
    speed = rand() % 16 + 5;
    this->pos = pos;
    lastSeen = {-1, -1};
}

Monster::Monster(Pos pos, char monType) {
    int num;
    monType = tolower(monType);
    if (monType >= '0' && monType <= '9') {
        num = monType - '0';
    }
    else if (monType >= 'a' && monType <= 'f') {
        num = monType - 'a' + 10;
    }
    else {
        // error
        return;
    }

    intelligent = num & 1;
    telepathic = (num >> 1) & 1;
    tunneling = (num >> 2) & 1;
    erratic = (num >> 3) & 1;
    speed = rand() % 16 + 5;
    this->pos = pos;
    lastSeen = {-1, -1};
}

void initDungeon() {
    generateHardness();

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (i == 0 || i == MAX_HEIGHT - 1 || j == 0 || j == MAX_WIDTH - 1) {
                dungeon[i][j].hardness = MAX_HARDNESS;
            }
            dungeon[i][j].type = ROCK;
        }
    }
}

void initRoom(Room *roomsLoaded) {
    rooms = new Room[roomCount];
    // if (!rooms) {
    //     errorHandle("Error: Failed to allocate memory for rooms");
    //     free(rooms);
    //     return;
    // }

    for (int i = 0; i < roomCount; i++) {
        rooms[i].width = roomsLoaded[i].width;
        rooms[i].height = roomsLoaded[i].height;
        rooms[i].x = roomsLoaded[i].x;
        rooms[i].y = roomsLoaded[i].y;
    }
}

int placeRoom(Room room) {
    for (int i = room.y - 1; i < room.y + room.height + 1; i++) {
        for (int j = room.x - 1; j < room.x + room.width + 1; j++) {
            if (dungeon[i][j].type == FLOOR) {
                return 0;
            }
        }
    }

    for (int i = room.y; i < room.y + room.height; i++) {
        for (int j = room.x; j < room.x + room.width; j++) {
            dungeon[i][j].type = FLOOR;
            dungeon[i][j].hardness = 0;
        }
    }

    return 1;
}

Room* buildRooms(int roomCount) {
    int roomsBuilt = 0;

    Room *rooms = new Room[roomCount];
    // if (!rooms) {
    //     errorHandle("Error: Failed to allocate memory for rooms");
    //     free(rooms);
    //     return NULL;
    // }

    for (int i = 0; i < ATTEMPTS; i++) {
        rooms[roomsBuilt].width = rand() % 9 + 4;
        rooms[roomsBuilt].height = rand() % 10 + 3;
        rooms[roomsBuilt].x = rand() % (MAX_WIDTH - rooms[roomsBuilt].width - 1) + 1;
        rooms[roomsBuilt].y = rand() % (MAX_HEIGHT - rooms[roomsBuilt].height - 1) + 1;

        if (placeRoom(rooms[roomsBuilt])) {
            roomsBuilt++;
        }

        if (roomsBuilt >= roomCount) {
            return rooms;
        }
    }
    
    //errorHandle("Error: Failed to build all rooms");
    delete[] rooms;
    return NULL;
}

void buildCorridors() {
    for (int i = 0 ; i < roomCount - 1; i++) {
        int x = rand() % (rooms[i].width - 2) + rooms[i].x + 1;
        int y = rand() % (rooms[i].height - 2) + rooms[i].y + 1;
        int x2 = rand() % (rooms[i + 1].width - 2) + rooms[i + 1].x + 1;
        int y2 = rand() % (rooms[i + 1].height - 2) + rooms[i + 1].y + 1;

        int xDir = (x2 - x > 0) ? 1 : -1;
        int yDir = (y2 - y > 0) ? 1 : -1;

        while (x != x2 && y != y2) {
            int dir = rand() % 5;

            if (dir == 0) {
                if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
                y += yDir;
            } 
            else {
                if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
                x += xDir;
            }
        }
        while (x != x2) {
            if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
            x += xDir;
        }
        while (y != y2) {
            if (dungeon[y][x].type != FLOOR) {
                    dungeon[y][x].type = CORRIDOR;
                    dungeon[y][x].hardness = 0;
                }
            y += yDir;
        }
    }
}

int buildStairs() {
    int xUp = rand() % rooms[0].width + rooms[0].x;
    int yUp = rand() % rooms[0].height + rooms[0].y;

    upStairsCount = 1;
    upStairs = new Pos[upStairsCount];
    // if (!upStairs) {
    //     errorHandle("Error: Failed to allocate memory for upStairs");
    //     free(upStairs);
    //     return 1;
    // }
    upStairs[0].x = xUp;
    upStairs[0].y = yUp;
    dungeon[yUp][xUp].type = STAIR_UP;

    int xDown = rand() % rooms[roomCount - 1].width + rooms[roomCount - 1].x;
    int yDown = rand() % rooms[roomCount - 1].height + rooms[roomCount - 1].y;

    downStairsCount = 1;
    downStairs = new Pos[downStairsCount];
    // if (!downStairs) {
    //     errorHandle("Error: Failed to allocate memory for downStairs");
    //     free(upStairs);
    //     free(downStairs);
    //     return 1;
    // }
    downStairs[0].x = xDown;
    downStairs[0].y = yDown;
    dungeon[yDown][xDown].type = STAIR_DOWN;

    return 0;
}

void spawnPlayer() {
    int x, y;
    int valid = 0;
    while (!valid) {
        x = rand() % (rooms[rand() % roomCount].width - 2) + rooms[rand() % roomCount].x + 1;
        y = rand() % (rooms[rand() % roomCount].height - 2) + rooms[rand() % roomCount].y + 1;

        if (dungeon[y][x].type == FLOOR) {
            valid = 1;
        }
        else {
            continue;
        }

        for (int i = 0; i < upStairsCount; i++) {
            for (int j = 0; j < roomCount; j++) {
                if (upStairs[i].x >= rooms[j].x && upStairs[i].x < rooms[j].x + rooms[j].width &&
                    upStairs[i].y >= rooms[j].y && upStairs[i].y < rooms[j].y + rooms[j].height &&
                    x >= rooms[j].x && x < rooms[j].x + rooms[j].width &&
                    y >= rooms[j].y && y < rooms[j].y + rooms[j].height) {
                        valid = 0;
                    }
            }
        }
        for (int i = 0; i < downStairsCount; i++) {
            for (int j = 0; j < roomCount; j++) {
                if (downStairs[i].x >= rooms[j].x && downStairs[i].x < rooms[j].x + rooms[j].width &&
                    downStairs[i].y >= rooms[j].y && downStairs[i].y < rooms[j].y + rooms[j].height &&
                    x >= rooms[j].x && x < rooms[j].x + rooms[j].width &&
                    y >= rooms[j].y && y < rooms[j].y + rooms[j].height) {
                        valid = 0;
                    }
            }
        }
    }

    player.pos.x = x;
    player.pos.y = y;
}

int spawnMonsterWithMonType(char monType) {
    monsters = new Monster[1];
    // if (!monsters) {
    //     errorHandle("Error: Failed to allocate memory for monsters");
    //     free(monsters);
    //     return 1;
    // }

    for (int j = 0; j < ATTEMPTS; j++) {
        int placed = 0;
        int x = rand() % MAX_WIDTH;
        int y = rand() % MAX_HEIGHT;
        if (dungeon[y][x].type != FLOOR) {
            continue;
        }
        for (int k = 0; k < roomCount; k++) {
            if ((player.pos.x >= rooms[k].x && player.pos.x <= rooms[k].x + rooms[k].width - 1 &&
                player.pos.y >= rooms[k].y && player.pos.y <= rooms[k].y + rooms[k].height - 1)) {
                continue;
            }
            else if (x >= rooms[k].x && x <= rooms[k].x + rooms[k].width - 1 &&
                        y >= rooms[k].y && y <= rooms[k].y + rooms[k].height - 1) {
                monsters[0] = Monster((Pos){x, y}, monType);
                // if (!monsters[0]) {
                //     errorHandle("Error: Failed to create monster with type %c", monType);
                //     free(monsters[0]);
                //     free(monsters);
                //     return 1;
                // }
                monsterAt[y][x] = &monsters[0];
                placed = 1;
                break;
            }
        }

        if (placed) {
            break;
        }
    }

    return 0;
}

int spawnMonsters(int numMonsters, int playerX, int playerY) {
    monsters = new Monster[numMonsters];
    // if (!monsters) {
    //     errorHandle("Error: Failed to allocate memory for monsters");
    //     free(monsters);
    //     return 1;
    // }

    for (int i = 0; i < numMonsters; i++) {
        for (int j = 0; j < ATTEMPTS; j++) {
            int placed = 0;
            int x = rand() % MAX_WIDTH;
            int y = rand() % MAX_HEIGHT;
            if (dungeon[y][x].type != FLOOR) {
                continue;
            }
            for (int k = 0; k < roomCount; k++) {
                if ((playerX >= rooms[k].x && playerX <= rooms[k].x + rooms[k].width - 1 &&
                    playerY >= rooms[k].y && playerY <= rooms[k].y + rooms[k].height - 1)) {
                    continue;
                }
                else if (x >= rooms[k].x && x <= rooms[k].x + rooms[k].width - 1 &&
                         y >= rooms[k].y && y <= rooms[k].y + rooms[k].height - 1) {
                    if (monsterAt[y][x]) {
                        continue;
                    }
                    else {
                        monsters[i] = Monster((Pos){x, y});
                        // if (!monsters[i]) {
                        //     for (int l = 0; l < i; l++) {
                        //         free(monsters[l]);
                        //     }
                        //     errorHandle("Error: Failed to create monster");
                        //     free(monsters);
                        //     return 1;
                        // }
                        monsterAt[y][x] = &monsters[i];
                        placed = 1;
                        break;
                    }
                }
            }

            if (placed) {
                break;
            }
        }
    }

    return 0;
}

void printHardness() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            int h = dungeon[i][j].hardness;
            if (h < 1) {
                printf(" ");
            }
            else if ( h < 65) {
                printf(".");
            }
            else if (h < 130) {
                printf(";");
            } 
            else if (h < 195) {
                printf("*");
            }
            else if (h < 255) {
                printf("%%");
            } 
            else {
                printf("#");
            }
        }
        printf("\n");
    }
}

void printTunnelingDistances() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (dungeon[i][j].tunnelingDist == UNREACHABLE) {
                printf(" ");
            }
            else if (dungeon[i][j].tunnelingDist == 0) {
                printf("@");
            }
            else {
                printf("%d", dungeon[i][j].tunnelingDist % 10);
            }
        }
        printf("\n");
    }
}

void printNonTunnelingDistances() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            if (dungeon[i][j].nonTunnelingDist == UNREACHABLE) {
                printf(" ");
            }
            else if (dungeon[i][j].nonTunnelingDist == 0) {
                printf("@");
            }
            else {
                printf("%d", dungeon[i][j].nonTunnelingDist % 10);
            }
        }
        printf("\n");
    }
}

int generateStructures(int numMonsters) {
    roomCount = rand() % 5 + 7;
    rooms = buildRooms(roomCount);
    if (!rooms) {
        return 1;
    }
    buildCorridors();
    if (buildStairs()) {
        delete[] rooms;
        return 1;
    }

    return 0;
}

void freeAll(int numMonsters) {
    delete[] rooms;
    delete[] upStairs;
    delete[] downStairs;
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            monsterAt[i][j] = NULL;
        }
    }
}
