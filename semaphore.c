#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

// define semaphore indeces and buffer size
#define EMPTY 0
#define FULL 1
#define MUTEX 2
#define N 10

// create semun struct
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

// create counter and buffer for demo of producer/consumer problem
int item = -1;
int buffer[N];

int readIndex = -1;
int writeIndex = -1;

// array for thread ID's
pthread_t threads[2];

// declare global semaphore ID
static int sem_id;

// semaphore down function
static int down(int semIndex)
{
	// declare sembuf
	struct sembuf sem_b;
	
	// set the sembuf values as needed for the current down
	sem_b.sem_num = semIndex;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	
	// update the semaphore with new values
	if (semop(sem_id, &sem_b, 1) == -1)
	{
		perror("semaphore down fail");
		exit(1);
	}
	return 1;
}

// semaphore up function
static int up(int semIndex)
{
	// declare sembuf
	struct sembuf sem_b;
	
	// set the sembuf values as needed for the current up
	sem_b.sem_num = semIndex;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	
	// update the semaphore with new values
	if (semop(sem_id, &sem_b, 1) == -1)
	{
		perror("semaphore up fail");
		exit(1);
	}
	return 1;
}

// produce function
// simply increments the global item variable, prints the value, and returns it
int produce_item()
{
	item++;
	printf("++PRODUCED:   %d\n", item);
	return item;
}

// insert function
// inserts the current item value into an avaiable slot in the buffer
void insert_item(int pItem)
{
	writeIndex = (writeIndex + 1) % N;
	buffer[writeIndex] = pItem;
}

// remove function
// "removes" a value from the buffer and returns the value that was consumed
// also handles the situation (that should not happen) where there is a 
// negative index
int remove_item()
{
	readIndex = (readIndex + 1) % N;
	return buffer[readIndex];
}

// consume function
// siimply prints the value that was consumed
void consume_item(int cItem)
{
	printf("--CONSUMED:   %d\n", cItem);
}

// producer function
// adapted directly from Dr. Park's lab assignment and PowerPoints
void *producer(void *tid)
{
	int pItem;
	
	while(1)
	{
		pItem = produce_item();
		down(EMPTY);
		down(MUTEX);
		insert_item(pItem);
		up(MUTEX);
		up(FULL);
		//sleep(rand() % 3);	// sleep used for testing purposes
	}
}

// consumer function
// adapted directly from Dr. Park's lab assignment and PowerPoints
void *consumer(void *tid)
{
	int cItem;
	
	while(1)
	{
		down(FULL);
		down(MUTEX);
		cItem = remove_item();
		up(MUTEX);
		up(EMPTY);
		consume_item(cItem);
		//sleep(rand() % 3);		// sleep used for testing purposes
	}
}

// delete semaphore function
static void del_semvalue(int semVal)
{
	union semun sem_union;
	
	semctl(sem_id, semVal, IPC_RMID, sem_union);
}

// signal handler stops threads and deletes semaphores when user presses
// Ctrl + C
void stop(int sig)
{
	pthread_cancel(threads[0]);
	pthread_cancel(threads[1]);
	
	for (int i = 0; i < 3; i++)
	{
		del_semvalue(i);
	}
}

// main function
// creates the threads and semaphores necessary for this assignment
int main()
{
	// create array for thread Id's and empty key variable
	key_t key;
	
	// declare semun struct
	union semun sem_union;
	
	// create key unique to this program
	if ((key = ftok("semaphore.c", 'T')) == -1)
	{
		perror("ftok error");
		exit(1);
	}
	
	// create 3 semaphores
	sem_id = semget(key, 3, IPC_CREAT|0666);
	
	// set value for the mutex semaphore
	sem_union.val = 1;
	semctl(sem_id, MUTEX, SETVAL, sem_union);
	
	// set value for the empty semaphore
	sem_union.val = N;
	semctl(sem_id, EMPTY, SETVAL, sem_union);
	
	// set value for the full semaphore
	sem_union.val = 0;
	semctl(sem_id, FULL, SETVAL, sem_union);
	
	signal(SIGINT, stop);
	
	// create the producer and consumer threads
	pthread_create(&threads[0], NULL, producer, (void *)threads[0]);
	pthread_create(&threads[1], NULL, consumer, (void *)threads[1]);
	
	// these joins and exits will never be reached since this program
	// runs infinitely
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	pthread_exit(NULL);
	
	exit(0);
}
