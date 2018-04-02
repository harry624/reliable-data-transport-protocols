#include "../include/simulator.h"

// A structure to represent a queue
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    struct pkt* array;
};

// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity);

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue);

// Queue is empty when size is 0
int isEmpty(struct Queue* queue);

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct pkt *item);

// Function to remove an item from queue.
// It changes front and size
struct pkt* dequeue(struct Queue* queue);

// Function to get front of queue
struct pkt* front(struct Queue* queue);

// Function to get rear of queue
struct pkt* rear(struct Queue* queue);
