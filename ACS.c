// CSC360 Assignment 2 - Stephane Goulet V00929722

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <time.h>
 #include <stdbool.h>
 #include <pthread.h>
 #include <sys/types.h>
 #include <sys/time.h>

// DATA STRUCTURES //

// use to record the customer information read from input text
struct customer_info{
    int user_id;
	int class_type;
	int arrival_time;
	int service_time;
	double start_time;
	double end_time;
};

// use to record the clerk id
struct clerk_info {
	int clerk_id;
};

// use in queue
struct node_t {	
	struct customer_info* customer_info;
	struct node_t* next;
};

// linked list used for queues, includes reference to head and tail
struct queue {
	struct node_t* head;
	struct node_t* tail;
	int len;
};

// LINKED LIST FUNCTIONS //

// creates a new node corresponding to a customer. Takes a node as parameter
struct node_t* make_node(struct customer_info* customer) {
	struct node_t* nn = (struct node_t*)malloc(sizeof(nn));
	nn->customer_info = customer;
	nn->next = NULL;
	return nn;
}

// creates an empty queue
struct queue* make_queue() {
	struct queue* queue = (struct queue*)malloc(sizeof(queue));
	queue->head = NULL;
	queue->tail = NULL;
	queue->len = 0;
	return queue;
}

// adds an element to a queue. Takes a queue and a node as parameters
void enqueue(struct queue* queue, struct node_t* nn) {
	if (queue->len == 0) {
		queue->head = nn;
		queue->tail = nn;
	} else {
		queue->tail->next = nn;
		queue->tail = nn;
	}
	queue->len++;
}

// removes the head from a queue. Takes a queue as input
void dequeue(struct queue* queue) {
	if (queue->len == 0) {
		exit(1);
	} else if (queue->len == 1) {
		queue->head = NULL;
		queue->tail = NULL;
		queue->len = 0;
	} else {
		queue->head = queue->head->next;
		queue->len--;
	}
}

// GLOBAL VARIABLES //
 
struct timeval init_time; // use this variable to record the simulation start time; No need to use mutex_lock when reading this variable since the value would not be changed by thread once the initial time was set.
double overall_waiting_time; // A global variable to add up the overall waiting time for all customers, every customer add their own waiting time to this variable, mutex_lock is necessary.
double overall_waiting_time_business; // Variable to add the overal waiting time for cutomers in business class
double overall_waiting_time_economy; // variable to add the overall waiting time for customers in economy
int clerk_signaled = 0; // holds clerk id of the clerk that was signaled
struct customer_info customerArr[128]; // array of customer_info
struct clerk_info clerkArr[5]; // array of clerk_info
struct queue* businessQueue = NULL; // business class queue (higher priority)
struct queue* economyQueue = NULL; // economy class queue
int customerNum = 0; // count of all customers
int businessNum = 0; // count of all customers in business
int economyNum = 0; // count of all customers in economy

// stores the user_id of a customer that each clerk is helping
int clerk1Helping = 0;
int clerk2Helping = 0;
int clerk3Helping = 0;
int clerk4Helping = 0;
int clerk5Helping = 0;

// Initalize all mutexes and convars
pthread_mutex_t clerk1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk1_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk2_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk3_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk3_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk4_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk4_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk5_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerk5_convar = PTHREAD_COND_INITIALIZER;

pthread_mutex_t businessQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t businessQueue_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t economyQueue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t economyQueue_convar = PTHREAD_COND_INITIALIZER;

pthread_mutex_t access_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t access_convar = PTHREAD_COND_INITIALIZER;

pthread_mutex_t time_mutex = PTHREAD_MUTEX_INITIALIZER;

// HELPER FUNCTIONS //

// used to read a file and store its input into a 2d array. Takes a file and a 2d array as input
void readfile(char* file, char fileinfo[][128]) {
  FILE *f = fopen(file, "r");
  if (f != NULL) {
    int i = 0;
    while (fgets(fileinfo[i] , 128, f) != NULL) {
      i++;
    }
    fclose(f);
    return;
  } else {
    printf("Error: Failed in readfile()");
    return;
  }
}

// used to parse info from 2d array, store the usefull info into customer_info and store the customer_info into the customer array
// takes 2d array and int total number of cutomers
void parseinfo(char fileinfo[128][128], int customerNum) {
	for (int i = 1; i<=customerNum; i++) {
		int j = 0;
		while (fileinfo[i][j] != '\0') {
			if (fileinfo[i][j] == ':' || fileinfo[i][j] == ',') {
          		fileinfo[i][j] = ' ';
        	}
        	j++;
		}
		// catch negative arrival time or service time
		if (atoi(&fileinfo[i][4]) < 0 || atoi(&fileinfo[i][6]) < 0) {
			fprintf(stdout, "Error: Illigal values in input file. \n");
			exit(1);
		}
		struct customer_info c = {
			atoi(&fileinfo[i][0]),
			atoi(&fileinfo[i][2]),
			atoi(&fileinfo[i][4]),
			atoi(&fileinfo[i][6]),
			0.0,
			0.0
		};
		customerArr[i-1] = c;
	}
}

// sets the corresponding clerkHelping var to the user_id of the customer the clerk is helping in business. takes clerk id as param
void update_businessQueue_clerk(int id) {
	if (businessQueue->head != NULL) {
		if (id == 1) {
			clerk1Helping = businessQueue->head->customer_info->user_id;
		}
		if (id == 2) {
			clerk2Helping = businessQueue->head->customer_info->user_id;
		}
		if (id == 3) {
			clerk3Helping = businessQueue->head->customer_info->user_id;
		}
		if (id == 4) {
			clerk4Helping = businessQueue->head->customer_info->user_id;
		}
		else {
			clerk5Helping = businessQueue->head->customer_info->user_id;
		}
	}
}

// sets the corresponding clerkHelping var to the user_id of the customer the clerk is helping in economy. Takes clerk id as param
void update_economyQueue_clerk(int id) {
	if (economyQueue->head != NULL) {
		if (id == 1) {
			clerk1Helping = economyQueue->head->customer_info->user_id;
		}
		if (id == 2) {
			clerk2Helping = economyQueue->head->customer_info->user_id;
		}
		if (id == 3) {
			clerk3Helping = economyQueue->head->customer_info->user_id;
		}
		if (id == 4) {
			clerk4Helping = economyQueue->head->customer_info->user_id;
		}
		else {
			clerk5Helping = economyQueue->head->customer_info->user_id;
		}
	}
}

// returns the user_id of a customer a clerk is helping. Takes clerk id as param
int get_helping_clerk(int id) {
	if (id == 1) {
		return clerk1Helping;
	}
	if (id == 2) {
		return clerk2Helping;
	}
	if (id == 3) {
		return clerk3Helping;
	}
	if (id == 4) {
		return clerk4Helping;
	}
	else {
		return clerk5Helping;
	}
}

double getCurrentSimulationTime(){
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}


// THREAD FUNCTIONS //

void * customer_entry(void * cus_info){
	struct customer_info* customer = (struct customer_info*) cus_info;
	usleep(customer->arrival_time * 100000);
	
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", customer->user_id);
	
	// Enqueue operation if customer is in business
	if (customer -> class_type == 1) {
		businessNum++;
		// lock business queue
		pthread_mutex_lock(&businessQueue_mutex);
		//customer enters business queue
		enqueue(businessQueue, make_node(customer));

		// signal clerk
		pthread_cond_signal(&access_convar);

		fprintf(stdout, "A customer enters a queue: the queue ID %2d, and length of the queue %2d \n", 1, businessQueue->len);

		customer->start_time = getCurrentSimulationTime();

		// wait for clerk to signal when it is ready
		pthread_cond_wait(&businessQueue_convar, &businessQueue_mutex);

		int clerk_signaled_holder = clerk_signaled;

		while (customer->user_id != get_helping_clerk(clerk_signaled_holder)) {
			pthread_cond_wait(&businessQueue_convar, &businessQueue_mutex);
		}

		// customer leaves the business queue
		dequeue(businessQueue);

		// unlock business queue
		pthread_mutex_unlock(&businessQueue_mutex);
		
		usleep(10);
		
		customer->end_time = getCurrentSimulationTime();

		// Calculate wait time
		pthread_mutex_lock(&time_mutex);
		double start = customer->start_time;
		double end = customer->end_time;
		overall_waiting_time += end - start;
		overall_waiting_time_business += end - start;
		pthread_mutex_unlock(&time_mutex);

		fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", customer->start_time, customer->user_id, clerk_signaled_holder);

		// sleep for service time
		usleep(customer->service_time * 100000);

		double end_time = getCurrentSimulationTime();

		fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, customer->user_id, clerk_signaled_holder);

		// signal clerk that customer is done sleeping
		if (clerk_signaled_holder == 1) {
			pthread_mutex_lock(&clerk1_mutex);
			pthread_cond_signal(&clerk1_convar);
			pthread_mutex_unlock(&clerk1_mutex);
		}
		if (clerk_signaled_holder == 2) {
			pthread_mutex_lock(&clerk2_mutex);
			pthread_cond_signal(&clerk2_convar);
			pthread_mutex_unlock(&clerk2_mutex);
		}
		if (clerk_signaled_holder == 3) {
			pthread_mutex_lock(&clerk3_mutex);
			pthread_cond_signal(&clerk3_convar);
			pthread_mutex_unlock(&clerk3_mutex);
		}
		if (clerk_signaled_holder == 4) {
			pthread_mutex_lock(&clerk4_mutex);
			pthread_cond_signal(&clerk4_convar);
			pthread_mutex_unlock(&clerk4_mutex);
		}
		else {
			pthread_mutex_lock(&clerk5_mutex);
			pthread_cond_signal(&clerk5_convar);
			pthread_mutex_unlock(&clerk5_mutex);
		}
	}
	// Enqueue operation if customer is in economy
	else {
		economyNum++;
		// lock economy queue
		pthread_mutex_lock(&economyQueue_mutex);
		
		//customer enters economy queue
		enqueue(economyQueue, make_node(customer));

		// signal clerk
		pthread_cond_signal(&access_convar);

		fprintf(stdout, "A customer enters a queue: the queue ID %2d, and length of the queue %2d \n", 0, economyQueue->len);

		customer->start_time = getCurrentSimulationTime();

		// wait for clerk to signal when it is ready
		pthread_cond_wait(&economyQueue_convar, &economyQueue_mutex);

		int clerk_signaled_holder = clerk_signaled;

		while (customer->user_id != get_helping_clerk(clerk_signaled_holder)) {
			pthread_cond_wait(&economyQueue_convar, &economyQueue_mutex);
		}

		// customer leaves the economy queue
		dequeue(economyQueue);

		// unlock economy queue
		pthread_mutex_unlock(&economyQueue_mutex);
		
		usleep(10);
		
		customer->end_time = getCurrentSimulationTime();

		// Calculate wait time
		pthread_mutex_lock(&time_mutex);
		double start = customer->start_time;
		double end = customer->end_time;
		overall_waiting_time += end - start;
		overall_waiting_time_economy += end - start;
		pthread_mutex_unlock(&time_mutex);

		fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", customer->start_time, customer->user_id, clerk_signaled_holder);

		// sleep for service time
		usleep(customer->service_time * 100000);

		double end_time = getCurrentSimulationTime();

		fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, customer->user_id, clerk_signaled_holder);

		// signal clerk that customer is done sleeping
		if (clerk_signaled_holder == 1) {
			pthread_mutex_lock(&clerk1_mutex);
			pthread_cond_signal(&clerk1_convar);
			pthread_mutex_unlock(&clerk1_mutex);
		}
		if (clerk_signaled_holder == 2) {
			pthread_mutex_lock(&clerk2_mutex);
			pthread_cond_signal(&clerk2_convar);
			pthread_mutex_unlock(&clerk2_mutex);
		}
		if (clerk_signaled_holder == 3) {
			pthread_mutex_lock(&clerk3_mutex);
			pthread_cond_signal(&clerk3_convar);
			pthread_mutex_unlock(&clerk3_mutex);
		}
		if (clerk_signaled_holder == 4) {
			pthread_mutex_lock(&clerk4_mutex);
			pthread_cond_signal(&clerk4_convar);
			pthread_mutex_unlock(&clerk4_mutex);
		}
		else {
			pthread_mutex_lock(&clerk5_mutex);
			pthread_cond_signal(&clerk5_convar);
			pthread_mutex_unlock(&clerk5_mutex);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

// function entry for clerk threads
void *clerk_entry(void * clerkNum){
	struct clerk_info* clerk = (struct clerk_info*) clerkNum;
	int id = clerk->clerk_id;
	
	while(true) {
		pthread_mutex_lock(&access_mutex);
		while (businessQueue->len == 0 && economyQueue->len == 0) { // while queues are empty wait for customer to signal clerk
			pthread_cond_wait(&access_convar, &access_mutex);
		}
		pthread_mutex_unlock(&access_mutex);

		// if business queue is selected
		if (businessQueue->len != 0) {
			// lock business queue
			pthread_mutex_lock(&businessQueue_mutex);
			clerk_signaled = id;
			update_businessQueue_clerk(id);

			// clerk is ready, signal businessQueue 
			pthread_cond_signal(&businessQueue_convar);

			// unlock queue
			pthread_mutex_unlock(&businessQueue_mutex);

			// wait for customer to be done sleeping
			if (id == 1) {
				pthread_mutex_lock(&clerk1_mutex);
				pthread_cond_wait(&clerk1_convar, &clerk1_mutex);
				pthread_mutex_unlock(&clerk1_mutex);
			}
			if (id == 2) {
				pthread_mutex_lock(&clerk2_mutex);
				pthread_cond_wait(&clerk2_convar, &clerk2_mutex);
				pthread_mutex_unlock(&clerk3_mutex);
			}
			if (id == 3) {
				pthread_mutex_lock(&clerk3_mutex);
				pthread_cond_wait(&clerk3_convar, &clerk3_mutex);
				pthread_mutex_unlock(&clerk3_mutex);
			}
			if (id == 4) {
				pthread_mutex_lock(&clerk4_mutex);
				pthread_cond_wait(&clerk4_convar, &clerk4_mutex);
				pthread_mutex_unlock(&clerk4_mutex);
			}
			else {
				pthread_mutex_lock(&clerk5_mutex);
				pthread_cond_wait(&clerk5_convar, &clerk5_mutex);
				pthread_mutex_unlock(&clerk5_mutex);
			}
		}
		// if economy queue is selected
		else if (economyQueue->len != 0) {
			pthread_mutex_lock(&economyQueue_mutex);
			clerk_signaled = id;
			update_economyQueue_clerk(id);

			// signal economyQueue
			pthread_cond_signal(&economyQueue_convar);

			// unlock queue
			pthread_mutex_unlock(&economyQueue_mutex);


			// wait for customer to be done sleeping
			if (id == 1) {
				pthread_mutex_lock(&clerk1_mutex);
				pthread_cond_wait(&clerk1_convar, &clerk1_mutex);
				pthread_mutex_unlock(&clerk1_mutex);
			}
			if (id == 2) {
				pthread_mutex_lock(&clerk2_mutex);
				pthread_cond_wait(&clerk2_convar, &clerk2_mutex);
				pthread_mutex_unlock(&clerk3_mutex);
			}
			if (id == 3) {
				pthread_mutex_lock(&clerk3_mutex);
				pthread_cond_wait(&clerk3_convar, &clerk3_mutex);
				pthread_mutex_unlock(&clerk3_mutex);
			}
			if (id == 4) {
				pthread_mutex_lock(&clerk4_mutex);
				pthread_cond_wait(&clerk4_convar, &clerk4_mutex);
				pthread_mutex_unlock(&clerk4_mutex);
			}
			else {
				pthread_mutex_lock(&clerk5_mutex);
				pthread_cond_wait(&clerk5_convar, &clerk5_mutex);
				pthread_mutex_unlock(&clerk5_mutex);
			}
		}	
	}
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char* argv[]) {

	// initialize all the condition variables
	pthread_t clerkId[5];
	pthread_t customerId[128];
	
	char fileinfo[128][128];
	readfile(argv[1], fileinfo);
	customerNum= atoi(fileinfo[0]);
	parseinfo(fileinfo, customerNum);
	businessQueue = make_queue();
	economyQueue = make_queue();

	for (int i = 1; i<=5; i++) {
		struct clerk_info temp = {i};
		clerkArr[i-1] = temp;
	}

	//create clerk thread
	for(int i = 0; i < 5; i++){ // number of clerks
		if (pthread_create(&clerkId[i], NULL, clerk_entry, &clerkArr[i])) {	// passing the clerk information (e.g., clerk ID) to clerk thread
			perror("Error: failed to create clerk thread");
			exit(0);
		}
	}

	gettimeofday(&init_time, NULL);
	
	//create customer thread
	for(int i = 0; i < customerNum; i++){ // number of customers
		if (pthread_create(&customerId[i], NULL, customer_entry, &customerArr[i])) { //customer_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
			perror("Error: failed to create customer thread");
			exit(0);
		}
	}

	// wait for all customer threads to terminate
	for (int i = 0; i<customerNum; i++) {
		if (pthread_join(customerId[i], NULL)) {
			perror("Error: failed to join customer thread");
			exit(0);
		}
	}

	// calculate and print waiting times
	fprintf(stdout, "The average waiting time for all customers in the system is: %2f seconds. \n", overall_waiting_time/customerNum);
	fprintf(stdout, "The average waiting time for all business-class customers is: %2f seconds. \n", overall_waiting_time_business/businessNum);
	fprintf(stdout, "The average waiting time for all economy-class customers is: %2f seconds. \n", overall_waiting_time_economy/economyNum);

	return 0;
}
