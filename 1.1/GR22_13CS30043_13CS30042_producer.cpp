// Assignment 5
// Q1. Multiple Producers and Consumers
// Case I: Without Deadlock Avoidance

// Objective
// Implement a system which ensures synchronisation in a producer-consumer scenario. You also have to demonstrate deadlock condition
// and provide solutions for avoiding deadlock.

// Group Details
// Group No: 22
// Member 1: Jeenu Grover (13CS30042)
// Member 2: Ashish Sharma (13CS30043)

// Filename: producer.c

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

using namespace std;

#define MSGSZ 4

#define NO_OF_PRODUCERS 5
#define NO_OF_CONSUMERS 5
#define NO_OF_QUEUES 2


// The Message Queue Struct
struct message
{
    long mtype;
    char mtext[MSGSZ];
};


int KEY,	// Semaphore for controlling access to Critical Section of Queue 0 and Queue 1
    Q0,		// Message Queue 0
    Q1,		// Message Queue 1
    MUTEX,	// Semaphore for controlling mutual exclusion of shared files
    EMPTY_KEY,	// Counts Empty Queue Slots
    FULL_KEY;	// Counts Full Queue Slots


int main(int argc, char * argv[])
{
    umask(000);

    KEY = atoi(argv[2]);
    Q0 = atoi(argv[3]);
    Q1 = atoi(argv[4]);
    MUTEX = atoi(argv[5]);
    EMPTY_KEY = atoi(argv[6]);
    FULL_KEY = atoi(argv[7]);

    printf("i: %d\n",atoi(argv[1]));


    // Get the Semaphore IDs
    int semid = semget(KEY,2,IPC_CREAT|0666);
    if(semid == -1 )printf("Semaphore could not be created\n");

    int mutexid = semget(MUTEX,1,IPC_CREAT|0666);
    if(mutexid == -1 )printf("Mutex Semaphore could not be created\n");

    int emptyid = semget(EMPTY_KEY,2,IPC_CREAT|0666);
    if(emptyid == -1 )printf("Semaphore could not be created\n");

    int fullid = semget(FULL_KEY,2,IPC_CREAT|0666);
    if(fullid == -1 )printf("Semaphore could not be created\n");

    //if(semctl(semid,0,SETVAL,1) == -1)printf("Value of 1st subsemphore could not be set\n");
    //if(semctl(semid,1,SETVAL,1) == -1)printf("Value of 2nd subsemphore could not be set\n");

    // Get the Message Queue IDs
    int q0 = msgget(Q0,IPC_CREAT|0666);
    if(q0 < 0)printf("Message Get Error in Q0\n");

    int q1 = msgget(Q1,IPC_CREAT|0666);
    if(q1 < 0)printf("Message Get Error in Q1\n");

    int rnd,rndnum;
    struct sembuf Sop,mutex,empSop,fulSop;

    FILE * fptr;

    int myid = atoi(argv[1]);

    int i,j,k,inserted,deleted;
    int graph[NO_OF_QUEUES][NO_OF_CONSUMERS+NO_OF_PRODUCERS];

    stringstream ss1;
    while(1)
    {
        //printf("Trying to insert...\n");
        srand(time(NULL));
        rnd = rand()%2;
        rndnum = rand()%50 + 1;
        struct message msg;
        msg.mtype = 1;

        ss1.str(std::string());
        ss1<<rndnum;

        strcpy(msg.mtext,ss1.str().c_str());


        // Check if the queue is empty
        empSop.sem_num = rnd;
        empSop.sem_op = -1;
        empSop.sem_flg = IPC_NOWAIT;
        if(semop(emptyid,&empSop,1)<0) continue;

        // Update the Matrix

        //Start
        mutex.sem_num = 0;
        mutex.sem_op = -1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1);

        fptr = fopen("matrix.txt","r");

        for(i=0; i<NO_OF_QUEUES; i++)
            for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
            {
                fscanf(fptr,"%d",&graph[i][j]);
            }

        fclose(fptr);

        graph[rnd][myid] = 1;

        fptr = fopen("matrix.txt","w");
        for(i=0; i<NO_OF_QUEUES; i++)
        {
            for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
            {
                fprintf(fptr,"%d ",graph[i][j]);
            }
            fprintf(fptr, "\n");
        }
        fclose(fptr);

        mutex.sem_num = 0;
        mutex.sem_op = 1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1);
        //End


        // Insert in Queue 0 if rnd == 0
        if(rnd == 0)
        {
            cout<<"Trying to Insert in Q0: "<<msg.mtext<<endl;
            Sop.sem_num = 0;
            Sop.sem_op = -1;
            Sop.sem_flg = 0;
            semop(semid, &Sop, 1);

            printf("Producer %d: Acquired Q0\n",myid);

            // Update the Matrix
            //Start
            mutex.sem_num = 0;
            mutex.sem_op = -1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);

            fptr = fopen("matrix.txt","r");

            for(i=0; i<NO_OF_QUEUES; i++)
                for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
                {
                    fscanf(fptr,"%d",&graph[i][j]);
                }

            fclose(fptr);

            graph[rnd][myid] = 2;

            fptr = fopen("matrix.txt","w");
            for(i=0; i<NO_OF_QUEUES; i++)
            {
                for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
                {
                    fprintf(fptr,"%d ",graph[i][j]);
                }
                fprintf(fptr, "\n");
            }
            fclose(fptr);


            fptr = fopen("result0.txt","r");
            fscanf(fptr,"%d %d",&inserted,&deleted);
            fclose(fptr);



            fptr = fopen("result0.txt","w");
            fprintf(fptr,"%d %d\n",inserted+1,deleted);
            fclose(fptr);

            mutex.sem_num = 0;
            mutex.sem_op = 1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);
            //End




            if(msgsnd(q0,&msg,MSGSZ,IPC_NOWAIT)<0)printf("Production Failure in Queue Q0\n");
            else cout<<"Successfully Inserted in Q0: "<<msg.mtext<<endl;


            Sop.sem_num = 0;
            Sop.sem_op = 1;
            Sop.sem_flg = 0;
            semop(semid,&Sop,1);

            // Release the Semaphore

            printf("Producer %d: Released Q0\n",myid);

            // UP(Full)
            fulSop.sem_num = 0;
            fulSop.sem_op = 1;
            fulSop.sem_flg = 0;
            semop(fullid,&fulSop,1);
        }
        else
        {
            cout<<"Trying Inserted in Q1: "<<msg.mtext<<endl;
            Sop.sem_num = 1;
            Sop.sem_op = -1;
            Sop.sem_flg = 0;
            semop(semid, &Sop, 1);

            printf("Producer %d: Acquired Q1\n",myid);

            //Start
            mutex.sem_num = 0;
            mutex.sem_op = -1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);

            fptr = fopen("matrix.txt","r");

            for(i=0; i<NO_OF_QUEUES; i++)
                for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
                {
                    fscanf(fptr,"%d",&graph[i][j]);
                }

            fclose(fptr);

            graph[rnd][myid] = 2;

            fptr = fopen("matrix.txt","w");
            for(i=0; i<NO_OF_QUEUES; i++)
            {
                for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
                {
                    fprintf(fptr,"%d ",graph[i][j]);
                }
                fprintf(fptr, "\n");
            }
            fclose(fptr);

            fptr = fopen("result1.txt","r");
            fscanf(fptr,"%d %d",&inserted,&deleted);
            fclose(fptr);



            fptr = fopen("result1.txt","w");
            fprintf(fptr,"%d %d\n",inserted+1,deleted);
            fclose(fptr);

            mutex.sem_num = 0;
            mutex.sem_op = 1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);
            //End


            if(msgsnd(q1,&msg,MSGSZ,IPC_NOWAIT)<0)printf("Production Failure in Queue Q1\n");
            else cout<<"Inserted in Q1: "<<msg.mtext<<endl;

            Sop.sem_num = 1;
            Sop.sem_op = 1;
            Sop.sem_flg = 0;
            semop(semid,&Sop,1);

            printf("Producer %d: Released Q1\n",myid);

            fulSop.sem_num = 1;
            fulSop.sem_op = 1;
            fulSop.sem_flg = 0;
            semop(fullid,&fulSop,1);
        }


        //Start
        mutex.sem_num = 0;
        mutex.sem_op = -1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1);

        fptr = fopen("matrix.txt","r");

        for(i=0; i<NO_OF_QUEUES; i++)
            for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
            {
                fscanf(fptr,"%d",&graph[i][j]);
            }

        fclose(fptr);

        graph[rnd][myid] = 0;

        fptr = fopen("matrix.txt","w");
        for(i=0; i<NO_OF_QUEUES; i++)
        {
            for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
            {
                fprintf(fptr,"%d ",graph[i][j]);
            }
            fprintf(fptr, "\n");
        }
        fclose(fptr);

        mutex.sem_num = 0;
        mutex.sem_op = 1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1);
        //End

        usleep((rand()%10)*10000);
    }




    return 0;
}
