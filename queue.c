/*
 * queue.c
 *
 *  Created on: 2011. 4. 23.
 *      Author: Chwang
 */

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

struct queue* create_queue(){
	struct queue* new_queue=(struct queue*)malloc(sizeof(struct queue));
	new_queue->count=0;
	new_queue->front=NULL;
	new_queue->rear=NULL;
	return new_queue;
}

void destroy_queue(struct queue* qptr){
	struct node *temp;
	struct process *temp_p;
	while(qptr->count!=0){
		temp = qptr->front;
		temp_p=temp->pcb;
		qptr->front=qptr->front->next;
		free(temp_p);
		free(temp);
	}
	free(qptr);
	printf("Queue Destroy Complete\n");
}
/*
void push(queue* qptr, int value){
	nptr new_node=(nptr)malloc(sizeof(node));
	new_node->value=value;
	new_node->next=NULL;

	if (qptr->count==0){
		qptr->front=new_node;
		qptr->rear=new_node;
	}
	else{
		qptr->rear->next=new_node;
		qptr->rear=new_node;
	}
	qptr->count++;
}

int pop(queue* qptr){
	if (qptr->count != 0){
		nptr tmp;
		struct process tmp_proc;
		tmp = (node*)malloc(sizeof(node));
		tmp = qptr->front;
		qptr->front=tmp->next;
		tmp_proc=tmp->proc;
		free(tmp_proc);
		qptr->count--;
		return tmp_value;
	}
	else{
w
		printf("No item left in queue\n");
		return -1;
	}
}
//same pop

*/



void push(struct queue* Queue,struct node *proc){
	//Create a new node
	proc->pcb;
	
	if(Queue->front == NULL){
		Queue->front = proc;
	}
	else{
		Queue->rear->next = proc;
	}
	proc->next= Queue->front;
	Queue->rear = proc;
	Queue->count++;	
}

struct node* pop (long pid, struct queue *Queue){
	//get the first item
	if(Queue->count==0){
		return NULL;
	}
	struct node *temp = find_node(pid, Queue);
	struct node *tmp_next=temp->next;
	if(Queue->front==Queue->rear){
		Queue->front = NULL;
		Queue->rear = NULL;
	}
	else if(temp==Queue->front) 	{
		Queue->rear->next = Queue->front->next;
		Queue->front= Queue->front->next;
	}
	else if(temp==Queue->rear){
		while(tmp_next->next != temp){
			tmp_next=tmp_next->next;
		}
		Queue->rear = tmp_next;
		Queue->rear->next = Queue-> front;
	}
	else{
		while(tmp_next->next != temp){
			tmp_next = tmp_next->next;
		}
		
		tmp_next->next = temp->next;
	}
	Queue->count--;	
	return temp;	
}

int isEmpty(struct queue *q){
	if(q->count!=0){
		return 1;
	}
	else
		return 0;

}

struct node *find_node(long pid, struct queue *Queue){
	struct node *tmp;
	tmp= Queue->front;
	do{
		if(tmp->pcb->pid==pid){
			return tmp;
		}
		else{
			tmp=tmp->next;
		}


	}while(tmp->next != Queue->front );
	return NULL;
}

