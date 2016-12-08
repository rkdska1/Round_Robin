#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "queue.h"

#define CHILDNUMBER 5
#define QUANTUM 5
#define READY 1
#define WAIT 0
#define MY_KEY 1027

struct child_p {
	long pid;
	int cpu_burst;
	int io_burst;
};

struct msg_node{
	long m_type;
	pid_t pid;
	int io_msg;
};

struct process_parent{
	long pid;
	int work_time;
	int wait_time;
	int remaining_io_time;
};

FILE* fp;
pid_t pid;
int global_time;
struct child_p *CP;
struct sigaction sa;
struct sigaction su;
struct queue *run_queue;
struct queue *wait_queue;
struct queue *rt_queue;
struct process_parent process_list[CHILDNUMBER];

void set_pcb(struct node* Node);
void scheduling();
void proc_generate(int i);
void cpu_b_handler();
void do_child();
void do_parent();
void alarm_handler(int sig);
void enable_time();
int Search_process(long pid);
void time_sign(int sig);

void scheduling(){
	struct node *temp;
	
	run_queue->front->pcb->remain_quantum= QUANTUM;
	temp=run_queue->front->next;
	push(rt_queue, pop(run_queue->front->pcb->pid, run_queue));
	if(run_queue->count==0){
	               struct queue *tmp_queue;
	               tmp_queue = run_queue;
	               run_queue = rt_queue;
	               rt_queue = tmp_queue;
	               tmp_queue=NULL;
	}
	else
	{
		run_queue->front = temp;
	}
	
}

void set_pcb(struct node* Node){
	struct process *new_process;
	new_process= (struct process *)malloc(sizeof(struct process));
	Node->pcb = new_process;
	Node->pcb->pid=pid;
	Node->pcb->status = READY;
	Node->pcb->remain_quantum = QUANTUM;
}

void proc_generate(int i){
	process_list[i].pid =0;
	process_list[i].work_time=0;
	process_list[i].wait_time=0;
	process_list[i].remaining_io_time=0;
}


int main(){
	int i;
	int process_return;	
	int time_tick=0;
	global_time=0;
	fp = fopen("Round_robin_K_J_C.txt","w");	
	
	run_queue= create_queue();
	wait_queue = create_queue();
	rt_queue = create_queue();

	
	for(i=0;i<CHILDNUMBER; i++){
                pid=fork();
                if(pid==-1){
                        exit(1);
                }
                else if(pid==0){        //child proces
			do_child();
	                return 0;
                }
                else{           //parent process
			struct node *tmp = (struct node*)malloc(sizeof(struct node));

			proc_generate(i);
			process_list[i].pid=pid;
			set_pcb(tmp);
			push(run_queue,tmp);
                }
	}
	enable_time();
	do_parent();

	kill(pid, SIGKILL);

	destroy_queue(wait_queue);
	destroy_queue(run_queue);	
	return 0;
}

void do_child(){
	struct sigaction old1;
	struct sigaction old2;
	sigemptyset(&old1.sa_mask);
	sigemptyset(&old2.sa_mask);
	CP = (struct child_p *)malloc(sizeof(struct child_p));	
	srand(time(NULL));
	CP->pid = getpid();

	CP->cpu_burst = (rand()%20+getpid()%9+6);
	CP->io_burst = (rand()%20+getpid()%5 +4);
	

	memset(&su, 0, sizeof(su));
	su.sa_handler = &alarm_handler;
	sigemptyset(&su.sa_mask);
	sigaction(SIGUSR1, &su, 0);
	
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &cpu_b_handler;

	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR2, &sa, 0);
	while(1);
	sigaction(SIGUSR1, &old1, NULL);
	sigaction(SIGUSR2, &old2, NULL);

}

void alarm_handler(int sig){
	int msqid;
	int i =0;
	struct node *temp_node;
	struct msg_node msg;
	key_t key;
	key = ftok(".",'B');
	
	if(-1==(msqid = msgget(key,IPC_CREAT|0644))){
		perror("msgget() failed");
		exit(1);
	}
	else{
		if(CP->cpu_burst!=0){
//find_node(CP->pid, run_queue)!=NULL){
			CP->cpu_burst--;
			printf("CPU BURST REMAINING : %d\n", CP->cpu_burst);
			fprintf(fp, "CPU BURST REMAINING : %d\n", CP->cpu_burst);
			if(CP->cpu_burst==0){
				msg.m_type= 4;
				msg.pid = getpid();
				msg.io_msg = CP->io_burst;
				msgsnd(msqid, &msg, sizeof(struct msg_node), 0);
			}
		}
	}
}

void cpu_b_handler(int sig){
	srand(time(NULL));
	CP->cpu_burst= (rand()%20*getpid()%9+1);
	CP->io_burst=6;	(rand()%20*getpid()%7+1);
}

void enable_time(){
	struct sigaction sa_tt;
	struct itimerval time;
	
	sigemptyset(&sa_tt.sa_mask);
	memset(&sa_tt, 0, sizeof(sa_tt));
	sa_tt.sa_handler = &time_sign;
	sigaction(SIGALRM, &sa_tt, NULL);
	
	time.it_value.tv_sec= 0;
	time.it_value.tv_usec=50000;
	time.it_interval.tv_sec=0;
	time.it_interval.tv_usec=50000;
	
	setitimer(ITIMER_REAL, &time, NULL);

}

void time_sign(int sig){
	int i; int j; int n; int temp_number=0;
	struct node *temp_node; struct node *temp_node2; struct node *temp_node3; struct node *temp_node4;
	struct node *temp;
	struct node *temp2;
	global_time++;





	printf("-------------------------------------------------\nTime : %d\n", global_time);
	fprintf(fp, "---------------------------------------------------\nTime : %d\n", global_time);
	if(run_queue->count>0){
		temp_node=run_queue->front;
		printf("***RUN QUEUE***\nPID : ");
		fprintf(fp, "***RUN QUEUE***\nPID : ");
		while(run_queue->count!=temp_number){
			printf(" (%ld)", temp_node->pcb->pid);
			fprintf(fp, " (%ld)", temp_node->pcb->pid);
			temp_node= temp_node->next;
			temp_number++;		
		}
		printf("\n***END OF RUN QUEUE***\n");
		fprintf(fp, "\n***END OF RUN QUEUE***\n");
	}
	temp_number=0;
	if(wait_queue->count>0){
		temp_node2=wait_queue->front;
		printf("***WAIT QUEUE***\n PID : ");
		fprintf(fp, "***WAIT QUEUE***\n PID :");
		while(wait_queue->count!=temp_number){
			printf(" (%ld)", temp_node2->pcb->pid);
			fprintf(fp, " (%ld)", temp_node2->pcb->pid);
			temp_node2=temp_node2->next;
			temp_number++;
		}
		printf("\n***END OF WAIT QUEUE***\n");
		fprintf(fp, "\n***END OF WAIT QUEUE***\n");
	}
	temp_number=0;
	if(rt_queue->count>0){
		temp_node4=rt_queue->front;
		printf("***RETIRE QUEUE***\n PID : ");
		fprintf(fp, "***RETIRE QUEUE***\n PID : ");
		while(rt_queue->count!=temp_number){
			printf(" (%ld)", temp_node4->pcb->pid);
			fprintf(fp, " (%ld)", temp_node4->pcb->pid);
			temp_node4=temp_node4->next;
			temp_number++;
		}
		printf("\n***END OF RETIRE QUEUE***\n");
		fprintf(fp, "\n***END OF RETIRE QUEUE***\n");
	}
	temp_number=0;







	if(run_queue->count){
		(run_queue->front->pcb->remain_quantum)--;
		process_list[Search_process(run_queue->front->pcb->pid)].work_time++;
		printf("Process :  %ld running currently\n", run_queue->front->pcb->pid);
		fprintf(fp, "Process :  %ld running currently\n", run_queue->front->pcb->pid);
		
		if(global_time>=10000){
			fprintf(fp, "EXITING PROGRAM!!!!!\n");
			for(i=0;i<CHILDNUMBER;i++){
				kill(process_list[i].pid, SIGKILL);
			}
			kill(getpid(), SIGKILL);
	
		}
		if(run_queue->front->pcb->remain_quantum==0){
			scheduling();
			kill(run_queue->front->pcb->pid, SIGUSR1);
		}
		else{
			kill(run_queue->front->pcb->pid, SIGUSR1);
		}
	}
	if(wait_queue->count>0){
		temp= wait_queue->front;
		for(n=0; n<wait_queue->count;n++){
		//	process_list[Search_process(temp->pcb->pid)].remaining_io_time--;
			temp->pcb->remain_io--;
			process_list[Search_process(temp->pcb->pid)].wait_time++;
			temp2=temp;
			temp=temp->next;
			if(temp2->pcb->remain_io<=0){
				temp2 = pop(temp2->pcb->pid , wait_queue);
				temp2->pcb->status= READY;
				temp2->pcb->remain_quantum=QUANTUM;
				push(run_queue, temp2);
				kill(temp2->pcb->pid, SIGUSR2);
			}
		}
	}
}

void do_parent(){
	int msqid;
	long pid_n;
	int i = 0; int j;
	int receiver;
	struct msg_node msg;
	struct process *tmp_process;
	struct node *tmp_node;
	key_t key;
	key = ftok(".",'B');
//	tmp_process = (struct process*)malloc(sizeof(struct process));
	if(-1==(msqid = msgget(key,IPC_CREAT|0644)))
	{
		perror("msgget() failed");
		exit(1);
	}
	
	while(1){
		receiver = msgrcv(msqid,&msg,sizeof(struct msg_node),4,0);
		if(receiver==-1){
		;

		}
		else{
//			printf("parent receive pid:[%ld],CPU_burst:%d,IO_burst:%d\n",msg.pid,msg.cpu_burst,msg.io_burst);
			if(-1== (j =Search_process(msg.pid)))
			{
				break;
			}
			pid_n = msg.pid;
			if(run_queue->count){
			tmp_node = pop(pid_n, run_queue);
			tmp_node->pcb->remain_io= msg.io_msg;
			tmp_node->pcb->status = WAIT;
			push(wait_queue, tmp_node);
			}
		}
	}
}
				

int Search_process(long pid){
	int i=0;
	while(i<CHILDNUMBER){
	if(pid==process_list[i].pid)
	{
		return i;
	}
	i++;
	}
		return -1;
}
