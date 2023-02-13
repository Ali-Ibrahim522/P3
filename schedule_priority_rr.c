/*
  created by Ali Ibrahim

  class to process tasks using the priority round robin algorithm
*/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"
// nodes keeping treack of start and end of list
struct node *head = NULL;
struct node *end = NULL;
// to see if we are running a round robin of tasks at the moment
bool roundRobin = false;
// num of tasks to be processed
int size = 0;

// adds a new task to the list
void add(char *name, int priority, int burst) {
    Task *newTask = (Task*)malloc(sizeof(Task*));
    newTask->name = name;
    newTask->priority = priority;
    newTask->burst = burst;
    // if the list is empty
    if (head == NULL) {
        head = (struct node*)malloc(sizeof(struct node*));
        head->task = newTask;
        head->next = NULL;
        end = head;
    } else {
        // if not empty, add at the end of the list
        end->next = (struct node*)malloc(sizeof(struct node*));
        end = end->next;
        end->task = newTask;
        end->next = NULL;
    }
    size++;
}

//picks process with the highest priority, or alphabetical order
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

//finds the index of a task in the info table, returns -1 if not found
int indexOf(char* names[], char* find) {
    for (int i = 0; i < size; i++) {
      if(strcmp(find, names[i]) == 0) return i;
    }
    return -1;
}

//builds a list of tasks that have an equal priority to the one passed in
struct node* buildRoundRobin(int prio) {
    struct node* rrHead = NULL;
    struct node* rrEnd = NULL;
    struct node* travel = head;
    struct node* temp = NULL;
    while (travel != NULL) {
        if (prio == travel->task->priority) {
            // if round robin list is empty
            if (rrHead == NULL) {
                rrHead = (struct node*)malloc(sizeof(struct node*));
                rrHead->task = travel->task;
                rrHead->next = NULL;
                rrEnd = rrHead;
            } else {
                //if the list is not empty
                rrEnd->next = (struct node*)malloc(sizeof(struct node*));
                rrEnd = rrEnd->next;
                rrEnd->task = travel->task;
                rrEnd->next = NULL;
            }
            if (travel->next == NULL) {
                // if there are more no more tasks to check, end loop
                delete(&head, travel->task);
                break;
            } else {
                // else delete the curr task and move on
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
    // num of times the CPu goes idle
    int idles = 0;
    // arrays for the info table
    char* names[size];
    for (int i = 0; i < size; i++) { names[i] = ""; }
    int tat[size];
    int wt[size];
    int rt[size];
    int ls[size];
    int end = 0;

    // the current time when processing tasks
    int time = 0;
    //if we are running through the round robin list
    bool runningRR = false;
    // temp node to hold the location of the original list when we move to the rr list
    struct node* storedHead = NULL;
    //head node for the rr list
    struct node* rrHead = NULL;
    // while there is a task to process
    Task* curr = pickNextTask();
    while(curr != NULL) {
        // if the task will be finished this quantum
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
            // checks to see if tasks should be ran as round robin
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
            //  if the task will not be finished this quantum
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
            // checks to see if tasks should be ran as round robin
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
            // adds the tasks back into the current list
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
        //idle caused by switching tasks
        idles++;
        //moves onto the next task
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
    // printing info
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
    // printing cpu util
    printf("|\n");
    printf("CPU Utilization: %.2f%%\n", ((time / (float)(time + --idles)) * 100));
}