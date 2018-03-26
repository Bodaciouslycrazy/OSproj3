//Bodie Malik
//3-20-2018

/*

COMPILE INSTRUCTIONS.
There are some flags you need to include while compiling.
Here is the command I used for compiling...

gcc -pthread -lrt -o csmc csmc.c

This should work an any bash/linux environment.
Please email me at bam150130@utdallas.edu if you have any problems.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

//**********************Arguments*******************
//Do not change these after they have been read!
//These here are just standard values.
int NUM_TUTORS = 5;
int NUM_STUDENTS = 10;
int NUM_MAXCHAIRS = 5;
int NUM_HELP = 3;
//**************************************************


//*********************MUTEXES***********************
//The variables under each mutex requires a lock from
//that mutex. Otherwise, DO NOT USE IT.

pthread_mutex_t MutexChairs;
int CurEmptyChairs = 5;

//***************************************************

//********************SEMAPHORES*********************
sem_t SemTutors;
sem_t SemStudents;
//***************************************************


void *ManagerStart(void *param)
{
	//waits for a tutor
	//waits for a student
	
	//pairs them
	
	//repeat
	
	while(1)
	{
		sem_wait(&SemTutors);
		sem_wait(&SemStudents);
		
		//Found a tutor and a student. Pair them.
		printf("Coordinator found a student/tutor pair.\n");
		
	}
	
	
	
	sleep(15);
	printf("Manager finished.\n");
	return NULL;
}

/***********************************************************
						TUTOR
************************************************************/
void *TutorStart(void *param)
{
	int num = *((int *) param);
	printf("Tutor thread %d created.\n", num);
	
	while( 1 )
	{
		printf("Tutor %d is wiating to be assigned to a student.\n",num);
		
		//Signal that a tutor is waiting for a student.
		sem_post(&SemTutors);
		//CONTINUE HERE===============================================================================
	}
	
	printf("Tutor thread %d has finished.\n", num);
	return NULL;
}


/***********************************************************
						STUDENT
************************************************************/
void *StudentStart(void *param)
{
	int num = *((int *) param);
	printf("Student thread %d created.\n", num);
	
	int HelpNeeded = NUM_HELP;
	
	//If you need help, program some, then try to find a waiting chair.
	while(HelpNeeded > 0)
	{
		//Program for some time, then seek help.
		sleep( rand() % 3 + 1); //will sleep anywhere from 1 to 3 seconds
		
		//I NEED HELP NOW, SEARCH FOR WAITING CHAIR
		
		//*********Using MutexChairs**************
		pthread_mutex_lock(&MutexChairs);
		if(CurEmptyChairs <= 0)
		{
			//all the chairs are taken. End mutex and program more..
			printf("Student %d found no empty chair. Will come back later...\n", num);
			pthread_mutex_unlock(&MutexChairs);
			continue;
		}
		else
		{
			CurEmptyChairs -= 1;
			printf("Student %d takes a seat. Waiting students = %d.\n", num, NUM_MAXCHAIRS - CurEmptyChairs);
		}
		pthread_mutex_unlock(&MutexChairs);
		//*********Finished using MutexChairs*****
		
		//We now have a waiting chair. Signal that a student is waiting.
		sem_post(&SemStudents);
		//INCOMPLETE======================================================================continue coding here
	}
	
	printf("Student thread %d has finished.\n", num);
	return NULL;
}

int main(int argc, char *argv[])
{
	//Read arguments
	if(argc >= 5)
	{
		NUM_STUDENTS 		= atoi( argv[1] );
		NUM_TUTORS 			= atoi( argv[2] );
		NUM_MAXCHAIRS 		= atoi( argv[3] );
		NUM_HELP 			= atoi( argv[4] );
	}
	
	//initialize randomizer
	srand(time(NULL));
	
	//initialize mutex locks.
	if( pthread_mutex_init(&MutexChairs, NULL) != 0)
	{
		printf("Error creating mutex.\n");
		return 1;
	}
	
	//initialize semaphores.
	if( sem_init( &SemTutors, 0, 0) != 0)
	{
		printf("Error creating tutor semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemStudents, 0, 0) != 0)
	{
		printf("Error creating students semaphore.\n");
		return 1;
	}
	
	
	//******************CREATE THE THREADS*******************
	pthread_t TutorThreads[NUM_TUTORS];
	pthread_t StudentThreads[NUM_STUDENTS];
	pthread_t ManagerThread;
	
	for(int i = 0; i < NUM_TUTORS; i++)
	{
		int *TutorNumber = malloc(sizeof(*TutorNumber));
		*TutorNumber = i;
		
		if( pthread_create(&TutorThreads[i], NULL, TutorStart, (void *)TutorNumber ) )
		{
			printf("Error creating tutor thread %d.\n", i);
			return 1;
		}
		
		printf("Created tutor %d\n", i);
	}
	
	for(int i = 0; i < NUM_STUDENTS; i++)
	{
		int *StudentNumber = malloc(sizeof(*StudentNumber));
		*StudentNumber = i;
		
		if( pthread_create(&StudentThreads[i], NULL, StudentStart, (void *)StudentNumber ) )
		{
			printf("Error creating student thread %d.\n", i);
			return 1;
		}
		
		printf("Created student %d\n", i);
	}
	
	if( pthread_create(&ManagerThread, NULL, ManagerStart, NULL) )
	{
		printf("Error creating manager thread.");
	}
	
	//MANAGER SHOULD NOW DO STUFF.
	//JOIN EVERYTHING BACK UP AFTER MANAGER RETURNS.
	
	//wait and join up.
	pthread_join(ManagerThread, NULL);
	
	for(int i = 0; i < NUM_TUTORS; i++)
	{
		pthread_join(TutorThreads[i], NULL);
	}
	
	for(int i = 0; i < NUM_STUDENTS; i++)
	{
		pthread_join(StudentThreads[i], NULL);
	}
	
	printf("All threads have joined. Finished.\n");
	return 0;
}
