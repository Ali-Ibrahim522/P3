/*
  created by Ali Ibrahim

  class to process tasks using the priority algorithm
*/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "schedulers.h"
#include "cpu.h"
// keeps track of the start and of the list
struct node *head = NULL;
struct node *end = NULL;
//the um of tasks to be processed
int size = 0;

//adds a new task to the list
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

//picks the task with the shortest burst, or alpha order
bool comesBefore(Task* temp, Task* best_sofar) {
    if (temp->burst < best_sofar->burst) return true;
    if (temp->burst > best_sofar->burst) return false;
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


// invoke the scheduler
void schedule() {
    // num of times the cpu goes idle
    int idles = 0;
    // arrays for the info table
    char* names[size];
    int tat[size];
    int wt[size];
    int rt[size];

    int i = 0;
    // the current time when processing tasks
    int time = 0;
    // while there is a task to process
    Task* curr = pickNextTask();
    while(curr != NULL) {
      // run task
      run(curr, curr->burst);
      time += curr->burst;
      printf("    Time is now: %d\n", time);
      // update info table
      names[i] = curr->name;
      tat[i] = time;
      wt[i] = tat[i] - curr->burst;
      rt[i] = wt[i];
      i++;
      free(curr);
      //pick next task
      curr = pickNextTask();
      //idle caused by cpu switching tasks
      idles++;
    }
    // printingk the info table
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