#include<bits/stdc++.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<time.h>
#include<signal.h>
#include<pthread.h>
#include <sys/syscall.h>

using namespace std;

#define SIZE 15
#define PSIZE 10

pthread_mutex_t m;
int buff[SIZE];
int cnt = 0;
int in = 0;
int out = 0;
int job_done = 0;


struct job{
	pid_t prod_id; 
	int prod_no; 
	int priority; 
	int time; 
	int job_id;

	job(){}

	job(int id, int no, int prio, int t, int j_id){
		prod_id = id;
		prod_no = no;
		priority = prio;
		time = t;
		job_id = j_id;
	}

	void print(){
		cout << "Producer: "<< prod_no <<", Producer TID: "<< prod_id <<", ";
		cout << "Priority: "<< priority <<", Compute Time: "<< time <<", ";
		cout << "Job ID: "  << job_id <<endl;
	}
};

struct Jobs{
	job jobs_queue[SIZE+1];
	int job_created, job_completed;
	int numJobs;
	pthread_mutex_t m;
};
struct Jobs H;

bool cmp(job a, job b){
	return a.priority > b.priority;
}

void heapify_up(Jobs& H, int pos){
    while((pos/2)>0)
    {
        job par = H.jobs_queue[pos/2];
        job cur = H.jobs_queue[pos];
        if(cmp(par, cur)) 
			return;
        else
			H.jobs_queue[pos]=par, H.jobs_queue[pos/2]=cur;
        pos/=2;
    }
}

void heapify_down(Jobs& H, int pos){
    int left = 2*pos, right = 2*pos+1, n = H.numJobs, smallest;
    job l, r;
    if(left <= n){
    	l = H.jobs_queue[left];
        r = H.jobs_queue[right];
        if(cmp(l,r)) 
			smallest=left;
        else 
			smallest=pos;
	}
    else{ 
		smallest=pos;
	}
	job sm = H.jobs_queue[smallest];
    if(right <= n && cmp(r,sm)){
		smallest=right;
    }
    
    if(smallest != pos){
        job temp=H.jobs_queue[smallest], temp1=H.jobs_queue[pos];
	    H.jobs_queue[smallest]=temp1, H.jobs_queue[pos]=temp;
        heapify_down(H, smallest);
    }
}

int retrieve(Jobs&  H,job *j){
    if(H.numJobs == 0) 
		return -1;

    *j = H.jobs_queue[1];

    H.jobs_queue[1] = H.jobs_queue[H.numJobs], H.numJobs--;
    heapify_down(H,1);
    return 0;
}

int insertJob(Jobs& H, job& j){
	if(H.numJobs == SIZE) 
		return -1;

    H.numJobs += 1;	
    H.jobs_queue[H.numJobs] = j; 
    heapify_up(H, H.numJobs);

	return 0;
}

int accessMemory(Jobs& H, int ch, job *jp = NULL)
{
	pthread_mutex_lock(&(H.m));
	
	if(ch == 0)
	{
		int x =  insertJob(H, *jp);
		pthread_mutex_unlock(&(H.m));
		return x;
	}

	else if(ch == 1)
	{
		int x = retrieve(H, jp);
		pthread_mutex_unlock(&(H.m));
		return x;

	}
	else if(ch == 2)
	{
		int x = H.job_created += 1;
		pthread_mutex_unlock(&(H.m));
		return x;
	}
	else if(ch == 3)
	{
		int x =  H.job_completed += 1; 
		pthread_mutex_unlock(&(H.m));
		return x;
	}
	else if(ch == 4)
	{
		int x = H.job_completed;  
		pthread_mutex_unlock(&(H.m));
		return x;
	}

	pthread_mutex_unlock(&(H.m));
	return -1;
}

void *consumer(void* ptr){
	while(true){
		srand(time(NULL) ^ (getpid()<<16));
		pthread_t   tid;
		tid = syscall(__NR_gettid);
		while(true)
		{
			sleep(rand() % 4);
			job hp;
			while(accessMemory(H, 1, &hp) == -1)
				usleep(10000);

			cout<<"Consumer: "<<*((int *)ptr)<<", Consumer TID: "<<tid<<", ";
			hp.print();

			accessMemory(H, 3);
			sleep(hp.time);
		}
	}
}

void* producer(void* ptr){
	srand(time(NULL) ^ (getpid()<<16));
	pthread_t   id;
	id = syscall(__NR_gettid);

	while(true)
	{
		job gen(id, *((int *)ptr), (rand() % 10) + 1, (rand() % 4) + 1, (rand() % 100000) + 1);  // Create a job
		sleep(rand() % 4);

		while(accessMemory(H, 0, &gen) == -1)
			usleep(10000);

			accessMemory(H, 2);
			gen.print();
	}
}

int main(){

	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&m, &attr);

    int nc, np;
    cout << "Enter no of Producer and Cosumer Respectively :";
    cin >> nc >> np;

    for(int i=0;i<nc;i++){
    	pthread_t ptid_P;
    	int *param = new int[1];
    	*param = i+1; 
    	pthread_create(&ptid_P, NULL, consumer, param);
    }

    for(int i=0;i<np;i++){
    	pthread_t ptid_C;
    	int *param = new int[1];
    	*param = i+1; 
    	pthread_create(&ptid_C, NULL, producer, param);

    }

    while(true){
    	if(H.job_completed >= PSIZE)return 0;
    }
}