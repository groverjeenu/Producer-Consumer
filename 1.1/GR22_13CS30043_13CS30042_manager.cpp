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

// Filename: manager.c


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
#include <list>
#include <vector>
#include <sys/stat.h>
#include <sstream>

using namespace std;

// #define KEY 1236
// #define Q0 332
// #define Q1 443
// #define MUTEX 2237
#define MSGSZ 4


#define NO_OF_PRODUCERS 5
#define NO_OF_CONSUMERS 5
#define NO_OF_QUEUES 2

#define BUFFER_SIZE 10


struct message
{
    long mtype;
    char mtext[MSGSZ];
};


int pid[50], graph[NO_OF_QUEUES][NO_OF_PRODUCERS+NO_OF_CONSUMERS];
int V = NO_OF_CONSUMERS + NO_OF_PRODUCERS + NO_OF_QUEUES;
int Graph[NO_OF_CONSUMERS + NO_OF_PRODUCERS + NO_OF_QUEUES][NO_OF_CONSUMERS + NO_OF_PRODUCERS + NO_OF_QUEUES] ;
vector<int> nodes;


//Standard Code taken from GEEKS FOR GEEKS

bool isCyclicUtil(int v, bool visited[], bool *recStack)
{
    nodes.push_back(v);
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;

        // Recur for all the vertices adjacent to this vertex
        int i;
        for(i = 0; i < V; ++i)
        {
            if(Graph[v][i] !=0)
            {
                if ( !visited[i] && isCyclicUtil(i, visited, recStack) )
                    return true;
                else if (recStack[i])
                {
                    nodes.push_back(i);
                    return true;
                }
            }
        }

    }
    recStack[v] = false;  // remove the vertex from recursion stack
    nodes.pop_back();
    return false;
}

// Returns true if the graph contains a cycle, else false.
bool isCyclic()
{
    // Mark all the vertices as not visited and not part of recursion
    // stack
    bool *visited = new bool[V];
    bool *recStack = new bool[V];
    nodes.clear();
    for(int i = 0; i < V; i++)
    {
        visited[i] = false;
        recStack[i] = false;
    }

    // Call the recursive helper function to detect cycle in different
    // DFS trees
    for(int i = 0; i < V; i++)
        if (isCyclicUtil(i, visited, recStack))
            return true;

    return false;
}


int KEY,	// Semaphore for controlling access to Critical Section of Queue 0 and Queue 1
    Q0,		// Message Queue 0
    Q1,		// Message Queue 1
    MUTEX,	// Semaphore for controlling mutual exclusion of shared files
    EMPTY_KEY,	// Counts Empty Queue Slots
    FULL_KEY;	// Counts Full Queue Slots


int main(int argc, char * argv[])
{
    if(argc<8)
    {
        printf("Usage: %s p KEY Q0 Q1 MUTEX EMPTY_KEY FULL_KEY\n",argv[0]);
        printf("Try Again\n");
        exit(1);
    }

    KEY = atoi(argv[2]);
    Q0 = atoi(argv[3]);
    Q1 = atoi(argv[4]);
    MUTEX = atoi(argv[5]);
    EMPTY_KEY = atoi(argv[6]);
    FULL_KEY = atoi(argv[7]);
    umask(000);


    // Create the respective Semaphores
    int semid = semget(KEY,2,IPC_CREAT|0666);
    if(semid == -1 )printf("Semaphore could not be created\n");

    int mutexid = semget(MUTEX,1,IPC_CREAT|0666);
    if(mutexid == -1 )printf("Mutex Semaphore could not be created\n");

    int emptyid = semget(EMPTY_KEY,2,IPC_CREAT|0666);
    if(emptyid == -1 )printf("Semaphore could not be created\n");

    int fullid = semget(FULL_KEY,2,IPC_CREAT|0666);
    if(fullid == -1 )printf("Semaphore could not be created\n");


    // Set the Values of Semaphores
    if(semctl(semid,0,SETVAL,1) == -1)printf("Value of 1st subsemphore could not be set\n");
    if(semctl(semid,1,SETVAL,1) == -1)printf("Value of 2nd subsemphore could not be set\n");
    if(semctl(mutexid,0,SETVAL,1) == -1)printf("Value of mutex subsemphore could not be set\n");
    if(semctl(fullid,0,SETVAL,0) == -1)printf("Value of FULL 0 subsemphore could not be set\n");
    if(semctl(fullid,1,SETVAL,0) == -1)printf("Value of FULL 1 subsemphore could not be set\n");
    if(semctl(emptyid,0,SETVAL,BUFFER_SIZE) == -1)printf("Value of EMPTY 0 subsemphore could not be set\n");
    if(semctl(emptyid,1,SETVAL,BUFFER_SIZE) == -1)printf("Value of EMPTY 1 subsemphore could not be set\n");


    // Create Message Queues
    int q0 = msgget(Q0,IPC_CREAT|0666);
    if(q0 < 0)printf("Message Get Error in Q0\n");

    int q1 = msgget(Q1,IPC_CREAT|0666);
    if(q1 < 0)printf("Message Get Error in Q1\n");

    // Set the Size of the message queues
    struct msqid_ds qstat;
    qstat.msg_qbytes = 400000;
    if(msgctl(q0,IPC_SET,&qstat)<0)printf("Initialisation error of message queue Q0\n");

    qstat.msg_qbytes = 400000;
    if(msgctl(q1,IPC_SET,&qstat)<0)printf("Initialisation error of message queue Q1\n");


    int i,j,k,val,flg,v1,v2;
    stringstream ss;

    struct sembuf mutex;

    FILE * fptr;

    // Start of initialization of files

    // Request access to Shared file
    mutex.sem_num = 0;
    mutex.sem_op = -1;
    mutex.sem_flg = 0;
    semop(mutexid,&mutex,1);	// Wait for access to be granted


    // Initialize the matrix
    close(open("matrix.txt", O_RDWR|O_CREAT, 0666));
    fptr = fopen("matrix.txt","w");
    for(i=0; i<NO_OF_QUEUES; i++)
    {
        for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
        {
            fprintf(fptr,"%d ",0);
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);

    // Initialize the Insertions and deletions of the 2 message queues
    close(open("result0.txt", O_RDWR|O_CREAT, 0666));
    fptr = fopen("result0.txt","w");
    fprintf(fptr,"%d %d\n",0,0);
    fclose(fptr);

    close(open("result1.txt", O_RDWR|O_CREAT, 0666));
    fptr = fopen("result1.txt","w");
    fprintf(fptr,"%d %d\n",0,0);
    fclose(fptr);

    mutex.sem_num = 0;
    mutex.sem_op = 1;
    mutex.sem_flg = 0;
    semop(mutexid,&mutex,1);	// Release the resource
    //End of initialization of files


    // Create Producer Process
    for( i = 0 ; i< NO_OF_PRODUCERS; i++)
    {
        ss.str(std::string());
        ss<<i;
        pid[i] = fork();

        if( pid[i] == 0)
        {
            execlp("xterm" ,"xterm" ,"-hold","-e","./producer",ss.str().c_str(),argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],(const char *)NULL);
            exit(1);

        }
    }


    // Create Consumer Processes
    for (; i < NO_OF_PRODUCERS + NO_OF_CONSUMERS ; i++)
    {
        ss.str(std::string());
        ss<<i;
        pid[i] = fork();

        if( pid[i] == 0)
        {
            execlp("xterm" ,"xterm" ,"-hold","-e","./consumer",ss.str().c_str(),argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],(const char *)NULL);

            exit(1);
        }
    }

    // Print the PIDs
    for(int j=0; j<NO_OF_CONSUMERS+NO_OF_PRODUCERS; ++j)
    {
        cout<<pid[j]<<" ";
    }
    cout<<endl;

    int ij = 0;

    while(1)
    {
        ij++;
        printf("Loop %d\n",ij);

        // Request access to Shared file
        mutex.sem_num = 0;
        mutex.sem_op = -1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1); // Wait for access to be granted

        fptr = fopen("matrix.txt","r");

        for(i=0; i<NO_OF_QUEUES; i++)
            for(j = 0 ; j<NO_OF_CONSUMERS + NO_OF_PRODUCERS ; j++)
            {
                fscanf(fptr,"%d",&graph[i][j]);
            }

        fclose(fptr);

        // Check for cycle -- Deadlock

        for( i = 0 ; i < NO_OF_CONSUMERS+NO_OF_PRODUCERS+NO_OF_QUEUES ; i++)
            for ( j=0 ; j < NO_OF_CONSUMERS+NO_OF_PRODUCERS+NO_OF_QUEUES ; j++)
                Graph[i][j] = 0;

        for( i= 0; i<NO_OF_QUEUES; i++)
            for(j = 0; j< NO_OF_PRODUCERS+NO_OF_CONSUMERS; j++)
            {

                if(graph[i][j] == 1)
                    Graph[j][NO_OF_PRODUCERS + NO_OF_CONSUMERS + i] = 1;
                else if (graph[i][j] == 2)
                    Graph[NO_OF_PRODUCERS + NO_OF_CONSUMERS + i][j] = 1;
            }

        if(isCyclic())
        {
            // If Yes, Print the cycle and exit
            printf("Cycle Detected\n");
            val = *(nodes.end() -1) ;
            flg = 0;

            for( i = 0 ; i < (nodes.size()-1); i++)
            {
                if(nodes[i] == val)
                {
                    flg = 1;
                }
                if(flg)
                {
                    if(nodes[i]<NO_OF_PRODUCERS)
                        printf("P%d --> ",nodes[i]);
                    else if(nodes[i]>= NO_OF_PRODUCERS && nodes[i]<NO_OF_CONSUMERS + NO_OF_PRODUCERS)
                        printf("C%d --> ",nodes[i]-NO_OF_PRODUCERS);
                    else if(nodes[i] >= NO_OF_PRODUCERS+NO_OF_CONSUMERS)
                        printf("Q%d --> ",nodes[i]- (NO_OF_PRODUCERS+NO_OF_CONSUMERS));


                }

            }
            if(nodes[i]<NO_OF_PRODUCERS)
                printf("P%d\n",nodes[i]);
            else if(nodes[i]>= NO_OF_PRODUCERS && nodes[i]<NO_OF_CONSUMERS + NO_OF_PRODUCERS)
                printf("C%d\n",nodes[i]-NO_OF_PRODUCERS);
            else if(nodes[i] >= NO_OF_PRODUCERS+NO_OF_CONSUMERS)
                printf("Q%d\n",nodes[i]- (NO_OF_PRODUCERS+NO_OF_CONSUMERS));
            //return 0;
            for(int j=0; j<NO_OF_CONSUMERS+NO_OF_PRODUCERS; ++j)
            {

                kill(pid[j],SIGTERM);
            }
            return 0;
        }

        //v1  = semctl(fullid,0,GETVAL,0);




        mutex.sem_num = 0;
        mutex.sem_op = 1;
        mutex.sem_flg = 0;
        semop(mutexid,&mutex,1);	// Release the resource
        //End

        sleep(2);

    }

    return 0;
}
