//tree var 7 signals pattern var 9

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define NODES_COUNT 8
#define SIGNALS_COUNT 101

const int TreeLength = sizeof(int) * NODES_COUNT;

typedef struct ProccessNode {
	int PID;
	int *Children;
	int ChildrensCount;
	struct sigaction ActionHandler;
	int ReceivedSignal;
	sigset_t IgnoredSignals;
} ProccessNode;

ProccessNode* Tree;

void KillProcesses()
{
	for (int i = 1; i <= NODES_COUNT; ++i)
	{
		if (Tree[i].ChildrensCount > 0)
		{
			free(Tree[i].Children);
		}
		
		if ((kill(Tree[i].PID, 0) != -1))
		{
			kill(Tree[i].PID, SIGKILL);
		}
	}

	if (munmap(Tree, TreeLength) == -1)
	{
		perror("Cannot clean shared memory");
	}
}

void PrintMsgAndExit(char *str)
{
	perror(str);
	KillProcesses();
    exit(-1);
}

sigset_t CreateEmptySet()
{
	sigset_t set;
	if (sigemptyset(&set) == -1)
    {
		PrintMsgAndExit("Cannot create root signal set");
    } 
	return set;
}

void* CreateSharedMemory()
{
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_SHARED | MAP_ANONYMOUS;
	return mmap(NULL, TreeLength, protection, visibility, -1, 0);
}

struct sigaction CreateAction(void (*handler) (int, siginfo_t*, void*), sigset_t signals)
{
    struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_sigaction = handler;	
	action.sa_mask = signals;
	action.sa_flags = SA_SIGINFO;
    return action;
}

long long GetTime() 
{
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) == -1) 
	{
		perror("Cannot get current time");
		return -1;
	}	
		
	return tv.tv_sec * 1000000 + tv.tv_usec; 
}

int SIGUSR1_count = 0, SIGUSR2_count = 0;

struct sigaction actTerm;

void ReceiveSignal(int sig, int ordChild) 
{
	long long time = GetTime();
	printf("[%d] PID: %d PPID: %d <RECEIVE> SIGUSR%d Time: %lld\n", ordChild, getpid(), getppid(), sig == SIGUSR1 ? 1 : 2, time);
	
	if (sig == SIGUSR1)
    {
		SIGUSR1_count++;
    }
	else if (sig == SIGUSR2)
    {
		SIGUSR2_count++;
    }
}

void TerminateProcess(int sig, siginfo_t *siginfo, void *code)
{
	int pid = getpid();
	int nodeNumber = 1;
	while (nodeNumber <= NODES_COUNT && Tree[nodeNumber].PID != pid)
	{
		++nodeNumber;
	}
	
	if (Tree[nodeNumber].ChildrensCount > 0)
	{
		if (killpg(Tree[nodeNumber].PID, SIGTERM) == -1)
		{
			PrintMsgAndExit("Cannot send terminatend signal from child to other nodes");	
		}
	}

	for (int i = 0; i < Tree[nodeNumber].ChildrensCount; ++i)
	{
		int childNumber = Tree[nodeNumber].Children[i];

		if (waitpid(Tree[childNumber].PID, NULL, 0) == -1)
        {
            perror("Cannot wait child process");
        }
	}
	printf("PID: %d PPID: %d <FINISHED> after %ds SIGUSR1 and %ds SIGUSR2\n", pid, getppid(), SIGUSR1_count, SIGUSR2_count);	
	exit(0);	
}

void Child1Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 1);
	
	if (SIGUSR1_count + SIGUSR2_count == SIGNALS_COUNT) 
    {
		TerminateProcess(0, NULL, NULL);
	}
	
	if (killpg(Tree[1].PID, SIGUSR2) == -1)
	{
		PrintMsgAndExit("Cannot send signal from child 1 to other nodes");	
	}	
	long long time = GetTime();
	printf("[%d] PID: %d PPID: %d <SENT> SIGUSR2 Time: %lld\n", 1, getpid(), getppid(), time);
}

void Child2Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 2);
}

void Child3Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 3);
}

void Child4Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 4);
}

void Child5Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 5);	
}

void Child6Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 6);
	
	if (killpg(Tree[6].PID, SIGUSR1) == -1)
	{
		PrintMsgAndExit("Cannot sent signal from child 6 to other nodes");
	}
	long long time = GetTime();
	printf("[%d] PID: %d PPID: %d <SENT> SIGUSR1 Time: %lld\n", 6, getpid(), getppid(), time);
}

void Child7Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 7);	
}

void Child8Handler(int sig, siginfo_t *siginfo, void *code) 
{
	ReceiveSignal(sig, 8);

	if (kill(Tree[1].PID, SIGUSR2) == -1)
    {
		PrintMsgAndExit("Cannot sent signal from child 8 to other nodes");
    }
	long long time = GetTime();
	printf("[%d] PID: %d PPID: %d <SENT> SIGUSR2 Time: %lld\n", 8, getpid(), getppid(), time);
}

void InitializeProcessTree(ProccessNode *node) 
{
	for (int i = 0; i <= NODES_COUNT; i++) 
    {
		memset(&node[i], 0, sizeof(node[i]));
	}
	
	sigset_t usr1_signalSet = CreateEmptySet(), usr2_signalSet = CreateEmptySet(), root_signalSet = CreateEmptySet();

	if (sigaddset(&usr1_signalSet, SIGUSR1) == -1) 
    {
        PrintMsgAndExit("Cannot add SIGUSR1 to USR1 signal set");
    }
	
	if (sigaddset(&usr2_signalSet, SIGUSR2) == -1) 
    {
		PrintMsgAndExit("Cannot add SIGUSR2 to USR2 signal set");
    }
	
	if (sigaddset(&root_signalSet, SIGUSR1) == -1)
    {
		PrintMsgAndExit("Cannot add SIGUSR1 to root signal set");
    } 
	if (sigaddset(&root_signalSet, SIGUSR2) == -1) 
    {
		PrintMsgAndExit("Cannot add SIGUSR2 to root signal set");
    }
	if (sigaddset(&root_signalSet, SIGTERM) == -1) 
    {
		PrintMsgAndExit("Cannot add SIGTERM to root signal set");
    }
	
	node[0].ChildrensCount = 1;
	node[0].Children = (int*)malloc(sizeof(int) * node[0].ChildrensCount);
	if (node[0].Children == NULL)
    {
		PrintMsgAndExit("Cannot allocate memory for root children array");
    }
	node[0].Children[0] = 1;
	node[0].IgnoredSignals = root_signalSet;
	
	node[1].ChildrensCount = 5;
	node[1].Children = (int*)malloc(sizeof(int) * node[1].ChildrensCount);
	if (node[1].Children == NULL)
    {
		PrintMsgAndExit("Cannot allocate memory for node1 children array");
    }
	node[1].Children[0] = 2;
    node[1].Children[1] = 3;
    node[1].Children[2] = 4;
    node[1].Children[3] = 5;
    node[1].Children[4] = 6;
	node[1].ReceivedSignal = SIGUSR2;
	node[1].ActionHandler = CreateAction(Child1Handler, usr2_signalSet);
	node[1].IgnoredSignals = usr1_signalSet;
	if (sigaddset(&node[1].IgnoredSignals, SIGTERM) == -1)
    {
		PrintMsgAndExit("Cannot add SIGTERM to node1 ignore set");	
    }
	
	node[2].ChildrensCount = 0;
	node[2].ReceivedSignal = SIGUSR2;
	node[2].ActionHandler = CreateAction(Child2Handler, usr2_signalSet);
	
    node[3].ChildrensCount = 0;
	node[3].ReceivedSignal = SIGUSR2;
	node[3].ActionHandler = CreateAction(Child3Handler, usr2_signalSet);
	
	node[4].ChildrensCount = 0;
	node[4].ReceivedSignal = SIGUSR2;
	node[4].ActionHandler = CreateAction(Child4Handler, usr2_signalSet);

	node[5].ChildrensCount = 0;	
	node[5].ReceivedSignal = SIGUSR2;
	node[5].ActionHandler = CreateAction(Child5Handler, usr2_signalSet);
	
	node[6].ChildrensCount = 2;
	node[6].Children = (int*)malloc(sizeof(int) * node[6].ChildrensCount);
	if (node[6].Children == NULL)
    {
		PrintMsgAndExit("Cannot allocate a memory");
    }
	node[6].Children[0] = 7;
    node[6].Children[1] = 8;
	node[6].ReceivedSignal = SIGUSR2;
	node[6].IgnoredSignals = usr1_signalSet;
	node[6].ActionHandler = CreateAction(Child6Handler, usr2_signalSet);
	
	node[7].ChildrensCount = 0;
	node[7].ReceivedSignal = SIGUSR1;
	node[7].ActionHandler = CreateAction(Child7Handler, usr1_signalSet);
	
	node[8].ChildrensCount = 0;
	node[8].ReceivedSignal = SIGUSR1;
	node[8].IgnoredSignals = usr2_signalSet;
	node[8].ActionHandler = CreateAction(Child8Handler, usr1_signalSet);
}

void StartNode(int nodeNumber)
{
	int pid = fork();
	if (pid == 0)
	{
		if (sigprocmask(SIG_SETMASK, &Tree[nodeNumber].IgnoredSignals, 0) == -1) 
        {
			PrintMsgAndExit("Cannot set child blocked signal list");
		}

		if (sigaction(Tree[nodeNumber].ReceivedSignal, &Tree[nodeNumber].ActionHandler, 0) == -1) 
        {
			PrintMsgAndExit("Cannot set child signal hanlder");
		}

		sigset_t set = CreateEmptySet();
		if (sigaddset(&set, SIGTERM) == -1) 
    	{	
			PrintMsgAndExit("Cannot add SIGTERM to root signal set");
    	}

		struct sigaction action = CreateAction(TerminateProcess, set);
		if (sigaction(SIGTERM, &action, 0) == -1)
		{
			PrintMsgAndExit("Cannot set child SIGTERM hanlder");
		}

		pid = getpid();

		if (Tree[nodeNumber].ChildrensCount > 0)
		{
			printf("New process group id: %d\n", pid);
			if (setpgid(pid, pid) == -1)
			{
				PrintMsgAndExit("Cannot create new process group");
			}
		}

		for (int i = 0; i < Tree[nodeNumber].ChildrensCount; ++i)
		{
			int childNumber = Tree[nodeNumber].Children[i];
			StartNode(childNumber);
			while (Tree[childNumber].PID == 0) {}
			int pidBefore = getpgid(Tree[childNumber].PID);
			int childPid = Tree[childNumber].PID;
			if (setpgid(Tree[childNumber].PID, pid) == -1)
			{
				fprintf(stderr, "Cannot add child process %d to parent group %d", childNumber, nodeNumber);
				KillProcesses();
				exit(-1);
			}			
			int pidAfter = getpgid(Tree[childNumber].PID);
			//printf("Chilld [%d] PID [%d] Before [%d] After [%d]\n", childNumber, childPid, pidBefore, pidAfter);
		}

		if (Tree[nodeNumber].ChildrensCount > 0)
		{
			pid_t parentPid = getppid();

			if (setpgid(pid, parentPid) == -1)
			{
				PrintMsgAndExit("Cannot reattach process to parent process group");
			}
		}

		long long time = GetTime();
		printf("PID: %d PPID: %d GroupID: %d Time: %ld CHILD%d <CREATED>\n", pid, getppid(), getpgid(pid), time, nodeNumber);

        while (1) {};
	}
	else if (pid > 0)
	{
		Tree[nodeNumber].PID = pid;
	}
	else 
	{
		PrintMsgAndExit("Child creation failed");
	}
}

void main(void) 
{
	Tree = (ProccessNode*)CreateSharedMemory(sizeof(ProccessNode) * NODES_COUNT);
	InitializeProcessTree(Tree);
	StartNode(1);

	for (int i = 1; i <= NODES_COUNT; ++i)
	{
		while (Tree[i].PID == 0) {}
	}

	if (kill(Tree[1].PID, SIGUSR2) == -1)
	{
		PrintMsgAndExit("Cannot statrt processing");
	}
	
	if (waitpid(Tree[1].PID, NULL, 0) == -1)
	{
		PrintMsgAndExit("Cannot wait for first process");
	}
	
	KillProcesses();
}