#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"

struct node *head = NULL;
struct node *end = NULL;
bool roundRobin = false;
int size = 0;


void add(char *name, int priority, int burst) {
    Task *newTask = (Task*)malloc(sizeof(Task*));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;

    if (head == NULL) {
        head = (struct node*)malloc(sizeof(struct node*));
        head->task = newTask;
        head->next = NULL;
        end = head;
    } else {
        end->next = (struct node*)malloc(sizeof(struct node*));
        end = end->next;
        end->task = newTask;
        end->next = NULL;
    }
    size++;
}

bool comesBefore(Task* temp, Task* best_sofar) {
    if (temp->priority > best_sofar->priority) return true;
    if (temp->priority < best_sofar->priority) return false;
    roundRobin = true;
    return strcmp(temp->name, best_sofar->name) < 0; 
}

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
    if (comesBefore(temp->task, best_sofar))
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

struct node* buildRoundRobin(int prio) {
    struct node* rrHead = NULL;
    struct node* rrEnd = NULL;
    struct node* travel = head;
    struct node* temp = NULL;
    while (travel != NULL) {
        if (prio == travel->task->priority) {
            //printf("----|- adding task[%s]|\n", travel->task->name);
            if (rrHead == NULL) {
                rrHead = (struct node*)malloc(sizeof(struct node*));
                rrHead->task = travel->task;
                rrHead->next = NULL;
                rrEnd = rrHead;
            } else {
                rrEnd->next = (struct node*)malloc(sizeof(struct node*));
                rrEnd = rrEnd->next;
                rrEnd->task = travel->task;
                rrEnd->next = NULL;
            }
            if (travel->next == NULL) {
                delete(&head, travel->task);
                break;
            } else {
                temp = travel->next;
                delete(&head, travel->task);
                travel = temp;
            }
        } else {
            travel = travel->next;
        }
    }
    return rrHead;
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

    int time = 0;
    bool runningRR = false;
    struct node* storedHead = NULL;
    struct node* rrHead = NULL;
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
            if (!runningRR) {
                rrHead = buildRoundRobin(curr->priority);
                if (rrHead != NULL) {
                    runningRR = true;
                    storedHead = head;
                    head = rrHead;
                }
            } else if (head == NULL) {
                runningRR = false;
                head = storedHead;
            }
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
            if (!runningRR) {
                rrHead = buildRoundRobin(curr->priority);
                if (rrHead != NULL) {
                    runningRR = true;
                    storedHead = head;
                    head = rrHead;
                }
            } else if (head == NULL) {
                runningRR = false;
                head = storedHead;
            }
            if (head == NULL) {
                head = (struct node*)malloc(sizeof(struct node*));
                head->task = curr;
                head->next = NULL;
            } else {
                struct node *travel = head;
                while (travel->next != NULL) {
                    travel = travel->next;
                }
                travel->next = (struct node*)malloc(sizeof(struct node*));
                travel = travel->next;
                travel->task = curr;
                travel->next = NULL;
            }
        }
        printf("        Time is now: %d\n", time);
        idles++;
        if (runningRR) {
            if (head != NULL) {
                curr = head->task;
                head = head->next;
            } else {
                curr = NULL;
            }
        } else {
            curr = pickNextTask();
        }
        
    }

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