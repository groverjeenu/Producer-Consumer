// Assignment 5
// Q1. Multiple Producers and Consumers
// Case II: With Deadlock Avoidance

// Objective
// Implement a system which ensures synchronisation in a producer-consumer scenario. You also have to demonstrate deadlock condition
// and provide solutions for avoiding deadlock.

// Group Details
// Group No: 22
// Member 1: Jeenu Grover (13CS30042)
// Member 2: Ashish Sharma (13CS30043)

// Filename: consumer.c


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
#include <sys/stat.h>

using namespace std;

// #define KEY 1236
// #define Q0 332
// #define Q1 443
// #define MUTEX 2237
#define MSGSZ 4

#define NO_OF_PRODUCERS 5
#define NO_OF_CONSUMERS 5
#define NO_OF_QUEUES 2


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
    KEY = atoi(argv[3]);
    Q0 = atoi(argv[4]);
    Q1 = atoi(argv[5]);
    MUTEX = atoi(argv[6]);
    EMPTY_KEY = atoi(argv[7]);
    FULL_KEY = atoi(argv[8]);

    // Get the Semaphore
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

    // Get the Message Queues
    int q0 = msgget(Q0,IPC_CREAT|0666);
    if(q0 < 0)printf("Message Get Error in Q0\n");

    int q1 = msgget(Q1,IPC_CREAT|0666);
    if(q1 < 0)printf("Message Get Error in Q1\n");

    int rnd,rndQueue;
    struct sembuf Sop,mutex,fulSop,empSop;

    FILE * fptr;

    int myid = atoi(argv[1]);


    int i,j,k,inserted,deleted;
    int graph[NO_OF_QUEUES][NO_OF_CONSUMERS+NO_OF_PRODUCERS];


    int  p = (int)(atof(argv[2])*1000);
    int low = (1000-p)/2;
    int high = 1000 - low ;


    while(1)
    {

        srand(time(NULL));
        rnd = rand()%1000;

        cout<<rnd<<" "<<low<<" "<<high<<endl;

        struct message msg;
        // msg.mtype = 1;
        // msg.mtext[0] = rndnum>>24 & 0xFF;
        // msg.mtext[1] = rndnum>>16 & 0xFF;
        // msg.mtext[2] = rndnum>>8 & 0xFF;
        // msg.mtext[3] = rndnum & 0xFF;


        if(rnd >=low && rnd <=high)
        {
            rndQueue = rand()%2;
            printf("Consumer %d: Trying to Read from just Q%d\n",myid,rndQueue);
        }
        else
        {
            printf("Consumer %d: Trying to Read from Both Queues\n",myid);
            rndQueue = 0;
        }


        // Update the matrix
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

        graph[rndQueue][myid] = 1;

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


        // Consuming from one queue
        if(rnd >=low && rnd <=high)
        {
            printf("Consumer %d: Trying to consume from Q%d\n",myid,rndQueue);
            fulSop.sem_num = rndQueue;
            fulSop.sem_op = -1;
            fulSop.sem_flg = 0;
            semop(fullid,&fulSop,1);

            Sop.sem_num = rndQueue;
            Sop.sem_op = -1;
            Sop.sem_flg = 0;
            semop(semid, &Sop, 1);

            printf("Consumer %d: Acquired Q%d\n",myid,rndQueue);

            // Update the matrix
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

            graph[rndQueue][myid] = 2;

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



            if(rndQueue == 0)
            {
                fptr = fopen("result0.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result0.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }

            else
            {
                fptr = fopen("result1.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result1.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }




            mutex.sem_num = 0;
            mutex.sem_op = 1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);
            //End




            if(rndQueue == 0)
            {
                if(msgrcv(q0 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q0\n",myid,msg.mtext);
            }
            else
            {
                if(msgrcv(q1 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q1\n",myid,msg.mtext);
            }


            Sop.sem_num = rndQueue;
            Sop.sem_op = 1;
            Sop.sem_flg = 0;
            semop(semid,&Sop,1);

            printf("Consumer %d: Released Resource Q%d\n",myid,rndQueue);

            empSop.sem_num = rndQueue;
            empSop.sem_op = 1;
            empSop.sem_flg = 0;
            semop(emptyid,&empSop,1);

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

            graph[rndQueue][myid] = 0;

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
        }

        else
        {
            // Consume from Both Queues
            printf("Consumer %d: Trying to consume from Q%d\n",myid,rndQueue);
            fulSop.sem_num = rndQueue;
            fulSop.sem_op = -1;
            fulSop.sem_flg = 0;
            semop(fullid,&fulSop,1);

            fulSop.sem_num = 1 - rndQueue;
            fulSop.sem_op = -1;
            fulSop.sem_flg = 0;
            semop(fullid,&fulSop,1);


            Sop.sem_num = rndQueue;
            Sop.sem_op = -1;
            Sop.sem_flg = 0;
            semop(semid, &Sop, 1);

            printf("Consumer %d: Acquired Q%d\n",myid,rndQueue);


            // Update the matrix
            //---------------------Start
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

            graph[rndQueue][myid] = 2;

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

            if(rndQueue == 0)
            {
                fptr = fopen("result0.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result0.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }

            else
            {
                fptr = fopen("result1.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result1.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }

            mutex.sem_num = 0;
            mutex.sem_op = 1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);
            //-----------------------------End



            if(rndQueue == 0)
            {
                if(msgrcv(q0 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q0\n",myid,msg.mtext);
            }
            else
            {
                if(msgrcv(q1 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q1\n",myid,msg.mtext);
            }


            //--------------------Start
            mutex.sem_num = 0;
            mutex.sem_op = -1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);

            printf("Consumer : Trying to Enter Critical Section 2 \n");

            fptr = fopen("matrix.txt","r");

            for(i=0; i<NO_OF_QUEUES; i++)
                for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
                {
                    fscanf(fptr,"%d",&graph[i][j]);
                }

            fclose(fptr);

            graph[1-rndQueue][myid] = 1;

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
            //-------------------------End



            //start entering 2nd queue

            printf("Consumer %d: Trying to consume from Q%d\n",myid,(1-rndQueue));


            Sop.sem_num = 1 - rndQueue;
            Sop.sem_op = -1;
            Sop.sem_flg = 0;
            semop(semid, &Sop, 1);

            printf("Consumer %d: Acquired Q%d\n",myid,(1-rndQueue));

            // Update the Matrix
            //---------------------Start
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

            graph[1 - rndQueue][myid] = 2;

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


            if(rndQueue == 1)
            {
                fptr = fopen("result0.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result0.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }

            else
            {
                fptr = fopen("result1.txt","r");
                fscanf(fptr,"%d %d",&inserted,&deleted);
                fclose(fptr);



                fptr = fopen("result1.txt","w");
                fprintf(fptr,"%d %d\n",inserted,deleted+1);
                fclose(fptr);
            }

            mutex.sem_num = 0;
            mutex.sem_op = 1;
            mutex.sem_flg = 0;
            semop(mutexid,&mutex,1);
            //-----------------------------End




            if((1 - rndQueue) == 0)
            {
                if(msgrcv(q0 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q0\n",myid,msg.mtext);
            }
            else
            {
                if(msgrcv(q1 ,&msg,MSGSZ,0,IPC_NOWAIT) < 0 ) printf("msrcv Error\n");
                printf("Consumer %d: Consumed %s from Q1\n",myid,msg.mtext);
            }



            Sop.sem_num = rndQueue;
            Sop.sem_op = 1;
            Sop.sem_flg = 0;
            semop(semid,&Sop,1);

            printf("Consumer %d: Released Resource Q%d\n",myid,rndQueue);

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

            graph[rndQueue][myid] = 0;

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



            Sop.sem_num = 1 - rndQueue;
            Sop.sem_op = 1;
            Sop.sem_flg = 0;
            semop(semid,&Sop,1);

            printf("Consumer %d: Released Resource Q%d\n",myid,(1-rndQueue));

            empSop.sem_num = 1 - rndQueue;
            empSop.sem_op = 1;
            empSop.sem_flg = 0;
            semop(emptyid,&empSop,1);

            empSop.sem_num = rndQueue;
            empSop.sem_op = 1;
            empSop.sem_flg = 0;
            semop(emptyid,&empSop,1);



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

            graph[1 - rndQueue][myid] = 0;

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
        }
        //usleep((rand()%10)*1000);
    }

    return 0;
}
