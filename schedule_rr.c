#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"

struct node *head = NULL;
struct node *end = NULL;

void add(char *name, int priority, int burst) {
    Task *newTask = (Task*)malloc(sizeof(Task*));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    if (head == NULL) {
        head = (struct node*)malloc(sizeof(struct node*));
        head->task = newTask;
        end = head;
    } else {
        end->next = (struct node*)malloc(sizeof(struct node*));
        end = end->next;
        end->task = newTask;
    }
}

bool comesBefore(char *a, char *b) { return strcmp(a, b) < 0; }

// based on traverse from list.c
// finds the task whose name comes first in dictionary
Task *pickNextTask() {
  // if list is empty, nothing to do
  if (head == NULL)
    return NULL;

  struct node *temp;
  temp = head;
  Task *best_sofar = temp->task;

  while (temp != NULL) {
    if (comesBefore(temp->task->name, best_sofar->name))
      best_sofar = temp->task;
    temp = temp->next;
  }
  // delete the node from list, Task will get deleted later
  delete(&head, best_sofar);
  return best_sofar;
}


// invoke the scheduler
void schedule() {
    struct node* loopHead;
    int time = 0;
    do {
        loopHead = NULL;
        Task* curr = pickNextTask();
        while(curr != NULL) {
            if (curr-> burst <= QUANTUM) {
                run(curr, curr->burst);
                time += curr->burst;
                free(curr);
            } else {
                run(curr, QUANTUM);
                time += QUANTUM;
                curr->burst -= QUANTUM;
                if (loopHead == NULL) {
                    loopHead = (struct node*)malloc(sizeof(struct node*));
                    loopHead->task = curr;
                } else {
                    struct node *travel = loopHead;
                    while (travel->next != NULL) {
                        travel = travel->next;
                    }
                    travel->next = (struct node*)malloc(sizeof(struct node*));
                    travel = travel->next;
                    travel->task = curr;
                }
            }
            printf("        Time is now: %d\n", time);
            curr = pickNextTask();
        }
        head = loopHead;
    } while(head != NULL);
}