#include <stdio.h>
#include <stdlib.h>

#include "dungeon.h"
#include "fibonacciHeap.h"

void initDistances() {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            dungeon[i][j].tunnelingDist = UNREACHABLE;
            dungeon[i][j].nonTunnelingDist = UNREACHABLE;
        }
    }
}

int tunnelingDistances() {
    FibHeap *heap = createFibHeap();
    if (!heap) {
        return 1;
    }
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};

    dungeon[player.y][player.x].tunnelingDist = 0;
    nodes[player.y][player.x] = insert(heap, 0, player);
    if (!nodes[player.y][player.x]) {
        destroyFibHeap(heap);
        return 1;
    }

    while (heap->min != NULL) {
        FibNode *minNode = extractMin(heap);
        if (!minNode) {
            destroyFibHeap(heap);
            return 1;
        }

        Pos pos = minNode->pos;
        int dist = minNode->key;

        if (dist == dungeon[pos.y][pos.x].tunnelingDist) {
            nodes[pos.y][pos.x] = NULL;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int newX = pos.x + j;
                    int newY = pos.y + i;

                    if ((i == 0 && j == 0) || dungeon[newY][newX].hardness == 255) {
                        continue;
                    }
        
                    int newDist = dist + dungeon[newY][newX].hardness / 85 + 1;
                    if (newDist < dungeon[newY][newX].tunnelingDist) {
                        dungeon[newY][newX].tunnelingDist = newDist;

                        if (nodes[newY][newX] == NULL) {
                            nodes[newY][newX] = insert(heap, newDist, (Pos){newX, newY});
                            if (!nodes[newY][newX]) {
                                destroyFibHeap(heap);
                                return 1;
                            }
                        }
                        else {
                            decreaseKey(heap, nodes[newY][newX], newDist);
                        }
                    }
                }
            }
        }

        free(minNode);        
    }

    destroyFibHeap(heap);
    return 0;
}

int nonTunnelingDistances() {
    FibHeap *heap = createFibHeap();
    if (!heap) {
        return 1;
    }
    FibNode *nodes[MAX_HEIGHT][MAX_WIDTH] = {NULL};

    dungeon[player.y][player.x].nonTunnelingDist = 0;
    nodes[player.y][player.x] = insert(heap, 0, player);
    if (!nodes[player.y][player.x]) {
        destroyFibHeap(heap);
        return 1;
    }

    while (heap->min != NULL) {
        FibNode *minNode = extractMin(heap);
        if (!minNode) {
            destroyFibHeap(heap);
            return 1;
        }
        Pos pos = minNode->pos;
        int dist = minNode->key;

        if (dist == dungeon[pos.y][pos.x].nonTunnelingDist) {
            nodes[pos.y][pos.x] = NULL;

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    int newX = pos.x + j;
                    int newY = pos.y + i;
                    
                    if ((i == 0 && j == 0) || dungeon[newY][newX].hardness > 0) {
                        continue;
                    }
        
                    int newDist = dist + 1;
                    if (newDist < dungeon[newY][newX].nonTunnelingDist) {
                        dungeon[newY][newX].nonTunnelingDist = newDist;

                        if (nodes[newY][newX] == NULL) {
                            nodes[newY][newX] = insert(heap, newDist, (Pos){newX, newY});
                            if (!nodes[newY][newX]) {
                                destroyFibHeap(heap);
                                return 1;
                            }
                        }
                        else {
                            decreaseKey(heap, nodes[newY][newX], newDist);
                        }
                    }
                }
            }
        }

        free(minNode);        
    }

    destroyFibHeap(heap);
    return 0;
}

int generateDistances() {
    initDistances();
    if (tunnelingDistances()) {
        return 1;
    }
    if (nonTunnelingDistances()) {
        return 1;
    }

    return 0;
}
