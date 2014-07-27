/* PPLS Assignment 2: PartA  --- Georgi Tsatsev - s1045049 

compile instructions :
	/usr/lib64/openmpi/bin/mpicc -o aquadPartA aquadPartA.c stack.h stack.c
run instructions :
	/usr/lib64/openmpi/bin/mpirun -c 5 ./aquadPartA

Implementation:

The program is divided into three main parts:

The main function:

It is provided by the assignment, calls the farmer and worker functions and is responsible for the MPI initialization and the printing of the results.

The farmer function:

The farmer’s main body is within a while loop that is executed until either a READY_TAG is received or a structure containing the bag of tasks becomes empty and the number of non-working processes (the “slackers” variable within the code) is not 0. Thus resulting in a loop that is executed until the workers finish computing their area or the bag of tasks is empty. The loop starts synchronous MPI primitive with any tag from any source. It takes the source of the message, its tag and changes the value of the is_worker_busy array ( an array that keeps track of which worker to send tasks to if it is 1 then the worker is to be sent a task) to the corresponding worker that it has received the message from. It then checks the tag if it is a SPLIT_TAG or an AREA_TAG. If it is an area tag it adds it to the total area else if it is a split tag it pushes the new task that was received in the bag of tasks receives another message from that worker with the second task and pushes it again in the bag of tasks, resulting in the two new tasks added to the bag. After these checks the farmer calls a helper function (send_tasks(bag_of_tasks, is_worker_busy,slackers,tasks_per_process)) that deals with the distribution of tasks. Within this function there is a “for” loop that iterates through the workers and checks if the value within the array is_worker_busy is 1. If so then the farmer sends a task to that worker, with a synchronous MPI primitive with a WORK_TAG, sets the value to 0, increments its number of tasks and pops that task from the bag. If we don’t have any more tasks to distribute or the number of slacking processes is equal to the total number of workers (this is when none of the workers are supposed to be given tasks) the loop stops and we return in the main farmer body doing the loop all over again. After the main loop finishes a signal to all worker with a READY_TAG is sent to all workers to tell them that the work is done and the accumulated area is returned.  

The worker function:

The worker initially starts with a synchronous MPI send primitive providing the initial handshake between the farmer and that respective worker. After the initial handshake is done there is a do while loop that is executed until a READY_TAG is received by the farmer. The inner body of the loop starts with an MPI_Recv primitive from the farmer that determines the task and the tag that the worker would be executing. The worker then calls a helper function (compute (left,right,&split)) that simulates the quadrature algorithm given two points (which are the task that was received by the farmer). There is a variable called split that within the compute function if a split should be initialized is set to 1 and if not to 0. The compute function returns either the computed area or the mid-point of the split. If the split is 1 then we initiate two synchronous MPI_Send s by that telling the farmer the two new tasks (with a SPLIT_TAG) otherwise one MPI_Send is used to send the farmer the computed area (with a AREA_TAG). 

MPI primitive alternatives:

In my implementations only synchronous, blocking MPI_Send and MPI_Recv have been used.  Alternative approaches would be to use an asynchronous receive to look at each worker to get its respective result but that would mean to iterate over all workers and in my approach iteration over all workers is avoided by the imposed synchronizations. The primitives used assume the data will be received eventually. If not then it would result in a deadlock. Alternative considered was to use a blocking MPI_Probe for the initial handshaking between the farmer and the workers.  Another to the MPI_Recv was to use the non-blocking MPI_Irecv but that would have required keeping track of the MPI_Request object created by the immediate receive.


*/



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "stack.h"

#define EPSILON 1e-3
#define F(arg)  cosh(arg)*cosh(arg)*cosh(arg)*cosh(arg)
#define A 0.0
#define B 5.0



#define SLEEPTIME 1

/* Defining constants for the tags and the farmer. */
#define FARMER 0
#define READY_TAG 0
#define WORK_TAG 1
#define SPLIT_TAG 2
#define AREA_TAG 3



int *tasks_per_process;

double farmer(int);
int send_tasks(stack* ,int* ,int ,int* );

void worker(int);
double compute(double, double, int*);

int main(int argc, char **argv ) {
  int i, myid, numprocs;
  double area, a, b;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  if(numprocs < 2) {
    fprintf(stderr, "ERROR: Must have at least 2 processes to run\n");
    MPI_Finalize();
    exit(1);
  }

  if (myid == 0) { // Farmer
    // init counters
    tasks_per_process = (int *) malloc(sizeof(int)*(numprocs));
    for (i=0; i<numprocs; i++) {
      tasks_per_process[i]=0;
    }
  }

  if (myid == 0) { // Farmer
    area = farmer(numprocs);
  } else { //Workers
    worker(myid);
  }

  if(myid == 0) {
    fprintf(stdout, "Area=%lf\n", area);
    fprintf(stdout, "\nTasks Per Process\n");
    for (i=0; i<numprocs; i++) {
      fprintf(stdout, "%d\t", i);
    }
    fprintf(stdout, "\n");
    for (i=0; i<numprocs; i++) {
      fprintf(stdout, "%d\t", tasks_per_process[i]);
    }
    fprintf(stdout, "\n");
    free(tasks_per_process);
  }
  MPI_Finalize();
  return 0;
}


double farmer(int numprocs) {
	
	MPI_Status status;
	stack* bag_of_tasks = new_stack();
	
	int i,id;
	int tag = -1;
	double area = 0.0;
	
	/* slackers is a variable to keep track of the free ("slacking") processes.*/
	int slackers = numprocs-1; 
	int* is_worker_busy = (int*) malloc(sizeof(int)*(numprocs-1)); 
	
	/* An integer array for workers. If 1 then the worker is busy if 0 then it is not doing any tasks.
	We fill the array with zeroes initially since none of the workers are working. */
	for (i=0;i<slackers;i++){
		is_worker_busy[i] = 0;
	}
		
	double point[2];
	point[0] = A;
	point[1] = B;
	/* We push the initial task on the bag of tasks stack*/	
	push(point, bag_of_tasks);
	
	
	while ((!is_empty(bag_of_tasks)||slackers!=0)&&(tag!=READY_TAG)){
		
		MPI_Recv(point, 2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
		
		id = status.MPI_SOURCE;
		tag = status.MPI_TAG;
		is_worker_busy[id-1] = 1;
		slackers--;

		if ( tag == AREA_TAG) { 
			area += point[0];
		}
		else if (tag == SPLIT_TAG) { 
			push(point,bag_of_tasks);
      			MPI_Recv(point, 2, MPI_DOUBLE, id, tag, MPI_COMM_WORLD, &status);
      			push(point,bag_of_tasks);
		} 
		
		
		slackers=send_tasks(bag_of_tasks,is_worker_busy,slackers,tasks_per_process);
		
	}	


	// Signal all the workers that we are ready.
	for (i=1;i<numprocs;i++) { 

		MPI_Send(NULL, 0, MPI_INT, i, READY_TAG, MPI_COMM_WORLD);
	}
	return area;
}
int send_tasks(stack* bag_of_tasks,int* is_worker_busy,int slackers,int* tasks_per_process){
	
	int i;
	for (i=0;i<=sizeof(*is_worker_busy);i++){
		
		/* If all of our processes are slacking or our bag_of_tasks is empty
		 we have nothing to send => break the loop. */
		if (slackers==sizeof(*is_worker_busy)||is_empty(bag_of_tasks)){
			break;
		}
		/* If our worker is chosen for the work send the task 
		to it and pop it out of the bag of tasks.*/
		if (is_worker_busy[i]){
			MPI_Send(pop(bag_of_tasks), 2, MPI_DOUBLE, i+1, WORK_TAG, MPI_COMM_WORLD);
				is_worker_busy[i] = 0;
				slackers++;
				tasks_per_process[i+1]++;	
		}	
	}

	return slackers;
}
void worker(int mypid) {
	int tag,split;
	MPI_Status status;
	double point[2];
	double mid_area;
	MPI_Send(point, 2, MPI_DOUBLE, FARMER, WORK_TAG, MPI_COMM_WORLD);
	

	do {
		MPI_Recv(point, 2, MPI_DOUBLE, FARMER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		tag = status.MPI_TAG;
		
	    		double left = point[0];
	    		double right = point[1];
			
			/* Call the compute function and sleep afterwards for the predefined time.*/
			mid_area = compute(left,right,&split);
			usleep(SLEEPTIME);			
			if (split){
				point[0] = left;
				point[1] = mid_area;
				MPI_Send(point, 2, MPI_DOUBLE, FARMER, SPLIT_TAG, MPI_COMM_WORLD);
				point[0] = mid_area;
				point[1] = right;
				MPI_Send(point, 2, MPI_DOUBLE, FARMER, SPLIT_TAG, MPI_COMM_WORLD);
			} else{
				point[0] = mid_area;
				point[1] = 0;
				MPI_Send(point, 2, MPI_DOUBLE, FARMER, AREA_TAG, MPI_COMM_WORLD);
			}


  	} while (tag != READY_TAG);
}

/* A helper function that given the left and right points and a pointer to an integer "split"
determines if the area needs to be split (and sets the split integer to 1 if true 0 otherwise)
or it needs to be computed. It returns it as a double variable called mid_area. It either holds
the mid point or the computed area.*/
double compute(double left, double right,int* split){
	
	double mid_area=0.0;
	double lrarea = (F(left) + F(right)) * (right - left) / 2;
	double mid, fmid, larea, rarea;
	mid = (left + right) / 2;
	fmid = F(mid);
	larea = (F(left) + fmid) * (mid - left) / 2;
	rarea = (fmid + F(right)) * (right - mid) / 2;

	if (fabs((larea + rarea) - lrarea) > EPSILON) { 
		*split = 1;
		mid_area=mid;				
	} else {
		*split = 0;
		mid_area=larea + rarea;			
	}

	return mid_area;

}
