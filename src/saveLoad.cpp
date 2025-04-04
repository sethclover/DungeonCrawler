#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <endian.h>

#include "dungeon.hpp"

char *dungeonFile;

int setupDungeonFile(char *filename) {
    char *homeDir = getenv("HOME");
    int dungeonFileLength = strlen(homeDir) + strlen("/.rlg327/") + strlen(filename) + 1;

    dungeonFile = new char[dungeonFileLength];
    // if (!dungeonFile) {
    //     errorHandle("Error: Memory allocation failed for dungeonFile");
    //     return 1;
    // }

    strcpy(dungeonFile, homeDir);
    strcat(dungeonFile, "/.rlg327/");
    strcat(dungeonFile, filename);
    return 0;
}

int loadDungeon(char *filename) {
    setupDungeonFile(filename);
    FILE *file = fopen(dungeonFile, "r");
    // if (!file) {
    //     errorHandle("Error: Failed to open file");
    //     return 1;
    // }

    char marker[12];
    fread(marker, 1, 12, file);

    uint32_t version;
    fread(&version, 4, 1, file);
    version = be32toh(version);

    uint32_t size;
    fread(&size, 4, 1, file);
    size = be32toh(size);

    uint8_t pos[2];
    fread(pos, 2, 1, file);
    player.pos.x = (int) pos[0];
    player.pos.y = (int) pos[1];

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            fread(&dungeon[i][j].hardness, 1, 1, file);
            if (dungeon[i][j].hardness == 0) {
                dungeon[i][j].type = CORRIDOR;
            }
            else {
                dungeon[i][j].type = ROCK;
            }
        }
    }

    uint16_t r = 0;
    fread(&r, 2, 1, file);
    r = be16toh(r);
    roomCount = r;

    Room *roomsLoaded = new Room[r];
    // if (!roomsLoaded) {
    //     errorHandle("Error: Memory allocation failed for roomsLoaded");
    //     return 1;
    // }
    for (int i = 0; i < r; i++) {
        fread(&roomsLoaded[i].x, 1, 1, file);
        fread(&roomsLoaded[i].y, 1, 1, file);
        fread(&roomsLoaded[i].width, 1, 1, file);
        fread(&roomsLoaded[i].height, 1, 1, file);
    }
    for (int i = 0; i < r; i++) {
        for (int j = roomsLoaded[i].y; j < roomsLoaded[i].y + roomsLoaded[i].height; j++) {
            for (int k = roomsLoaded[i].x; k < roomsLoaded[i].x + roomsLoaded[i].width; k++) {
                dungeon[j][k].type = FLOOR;
            }
        }
    }
    initRoom(roomsLoaded);
    delete[] roomsLoaded;

    uint16_t u;
    fread(&u, 2, 1, file);
    u = be16toh(u);
    upStairsCount = u;

    Pos *upStairs = new Pos[u];
    // if (!upStairs) {
    //     errorHandle("Error: Memory allocation failed for upStairs");
    //     return 1;
    // }
    for (int i = 0; i < u; i++) {
        fread(&upStairs[i].x, 1, 1, file);
        fread(&upStairs[i].y, 1, 1, file);

        dungeon[upStairs[i].y][upStairs[i].x].type = STAIR_UP;
    }

    uint16_t d;
    fread(&d, 2, 1, file);
    d = be16toh(d);
    downStairsCount = d;

    Pos *downStairs = new Pos[d];
    // if (!downStairs) {
    //     errorHandle("Error: Memory allocation failed for downStairs");
    //     return 1;
    // }
    for (int i = 0; i < d; i++) {
        fread(&downStairs[i].x, 1, 1, file);
        fread(&downStairs[i].y, 1, 1, file);

        dungeon[downStairs[i].y][downStairs[i].x].type = STAIR_DOWN;
    }

    printf("Dungeon loaded from %s\n", dungeonFile);
    fclose(file);
    delete dungeonFile;
    return 0;
}

int saveDungeon(char *filename) {
    setupDungeonFile(filename);
    FILE *file = fopen(dungeonFile, "w");
    // if (!file) {
    //     errorHandle("Error: Failed to open file");
    //     return 1;
    // }

    fwrite("RLG327-S2025", 1, 12, file);

    uint32_t version = htobe32(0);
    fwrite(&version, 4, 1, file);

    uint32_t size = htobe32(sizeof(1712 + roomCount * 4));
    fwrite(&size, 4, 1, file);

    uint8_t pos[2] = {(uint8_t) player.pos.x, (uint8_t) player.pos.y};
    fwrite(pos, 2, 1, file);

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            fwrite(&dungeon[i][j].hardness, 1, 1, file);
        }
    }

    uint16_t r = htobe16(roomCount);
    fwrite(&r, 2, 1, file);

    for (int i = 0; i < roomCount; i++) {
        uint8_t room[4] = {
            static_cast<uint8_t>(rooms[i].x),
            static_cast<uint8_t>(rooms[i].y),
            static_cast<uint8_t>(rooms[i].width),
            static_cast<uint8_t>(rooms[i].height)
        };
        fwrite(room, 4, 1, file);
    }

    uint16_t u = htobe16(upStairsCount);
    fwrite(&u, 2, 1, file);

    for (int i = 0; i < upStairsCount; i++) {
        uint8_t upStairsPos[2] = {static_cast<uint8_t>(upStairs[i].x), static_cast<uint8_t>(upStairs[i].y)};
        fwrite(upStairsPos, 2, 1, file);
    }

    uint16_t d = htobe16(downStairsCount);
    fwrite(&d, 2, 1, file);

    for (int i = 0; i < downStairsCount; i++) {
        uint8_t downStairsPos[2] = {static_cast<uint8_t>(downStairs[i].x), static_cast<uint8_t>(downStairs[i].y)};
        fwrite(downStairsPos, 2, 1, file);
    }

    printf("Dungeon saved to %s\n", dungeonFile);
    fclose(file);
    delete dungeonFile;
    return 0;
}
