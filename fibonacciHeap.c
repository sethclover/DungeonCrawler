#include <stdlib.h>
#include <math.h>

typedef struct FibNode {
    int key;
    int degree;
    struct FibNode *parent;
    struct FibNode *child;
    struct FibNode *left;
    struct FibNode *right;
    int marked;
} FibNode;

typedef struct FibHeap {
    int numNodes;
    FibNode *min;
} FibHeap;

FibHeap *createFibHeap() {
    FibHeap *heap = (FibHeap*)malloc(sizeof(FibHeap));
    heap->numNodes = 0;
    heap->min = NULL;

    return heap;
}

FibNode *createFibNode(int key) {
    FibNode *node = (FibNode*)malloc(sizeof(FibNode)); // FREEEEE
    node->key = key;
    node->degree = 0;
    node->parent = NULL;
    node->child = NULL;
    node->left = node;
    node->right = node;
    node->marked = 0;

    return node;
}

void *insert(FibHeap *heap, int key) {
    FibNode *node = createFibNode(key);
    if (heap->min == NULL) {
        heap->min = node;
    }
    else {
        node->right = heap->min->right;
        node->left = heap->min;
        heap->min->right->left = node;
        heap->min->right = node;

        if (node->key < heap->min->key) {
            heap->min = node;
        }
    }

    heap->numNodes++;
}

FibNode *getMin(FibHeap *heap) {
    return heap->min;
}

void consolidate(FibHeap *heap) {
    int maxDegree = (int)(log(heap->numNodes - 1) / log(2)) + 1;
    FibNode **A = (FibNode**)calloc(maxDegree, sizeof(FibNode*));

    FibNode **nodes = (FibNode**)malloc((heap->numNodes - 1) * sizeof(FibNode*));
    int count = 0;

    if (heap->min) {
        FibNode *curr = heap->min;
        do {
            nodes[count++] = curr;
            curr = curr->right;
        } while (curr != heap->min);
    }

    for (int i = 0; i < count; i++) {
        FibNode *node = nodes[i];
        int degree = node->degree;

        while (A[degree] != NULL) {
            FibNode *compareNode = A[degree];

            if (node->key > compareNode->key) {
                FibNode *temp = node;
                node = compareNode;
                compareNode = temp;
            }

            compareNode->left->right = compareNode->right;
            compareNode->right->left = compareNode->left;
            compareNode->parent = node;
            compareNode->marked = 0;

            if (node->child == NULL) {
                node->child = compareNode;
                compareNode->left = compareNode;
                compareNode->right = compareNode;
            }
            else {
                compareNode->right = node->child->right;
                compareNode->left = node->child;
                node->child->right->left = compareNode;
                node->child->right = compareNode;
            }

            node->degree++;
            A[degree] = NULL;
            degree++;   
        }
        A[degree] = node;
    }

    heap->min = NULL;
    for (int i = 0; i < maxDegree; i++) {
        if (A[i] != NULL) {
            if (heap->min == NULL) {
                heap->min = A[i];
                heap->min->left = heap->min;
                heap->min->right = heap->min;
            }
            else {
                A[i]->right = heap->min->right;
                A[i]->left = heap->min;
                heap->min->right->left = A[i];
                heap->min->right = A[i];

                if (A[i]->key < heap->min->key) {
                    heap->min = A[i];
                }
            }
        }
    }

    free(A);
    free(nodes);
}

FibNode *extractMin(FibHeap *heap) {
    FibNode *minNode = heap->min;

    if (minNode != NULL) {
        if (minNode->child != NULL) {
            FibNode *child = minNode->child;

            do {
                FibNode *next = child->right;
                child->parent = NULL;
                child->right = heap->min->right;
                child->left = heap->min;
                heap->min->right->left = child;
                heap->min->right = child;
                child = next;
            } while (child != minNode->child);
        }
        minNode->left->right = minNode->right;
        minNode->right->left = minNode->left;

        if (minNode == minNode->right) {
            heap->min = NULL;
        }
        else {
            heap->min = minNode->right;
            consolidate(heap);
        }

        heap->numNodes--;
    }

    return minNode;
}

void cut(FibHeap *heap, FibNode *node, FibNode *parent) {
    if (node == node->right) {
        parent->child = NULL;
    }
    else {
        node->left->right = node->right;
        node->right->left = node->left;

        if (parent->child == node) {
            parent->child = node->right;
        }
    }
    parent->degree--;

    node->left = heap->min;
    node->right = heap->min->right;
    heap->min->right->left = node;
    heap->min->right = node;
    node->parent = NULL;
    node->marked = 0;
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}

void cascadingCut(FibHeap *heap, FibNode *node) {
    FibNode *parent = node->parent;
    if (parent != NULL) {
        if (!node->marked) {
            node->marked = 1;
        }
        else {
            cut(heap, node, parent);
            cascadingCut(heap, parent);
        }
    }
}

void decreaseKey(FibHeap *heap, FibNode *node, int newKey) {
    if (newKey > node->key) {
        return;
    }

    node->key = newKey;
    FibNode *parent = node->parent;
    if (parent != NULL && node->key < parent->key) {
        cut(heap, node, parent);
        cascadingCut(heap, parent);
    }
    if (node->key < heap->min->key) {
        heap->min = node;
    }
}
