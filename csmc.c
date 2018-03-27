//Bodie Malik
//3-20-2018

/*
COMPILE INSTRUCTIONS.
There are some flags you need to include while compiling.
Here is the command I used for compiling...

gcc -pthread -lrt -o csmc csmc.c

This should work an any bash/linux environment.
If you have any problems, email me at bam150130@utdallas.edu
*/

/*
ToDo:
Noting.
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
int NUM_TUTORS = 3;
int NUM_STUDENTS = 10;
int NUM_MAXCHAIRS = 5;
int NUM_HELP = 2;
//***************************************************


//*********************MUTEXES***********************
pthread_mutex_t MutexChairs;	//This is used so that two students don't sit down at the same time.
int EmptyChairs = 0;
//***************************************************


//********************SEMAPHORES*********************
sem_t SemTutorsReady; 			//Used to signal manager that a tutor is waiting.
sem_t SemStudentsReady; 		//Used to signal manager that a student is waiting.

sem_t SemTutorsWaiting;			//Used to block tutors until a student is found.
sem_t SemStudentsWaiting;		//Used to block students until a tutor is found.

sem_t SemStudentsGettingHelp;	//Used to make students wait for their tutor to "teach something" (sleep).

sem_t SemStudentsFinished;		//Used to signal the manager that a student has finished.
//***************************************************

int TutorsBreak = 0;			//Makes the tutors break and end thread.


/***********************************************************
						Manager
************************************************************/
void *ManagerStart(void *param)
{
	printf("Manager thread created.\n");
	//waits for a tutor
	//waits for a student
	//pairs them
	//repeat
	int pairsNeeded = NUM_STUDENTS * NUM_HELP;
	
	while( pairsNeeded > 0)
	{
		sem_wait(&SemTutorsReady);
		sem_wait(&SemStudentsReady);
		//Found a tutor and a student. Pair them.
		printf("Manager found a student/tutor pair. Waking them up...\n");
		
		sem_post(&SemStudentsWaiting);
		sem_post(&SemTutorsWaiting);
		
		pairsNeeded -= 1;
	}
	
	//Wait until all students have terminated.
	for(int i = 0; i < NUM_STUDENTS; i++)
	{
		sem_wait(&SemStudentsFinished);
	}
	
	//Now it is safe to terminate tutors.
	TutorsBreak = 1;
	for(int i = 0; i < NUM_TUTORS; i++)
	{
		sem_post(&SemTutorsWaiting);
	}
	
	printf("Manager thread finished.\n");
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
		sem_post(&SemTutorsReady);
		sem_wait(&SemTutorsWaiting);
		
		if(TutorsBreak == 1)
			break;
		
		int waitNum = rand() % 3 + 1;
		
		//*********Using MutexChairs***********************
		pthread_mutex_lock(&MutexChairs);
		printf("Tutor %d is helping a student for %d seconds. Waiting Students = %d.\n", num, waitNum, NUM_MAXCHAIRS - EmptyChairs);
		pthread_mutex_unlock(&MutexChairs);
		//*********finished using MutexChairs**************
		
		sleep(waitNum);
		
		sem_post(&SemStudentsGettingHelp);
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
		
		//STUDENT NEEDS HELP NOW, SEARCH FOR WAITING CHAIR
		
		//*********Using MutexChairs**********************
		pthread_mutex_lock(&MutexChairs);
		if( EmptyChairs > 0 )
		{
			EmptyChairs -= 1;
			printf("Student %d takes a seat. Waiting students = %d.\n", num, NUM_MAXCHAIRS - EmptyChairs);
			sem_post(&SemStudentsReady);
			pthread_mutex_unlock(&MutexChairs);
		}
		else
		{
			//all the chairs are taken. End mutex and program more..
			printf("Student %d found no empty chair. Will come back later...\n", num);
			pthread_mutex_unlock(&MutexChairs);
			continue;
		}
		//*********Finished using MutexChairs************
		
		//We now have a waiting chair. Wait for the coordinator.
		sem_wait(&SemStudentsWaiting);
		
		//We are getting help now, get up from chair
		//*********Using MutexChairs**************
		pthread_mutex_lock(&MutexChairs);
		EmptyChairs += 1;
		printf("Student %d left his chair to get help. Waiting students = %d.\n", num, NUM_MAXCHAIRS - EmptyChairs);
		pthread_mutex_unlock(&MutexChairs);
		//*********Finished using MutexChairs**************
		
		//wait for tutor to finish sleeping
		sem_wait(&SemStudentsGettingHelp);
		
		//I HAVE OFFICIALLY BEEN HELPED!
		HelpNeeded -= 1;
		printf("Student %d has been helped. Need help %d more times.\n",num, HelpNeeded);
	}
	
	printf("Student thread %d has finished.\n", num);
	sem_post(&SemStudentsFinished);
	return NULL;
}



/***********************************************************
						MAIN
************************************************************/
int main(int argc, char *argv[])
{
	//Read arguments
	if(argc >= 5)
	{
		NUM_STUDENTS 		= atoi( argv[1] );
		NUM_TUTORS 			= atoi( argv[2] );
		NUM_MAXCHAIRS 		= atoi( argv[3] );
		NUM_HELP 			= atoi( argv[4] );
		
		if(NUM_STUDENTS < 0 || NUM_TUTORS < 0 || NUM_MAXCHAIRS < 0 || NUM_HELP < 0)
		{
			printf("If you want to break my code, you are gonna have to try harder than that.\n");
			printf("Negative arguments are not allowed.");
			return 1;
		}
	}
	else if(argc == 1)
	{
		printf("No arguments read. Using default arguments.\n");
	}
	else
	{
		printf("There was in incorrect number of arguments.\n");
		printf("Either input 4 positive ints, or no arguments to use default args.\n");
		return 1;
	}
	
	EmptyChairs = NUM_MAXCHAIRS;
	
	
	//initialize randomizer
	srand(time(NULL));
	
	//initialize mutex locks.
	if( pthread_mutex_init(&MutexChairs, NULL) != 0)
	{
		printf("Error creating mutex.\n");
		return 1;
	}
	
	//initialize semaphores.
	if( sem_init( &SemTutorsReady, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemStudentsReady, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemTutorsWaiting, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemStudentsWaiting, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemStudentsGettingHelp, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	if( sem_init( &SemStudentsFinished, 0, 0) != 0)
	{
		printf("Error creating semaphore.\n");
		return 1;
	}
	
	
	//******************CREATE THE THREADS***************************
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
		
		//printf("Created tutor %d\n", i);
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
		
		//printf("Created student %d\n", i);
	}
	
	if( pthread_create(&ManagerThread, NULL, ManagerStart, NULL) )
	{
		printf("Error creating manager thread.");
	}
	
	//MANAGER SHOULD NOW DO STUFF.
	//JOIN EVERYTHING BACK UP AFTER MANAGER RETURNS.
	
	//******************JOIN THE THREADS***************************
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
