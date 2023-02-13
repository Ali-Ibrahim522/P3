/*
  created by Ali Ibrahim

  class to process tasks using the round robin algorithm
*/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"
// pointers for the start and end of the list
struct node *head = NULL;
struct node *end = NULL;
// how many tasks are to be processed
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
        end = head;
    } else {
        // if the list isn't empty, add task at end of list
        end->next = (struct node*)malloc(sizeof(struct node*));
        end = end->next;
        end->task = newTask;
    }
    size++;
}

//chooses tasks in alphabetical order
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

//finds the index of a task in the names array, if it doesnt exist returns -1
int indexOf(char* names[], char* find) {
    for (int i = 0; i < size; i++) {
      if(strcmp(find, names[i]) == 0) return i;
    }
    return -1;
}

// invoke the scheduler
void schedule() {
    // number of times the CPU goes idle
    int idles = 0;
    // arrays for the info table
    char* names[size];
    for (int i = 0; i < size; i++) { names[i] = ""; }
    int tat[size];
    int wt[size];
    int rt[size];
    int ls[size];

    // keeps track of the earlest open index in the info table
    int end = 0;
    // list head, list keeps track of processed tasks to come back to 
    struct node* loopHead;
    // if we are processing tasks for the first time
    bool firstLoop = true;
    // the current time when processing tasks
    int time = 0;
    do {
        loopHead = NULL;
        // while there is a task to process
        Task* curr = pickNextTask();
        while(curr != NULL) {
            // if the curent task will finish in this quantum 
            if (curr-> burst <= QUANTUM) {
                run(curr, curr->burst);
                int ind = indexOf(names, curr->name);
                // updating info table
                if (ind == -1) {
                    // if the task hasen;t been processed yet
                    names[end] = curr->name;
                    tat[end] = time + curr->burst;
                    wt[end] = time;
                    rt[end] = time;
                    ls[end] = tat[end];
                    end++;
                } else {
                    // if it has been processed before
                    tat[ind] = time + curr->burst;
                    wt[ind] += time - ls[ind];
                    ls[ind] = tat[ind];
                }
                time += curr->burst;
                free(curr);
            } else {
                // if the task wont be finished this run
                run(curr, QUANTUM);
                int ind = indexOf(names, curr->name);
                if (ind == -1) {
                    // if the task hasen;t been processed yet
                    names[end] = curr->name;
                    tat[end] = time + QUANTUM;
                    wt[end] = time;
                    rt[end] = time;
                    ls[end] = tat[end];
                    end++;
                } else {
                    // if it has been processed before
                    tat[ind] = time + QUANTUM;
                    wt[ind] += (time - ls[ind]);
                    ls[ind] = tat[ind];
                }
                time += QUANTUM;
                curr->burst -= QUANTUM;
                // adding proccessed task to loop list
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
            //printing time
            printf("        Time is now: %d\n", time);
            //picking next task
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
            //idle caused by switching processes
            idles++;
        }
        head = loopHead;
        firstLoop = false;
    } while(head != NULL);
    // printing info table
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
    //printing cpu util
    printf("|\n");
    printf("CPU Utilization: %.2f%%\n", ((time / (float)(time + --idles)) * 100));
}
