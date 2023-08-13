#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>

#define MAX_AUTO 512
// AVL

typedef struct carList {
    int capacity;
    int numCars;
    int *cars;
} carList;

// stations are organized in an AVL
typedef struct station {
    int distance;
    carList *carPool;
    struct station *left;
    struct station *right;
    struct station *parent;
    int maxAutonomy;

    bool visited;
    int minWeight;
    int steps;
    struct station *pathPrevious;

    int height;
} station;

carList *createCarPool() {
    carList *newCarPool = (carList *)malloc(sizeof(carList));
    newCarPool->capacity = MAX_AUTO;
    newCarPool->numCars = 0;
    newCarPool->cars = (int *)malloc(newCarPool->capacity * sizeof(int));

    for (int i = 0; i < newCarPool->capacity; i++) {
        newCarPool->cars[i] = 0;
    }

    return newCarPool;
}

void freeCarPool(carList **carPoolToRemove) {
    if (*carPoolToRemove) {
        free((*carPoolToRemove)->cars);
        free(*carPoolToRemove);
        *carPoolToRemove = NULL;
    }
}

void printTreeDetails(station *root) {
    if (root == NULL) {
        return;
    }
    printTreeDetails(root->left);

    printf("Distance: %d\n", root->distance);
    if (root->parent)
        printf("Parent: %d\n", root->parent->distance);
    else
        printf("Parent: NULL\n");

    if (root->left)
        printf("Left: %d\n", root->left->distance);
    else
        printf("Left: NULL\n");

    if (root->right)
        printf("Right: %d\n", root->right->distance);
    else
        printf("Right: NULL\n");
    // if (root->deleted)
    //     printf("DELETED\n");
    // printf("maxAut: %d\n", root->maxAutonomy);
    // printf("weight: %d\n", root->minWeight);
    // printf("steps: %d\n", root->steps);
    // if (root->pathPrevious)
    //     printf("previous: %d\n", root->pathPrevious->distance);
    // else
    //     printf("previous: \n");
    // printCars(root->cars);
    // printf("Reachable Nodes: ");
    // printReachableNodes(root->reachable);
    // printf("Reachable By Nodes: ");
    // printReachableNodes(root->reachableBy);
    printf("\n");

    printTreeDetails(root->right);
}

void freeTree(station **root) {
    if (*root != NULL) {
        freeTree(&(*root)->left);
        freeTree(&(*root)->right);
        freeCarPool(&(*root)->carPool);
        // freeCarList(&(*root)->cars);
        // freeReachList(&(*root)->reachable);
        // freeReachList(&(*root)->reachableBy);
        free(*root);
    }
}

void resetVisitedStations(station *root) {
    if (root != NULL) {
        resetVisitedStations(root->left);
        root->visited = false;
        root->minWeight = INT_MAX;
        root->steps = INT_MAX;
        root->pathPrevious = NULL;
        // root->alreadyInQueue = false;
        // freeReachList(&root->reachable);
        // root->reachable = NULL;
        resetVisitedStations(root->right);
    }
}

station *findStation(station *root, int distance) {
    while (root != NULL) {
        if (root->distance == distance) {
            return root;
        } else if (distance < root->distance) {
            root = root->left;
        } else {
            root = root->right;
        }
    }
    return NULL;
}

// Helper function to get the height of a node
int getHeight(station *node) {
    if (node == NULL) {
        return 0;
    }
    return node->height;
}

// Helper function to update the height of a node
void updateHeight(station *node) {
    if (node == NULL) {
        return;
    }
    int leftHeight = getHeight(node->left);
    int rightHeight = getHeight(node->right);
    node->height = 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// Helper function to perform a right rotation
station *rotateRight(station *y) {
    station *x = y->left;
    station *T2 = x->right;

    x->right = y;
    y->left = T2;

    // Update parent pointers
    x->parent = y->parent;
    y->parent = x;
    if (T2 != NULL) {
        T2->parent = y;
    }

    updateHeight(y);
    updateHeight(x);

    return x;
}

// Helper function to perform a left rotation
station *rotateLeft(station *x) {
    station *y = x->right;
    station *T2 = y->left;

    y->left = x;
    x->right = T2;

    // Update parent pointers
    y->parent = x->parent;
    x->parent = y;
    if (T2 != NULL) {
        T2->parent = x;
    }

    updateHeight(x);
    updateHeight(y);

    return y;
}

// Helper function to get the balance factor of a node
int getBalanceFactor(station *node) {
    if (node == NULL) {
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}

// Recursive function to insert a station into an AVL tree
station *insertStationInTreeAVL(station *root, station *newStation) {
    // Perform the standard BST insertion
    if (root == NULL) {
        newStation->parent = NULL;
        return newStation;
    }

    if (newStation->distance < root->distance) {
        root->left = insertStationInTreeAVL(root->left, newStation);
        root->left->parent = root;
    } else if (newStation->distance > root->distance) {
        root->right = insertStationInTreeAVL(root->right, newStation);
        root->right->parent = root;
    }

    // Update height of the current node
    updateHeight(root);

    // Get the balance factor of this node
    int balance = getBalanceFactor(root);

    // Left Heavy
    if (balance > 1) {
        if (newStation->distance < root->left->distance) {
            // Left-Left case: Perform a right rotation
            return rotateRight(root);
        } else {
            // Left-Right case: Perform a left rotation on left child and then a right rotation on root
            root->left = rotateLeft(root->left);
            return rotateRight(root);
        }
    }

    // Right Heavy
    if (balance < -1) {
        if (newStation->distance > root->right->distance) {
            // Right-Right case: Perform a left rotation
            return rotateLeft(root);
        } else {
            // Right-Left case: Perform a right rotation on right child and then a left rotation on root
            root->right = rotateRight(root->right);
            return rotateLeft(root);
        }
    }

    // Return the unchanged node
    return root;
}

bool insertOrUpdateStationInTree(station *bkpRoot, station **root, station *newStation) {
    if (newStation == NULL) {
        return false;
    }

    // int initialHeight = (*root) ? (*root)->height : 0;

    // Search for the existing node with the same distance
    station *existingNode = findStation(*root, newStation->distance);

    if (existingNode) {
        return false;
    } else {
        // Node doesn't exist, perform insertion

        *root = insertStationInTreeAVL(*root, newStation);

        return true;
        // int finalHeight = (*root) ? (*root)->height : 0;
        // // If height increased, a new node was successfully inserted or updated
        // return finalHeight > initialHeight;
    }
}

char *addStation(station **root, int dist, int numCars, int *cars) {
    if (numCars > MAX_AUTO) {
        return "non aggiunta\n";
    }

    station *newStation = (station *)malloc(sizeof(station));
    if (!newStation) {
        return "memory allocation error\n";
    }

    newStation->distance = dist;
    // newStation->reachable = NULL;
    newStation->left = NULL;
    newStation->right = NULL;
    newStation->carPool = createCarPool();  // Initialize the car list to NULL
    // newStation->numCars = 0;                // Initialize the number of cars to 0
    newStation->maxAutonomy = 0;  // Initialize max autonomy to 0
    // newStation->deleted = false;

    newStation->visited = false;
    newStation->minWeight = INT_MAX;
    newStation->pathPrevious = NULL;
    newStation->steps = INT_MAX;

    newStation->height = 0;

    for (int i = 0; i < numCars && i < MAX_AUTO; i++) {
        newStation->carPool->cars[i] = cars[i];
        newStation->carPool->numCars++;

        if (cars[i] > newStation->maxAutonomy) {
            newStation->maxAutonomy = cars[i];
        }
    }

    if (insertOrUpdateStationInTree(*root, root, newStation)) {
        return "aggiunta\n";
    } else {
        // freeCarList(&newStation->cars);
        freeCarPool(&newStation->carPool);
        free(newStation);
        return "non aggiunta\n";
    }
}

// Function to find the node with the smallest value in a given AVL tree
station *minValueNode(station *node) {
    if (node == NULL) {
        return NULL;
    }
    // station *current = node;
    while (node->left != NULL) {
        node = node->left;
    }
    return node;
}

station *maxValueNode(station *node) {
    if (node == NULL) {
        return NULL;
    }

    while (node->right != NULL) {
        node = node->right;
    }

    return node;
}

// Recursive function to remove a station from an AVL tree
void removeStationFromTreeAVL(station **root, int distance) {
    if (*root == NULL) {
        printf("non demolita\n");
        return;
    }

    if (distance < (*root)->distance) {
        removeStationFromTreeAVL(&(*root)->left, distance);
    } else if (distance > (*root)->distance) {
        removeStationFromTreeAVL(&(*root)->right, distance);
    } else {
        if ((*root)->left == NULL || (*root)->right == NULL) {
            station *temp = (*root)->left ? (*root)->left : (*root)->right;

            if (temp == NULL) {
                freeCarPool(&(*root)->carPool);
                free(*root);
                *root = NULL;
            } else {
                // *root = *temp;  // Copy the contents of the non-empty child
                (*root)->distance = temp->distance;
                // freeCarList(&(*root)->cars);
                freeCarPool(&(*root)->carPool);
                // freeReachList(&(*root)->reachable);
                (*root)->carPool = temp->carPool;
                // (*root)->deleted = temp->deleted;
                (*root)->height = temp->height;
                // (*root)->alreadyInQueue = temp->alreadyInQueue;
                (*root)->maxAutonomy = temp->maxAutonomy;
                (*root)->minWeight = temp->minWeight;
                // (*root)->numCars = temp->numCars;
                (*root)->pathPrevious = temp->pathPrevious;
                // (*root)->reachable = temp->reachable;
                (*root)->steps = temp->steps;
                (*root)->visited = temp->visited;
                (*root)->left = temp->left;
                (*root)->right = temp->right;
            }
            free(temp);
            printf("demolita\n");

        } else {
            // Node with two children, get the inorder successor (smallest in the right subtree)
            station *temp = minValueNode((*root)->right);

            // Copy the inorder successor's data to this node
            (*root)->distance = temp->distance;
            // freeCarList(&(*root)->cars);
            freeCarPool(&(*root)->carPool);
            // freeReachList(&(*root)->reachable);
            (*root)->carPool = temp->carPool;
            // (*root)->deleted = temp->deleted;
            // left right height remain the same
            // (*root)->alreadyInQueue = temp->alreadyInQueue;
            (*root)->maxAutonomy = temp->maxAutonomy;
            (*root)->minWeight = temp->minWeight;
            // (*root)->numCars = temp->numCars;
            (*root)->pathPrevious = temp->pathPrevious;
            // (*root)->reachable = temp->reachable;
            (*root)->steps = temp->steps;
            (*root)->visited = temp->visited;

            // remove pointer cars and reachable
            temp->carPool = NULL;
            // temp->reachable = NULL;

            // Update parent pointer for the successor node's parent
            // if (temp->parent == *root) {
            //     (*root)->right = temp->right;  // Update right pointer
            // } else {
            //     temp->parent->left = temp->right;  // Update left pointer of successor's parent
            // }

            // Delete the inorder successor
            removeStationFromTreeAVL(&(*root)->right, temp->distance);
            // printf("demolita\n");

            // Update parent pointers for the current subtree
            if ((*root)->left) {
                (*root)->left->parent = *root;
            }
            if ((*root)->right) {
                (*root)->right->parent = *root;
            }
        }
    }

    // If the tree had only one node, then return
    if ((*root) == NULL) {
        return;
    }

    // Update height of the current node
    updateHeight((*root));

    // Get the balance factor of this node
    int balance = getBalanceFactor((*root));

    // Left Heavy
    if (balance > 1) {
        if (getBalanceFactor((*root)->left) >= 0) {
            // Left-Left case: Perform a right rotation
            *root = rotateRight(*root);
        } else {
            // Left-Right case: Perform a left rotation on left child and then a right rotation on *root
            (*root)->left = rotateLeft((*root)->left);
            *root = rotateRight(*root);
        }
    }

    // Right Heavy
    if (balance < -1) {
        if (getBalanceFactor((*root)->right) <= 0) {
            // Right-Right case: Perform a left rotation
            *root = rotateLeft(*root);
        } else {
            // Right-Left case: Perform a right rotation on right child and then a left rotation on *root
            (*root)->right = rotateRight((*root)->right);
            *root = rotateLeft(*root);
        }
    }

    if ((*root)->left) {
        (*root)->left->parent = *root;
    }
    if ((*root)->right) {
        (*root)->right->parent = *root;
    }
}

char *addCar(station **root, int distance, int carAutonomy) {
    station *current = findStation(*root, distance);
    if (!current) {
        return "non aggiunta\n";
    }
    if (current->carPool->numCars < MAX_AUTO) {
        current->carPool->cars[current->carPool->numCars] = carAutonomy;
        current->carPool->numCars++;

        if (carAutonomy > current->maxAutonomy) {
            current->maxAutonomy = carAutonomy;
        }
        return "aggiunta\n";
    } else {
        return "non aggiunta\n";
    }
}

char *removeCar(station **root, int distance, int carAutonomy) {
    station *current = findStation(*root, distance);
    if (!current) {
        return "non rottamata\n";
    }

    int newMaxAutonomy = 0;
    bool removed = false;
    for (int i = 0; i < current->carPool->numCars; i++) {
        if (!removed && current->carPool->cars[i] == carAutonomy) {
            if (i < current->carPool->numCars - 1)
                current->carPool->cars[i] = current->carPool->cars[current->carPool->numCars - 1];
            else
                current->carPool->cars[i] = 0;
            current->carPool->numCars--;
            removed = true;
        }
        if (current->carPool->cars[i] > newMaxAutonomy)
            newMaxAutonomy = current->carPool->cars[i];
    }
    current->maxAutonomy = newMaxAutonomy;

    if (removed)
        return "rottamata\n";
    else
        return "non rottamata\n";
}

typedef struct queueNode {
    station *station;
} queueNode;

typedef struct PriorityQueue {
    queueNode **heapArray;
    int capacity;
    int size;
} PriorityQueue;

PriorityQueue *createPriorityQueue(int capacity) {
    PriorityQueue *pq = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    pq->capacity = capacity;
    pq->size = 0;
    pq->heapArray = (queueNode **)malloc(capacity * sizeof(queueNode *));

    // Initialize heapArray pointers
    for (int i = 0; i < capacity; i++) {
        pq->heapArray[i] = NULL;
    }

    return pq;
}

void swap(queueNode **a, queueNode **b) {
    queueNode *temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(PriorityQueue *pq, int index) {
    while (index > 0) {
        int parentIndex = (index - 1) / 2;

        if (pq->heapArray[parentIndex] != NULL &&
            (pq->heapArray[parentIndex]->station->steps > pq->heapArray[index]->station->steps ||
             (pq->heapArray[parentIndex]->station->steps == pq->heapArray[index]->station->steps &&
              pq->heapArray[parentIndex]->station->minWeight > pq->heapArray[index]->station->minWeight) ||
             (pq->heapArray[parentIndex]->station->steps == pq->heapArray[index]->station->steps &&
              pq->heapArray[parentIndex]->station->minWeight == pq->heapArray[index]->station->minWeight &&
              pq->heapArray[parentIndex]->station->distance > pq->heapArray[index]->station->distance))) {
            swap(&(pq->heapArray[parentIndex]), &(pq->heapArray[index]));
            index = parentIndex;
        } else {
            break;
        }
    }
}

void resizeArray(PriorityQueue *pq, int newCapacity) {
    pq->heapArray = (queueNode **)realloc(pq->heapArray, newCapacity * sizeof(queueNode *));
    // Initialize heapArray pointers
    for (int i = pq->capacity; i < newCapacity; i++) {
        pq->heapArray[i] = NULL;
    }
    pq->capacity = newCapacity;
}

void insertInQueue(PriorityQueue *pq, station *node) {
    if (pq->size == pq->capacity) {
        // Resize the array or perform other actions if needed
        resizeArray(pq, pq->capacity * 2);
        // return;
    }

    queueNode *newNode = (queueNode *)malloc(sizeof(queueNode));
    newNode->station = node;

    pq->heapArray[pq->size] = newNode;
    pq->size++;

    heapifyUp(pq, pq->size - 1);
}

queueNode *pop(PriorityQueue *pq) {
    if (pq->size == 0) {
        return NULL;
    }

    queueNode *minNode = pq->heapArray[0];
    pq->heapArray[0] = pq->heapArray[pq->size - 1];
    pq->heapArray[pq->size - 1] = NULL;  // Clear the last position
    pq->size--;

    // Perform heapify-down operation to maintain heap property
    int currentIndex = 0;
    while (1) {
        int leftChild = 2 * currentIndex + 1;
        int rightChild = 2 * currentIndex + 2;
        int smallest = currentIndex;

        if (leftChild < pq->size &&
            (pq->heapArray[leftChild]->station->steps < pq->heapArray[smallest]->station->steps ||
             (pq->heapArray[leftChild]->station->steps == pq->heapArray[smallest]->station->steps &&
              pq->heapArray[leftChild]->station->minWeight < pq->heapArray[smallest]->station->minWeight) ||
             (pq->heapArray[leftChild]->station->steps == pq->heapArray[smallest]->station->steps &&
              pq->heapArray[leftChild]->station->minWeight == pq->heapArray[smallest]->station->minWeight &&
              pq->heapArray[leftChild]->station->distance < pq->heapArray[smallest]->station->distance))) {
            smallest = leftChild;
        }

        if (rightChild < pq->size &&
            (pq->heapArray[rightChild]->station->steps < pq->heapArray[smallest]->station->steps ||
             (pq->heapArray[rightChild]->station->steps == pq->heapArray[smallest]->station->steps &&
              pq->heapArray[rightChild]->station->minWeight < pq->heapArray[smallest]->station->minWeight) ||
             (pq->heapArray[rightChild]->station->steps == pq->heapArray[smallest]->station->steps &&
              pq->heapArray[rightChild]->station->minWeight == pq->heapArray[smallest]->station->minWeight &&
              pq->heapArray[rightChild]->station->distance < pq->heapArray[smallest]->station->distance))) {
            smallest = rightChild;
        }

        if (smallest != currentIndex) {
            swap(&(pq->heapArray[currentIndex]), &(pq->heapArray[smallest]));
            currentIndex = smallest;
        } else {
            break;
        }
    }

    return minNode;
}

station *getSuccessor(station *node) {
    if (node == NULL) {
        return NULL;
    }

    // If the node has a right subtree, then the successor is the leftmost node in that subtree
    if (node->right != NULL) {
        return minValueNode(node->right);
    }

    // Otherwise, traverse up the tree to find the first ancestor whose left child is also an ancestor of the given node
    station *parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

station *getPredecessor(station *node) {
    if (node == NULL) {
        return NULL;
    }

    // If the node has a left subtree, then the predecessor is the rightmost node in that subtree
    if (node->left != NULL) {
        return maxValueNode(node->left);
    }

    // Otherwise, traverse up the tree to find the first ancestor whose right child is also an ancestor of the given node
    station *parent = node->parent;
    while (parent != NULL && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

void insertReachableStationsInQueue(station *root, station *toCheckStation, PriorityQueue *headQueue, bool *found, int start, int finish, bool firstInsert, station **currentBiggest, station **currentLowest) {
    if (toCheckStation->distance == finish) {
        *found = true;
    }

    toCheckStation->visited = true;

    if (!(*found)) {
        station *temp = NULL;
        if (start < finish)
            temp = getSuccessor(*currentBiggest);
        else
            temp = getPredecessor(*currentLowest);

        while (temp && abs(temp->distance - toCheckStation->distance) <= toCheckStation->maxAutonomy) {
            if (!temp->visited) {
                if (firstInsert) {
                    temp->steps = toCheckStation->steps + 1;
                    temp->minWeight = temp->distance;
                    temp->pathPrevious = toCheckStation;

                    insertInQueue(headQueue, temp);
                } else {
                    int checkValue = 0;
                    if (temp->distance != finish)
                        checkValue = (toCheckStation->minWeight < temp->distance) ? toCheckStation->minWeight : temp->distance;
                    else
                        checkValue = toCheckStation->minWeight;

                    if (temp->steps > toCheckStation->steps + 1) {
                        temp->steps = toCheckStation->steps + 1;
                        temp->minWeight = checkValue;
                        temp->pathPrevious = toCheckStation;

                        insertInQueue(headQueue, temp);

                    } else if (temp->steps == toCheckStation->steps + 1) {
                        if (temp->minWeight > checkValue) {
                            temp->steps = toCheckStation->steps + 1;
                            temp->minWeight = checkValue;
                            temp->pathPrevious = toCheckStation;

                            insertInQueue(headQueue, temp);

                        } else if (temp->minWeight == checkValue && temp->pathPrevious->distance > toCheckStation->distance) {
                            temp->steps = toCheckStation->steps + 1;
                            temp->minWeight = checkValue;
                            temp->pathPrevious = toCheckStation;

                            insertInQueue(headQueue, temp);
                        }
                    }
                }
            }

            if ((*currentBiggest)->distance < temp->distance)
                *currentBiggest = temp;
            if ((*currentLowest)->distance > temp->distance)
                *currentLowest = temp;

            if (start < finish)
                temp = getSuccessor(*currentBiggest);
            else
                temp = getPredecessor(*currentLowest);
        }
    }
}

void findPathHelper(station *root, station *startStation, bool *found, PriorityQueue *headQueue, int start, int finish) {
    station *currentBiggest = startStation;
    station *currentLowest = startStation;
    // check start station and insert in queue the reachable stations
    insertReachableStationsInQueue(root, startStation, headQueue, found, start, finish, true, &currentBiggest, &currentLowest);

    // pop stations from queue and check them
    while (headQueue->size != 0) {
        queueNode *extracted = pop(headQueue);
        if (extracted) {
            station *toCheckStation = extracted->station;
            free(extracted);

            // printf("biggest: %d lowest: %d\n", currentBiggest->distance, currentLowest->distance);
            insertReachableStationsInQueue(root, toCheckStation, headQueue, found, start, finish, false, &currentBiggest, &currentLowest);
        }
    }
}

void printPath3(station *startStation, station *finishStation, int finish) {
    if (finishStation->distance == startStation->distance) {
        printf("%d ", startStation->distance);
        return;
    }
    printPath3(startStation, finishStation->pathPrevious, finish);
    printf("%d", finishStation->distance);
    if (finishStation->distance != finish) {
        printf(" ");
    } else {
        printf("\n");
    }
}

void findPath(station *root, int start, int finish) {
    if (start == finish) {
        printf("%d\n", start);
        return;
    }
    station *startStation = findStation(root, start);
    station *finishStation = findStation(root, finish);

    if (startStation == NULL || finishStation == NULL) {
        printf("nessun percorso\n");
        return;
    }
    if (abs(start - finish) <= startStation->maxAutonomy) {
        printf("%d %d", start, finish);
        return;
    }

    PriorityQueue *headQueue = createPriorityQueue(100);

    bool found = false;

    startStation->minWeight = 0;
    startStation->steps = 0;
    findPathHelper(root, startStation, &found, headQueue, start, finish);
    if (!found) {
        printf("nessun percorso\n");
    } else {
        printPath3(startStation, finishStation, finish);
    }

    resetVisitedStations(root);
}

int main() {
    char command[19];
    int dist;
    int numCars;
    int cars[MAX_AUTO];

    int singleCar;
    int carAutonomy;

    int start;
    int finish;

    station *root = NULL;

    while (scanf("%18s", command) == 1) {
        if (strcmp(command, "aggiungi-stazione") == 0) {
            if (!scanf("%d", &dist)) {
                printf("Failed getting dist in aggiungi-stazione\n");
                return 1;
            }

            if (!scanf("%d", &numCars)) {
                printf("Failed getting numcars in aggiungi-stazione\n");
                return 1;
            }

            for (int i = 0; i < numCars; i++) {
                if (!scanf("%d", &cars[i])) {
                    printf("Failed getting car in aggiungi-stazione\n");
                    return 1;
                }
            }

            printf("%s", addStation(&root, dist, numCars, cars));

        } else if (strcmp(command, "aggiungi-auto") == 0) {
            if (!scanf("%d", &dist)) {
                printf("Failed getting dist in aggiungi-auto\n");
                return 1;
            }

            if (!scanf("%d", &singleCar)) {
                printf("Failed getting car in aggiungi-auto\n");
                return 1;
            }

            printf("%s", addCar(&root, dist, singleCar));

        } else if (strcmp(command, "demolisci-stazione") == 0) {
            if (!scanf("%d", &dist)) {
                printf("Failed getting dist in demolisci-stazione\n");
                return 1;
            }
            // printf("%s", removeStation(&root, dist));
            removeStationFromTreeAVL(&root, dist);

        } else if (strcmp(command, "rottama-auto") == 0) {
            if (!scanf("%d", &dist)) {
                printf("Failed getting dist in rottama-auto\n");
                return 1;
            }

            if (!scanf("%d", &carAutonomy)) {
                printf("Failed getting carAutonomy in rottama-auto\n");
                return 1;
            }
            printf("%s", removeCar(&root, dist, carAutonomy));

        } else if (strcmp(command, "pianifica-percorso") == 0) {
            if (!scanf("%d", &start)) {
                printf("Failed getting start in pianifica-percorso\n");
                return 1;
            }

            if (!scanf("%d", &finish)) {
                printf("Failed getting finish in pianifica-percorso\n");
                return 1;
            }
            findPath(root, start, finish);

        } else {
            printf("Comando non riconosciuto\n");
            break;
        }
    }
    freeTree(&root);

    return 0;
}
