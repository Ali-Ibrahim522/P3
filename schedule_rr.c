#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"

struct node *head = NULL;
struct node *end = NULL;
int size = 0;

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
    size++;
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

int indexOf(char* names[], char* find) {
    for (int i = 0; i < size; i++) {
      if(strcmp(find, names[i]) == 0) return i;
    }
    return -1;
}

// invoke the scheduler
void schedule() {
    int idles = 0;
    char* names[size];
    for (int i = 0; i < size; i++) { names[i] = ""; }
    int tat[size];
    int wt[size];
    int rt[size];
    int ls[size];

    int end = 0;
    struct node* loopHead;
    bool firstLoop = true;
    int time = 0;
    do {
        loopHead = NULL;
        Task* curr = pickNextTask();
        while(curr != NULL) {
            if (curr-> burst <= QUANTUM) {
                run(curr, curr->burst);
                int ind = indexOf(names, curr->name);
                if (ind == -1) {
                    names[end] = curr->name;
                    tat[end] = time + curr->burst;
                    wt[end] = time;
                    rt[end] = time;
                    ls[end] = tat[end];
                    end++;
                } else {
                    tat[ind] = time + curr->burst;
                    wt[ind] += time - ls[ind];
                    ls[ind] = tat[ind];
                }
                time += curr->burst;
                free(curr);
            } else {
                run(curr, QUANTUM);
                int ind = indexOf(names, curr->name);
                if (ind == -1) {
                    names[end] = curr->name;
                    tat[end] = time + QUANTUM;
                    wt[end] = time;
                    rt[end] = time;
                    ls[end] = tat[end];
                    end++;
                } else {
                    tat[ind] = time + QUANTUM;
                    wt[ind] += (time - ls[ind]);
                    ls[ind] = tat[ind];
                }
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
            if (firstLoop) {
              curr = pickNextTask();
            } else {
              if (head == NULL) {
                curr = NULL;
              } else {
                curr = head->task;
                head = head->next;
              }
            }
            idles++;
        }
        head = loopHead;
        firstLoop = false;
    } while(head != NULL);

    printf("\n...");
    for (int i = 0; i < size; i++) {
      printf("| %3s ", names[i]);
    }
    printf("|\n");

    printf("TAT");
    for (int i = 0; i < size; i++) {
      printf("| %3d ", tat[i]);
    }
    printf("|\n");

    printf("WT ");
    for (int i = 0; i < size; i++) {
      printf("| %3d ", wt[i]);
    }
    printf("|\n");

    printf("RT ");
    for (int i = 0; i < size; i++) {
      printf("| %3d ", rt[i]);
    }
    printf("|\n");
    printf("CPU Utilization: %.2f%%\n", ((time / (float)(time + --idles)) * 100));
}
