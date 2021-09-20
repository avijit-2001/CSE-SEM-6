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

struct job {
  pid_t prod_id;
  int prod_no;
  int priority;
  int time;
  int job_id;

  job() {}

  job(int id, int no, int prio, int t, int j_id) {
    prod_id = id;
    prod_no = no;
    priority = prio;
    time = t;
    job_id = j_id;
  }

  void print() {
    printf("Producer: %d, Producer TID: %d, Priority: %d, Compute Time: %d, Job ID: %d\n", 
    	prod_no, prod_id, priority, time, job_id);
  }
};

struct Jobs {
  job jobs_queue[SIZE + 1];
  int in;
  int out;
  int job_created;
  int job_completed;
  int count;
  pthread_mutex_t m;

  Jobs(){
    in = 0;
    out = 0;
    job_completed = 0;
    count = 0;
    job_created = 0;
  }

};
struct Jobs H;

int retrieve(Jobs & H, job * j) {
  if (H.count == 0) {
    return -1;
  }

  * j = H.jobs_queue[in];
  H.out = (H.out + 1)%SIZE;
  H.count = H.count - 1;

  return 0;
}

int insertJob(Jobs & H, job & j) {
  if (H.count == SIZE) {
    return -1;
  }

  H.count = H.count + 1;
  H.jobs_queue[H.in] = j;
  H.in = (H.in + 1)%SIZE;

  return 0;
}

int accessMemory(Jobs & H, int ch, job * jp = NULL) {
  pthread_mutex_lock( & (H.m));
  int x;
  switch (ch) {
  case 0:
    x = insertJob(H, * jp);
    break;
  case 1:
    x = retrieve(H, jp);
    break;
  case 2:
    x = H.job_created += 1;
    break;
  case 3:
    x = H.job_completed += 1;
    break;
  default:
    pthread_mutex_unlock( & (H.m));
    x = -1;
  }
  pthread_mutex_unlock( & (H.m));
  return x;
}

void * consumer(void * ptr) {

  srand(time(NULL) ^ (getpid() << 16));
  pthread_t tid;
  tid = syscall(__NR_gettid);
  while (true) {
    while (H.job_completed >= PSIZE) continue;
    sleep(rand() % 4);
    job hp;
    while (accessMemory(H, 1, & hp) == -1)
      usleep(10000);

    printf("Consumer: %d, Consumer TID: %d,", *((int * ) ptr), (int) tid);
    hp.print();

    accessMemory(H, 3);
    sleep(hp.time);
  }
}

void * producer(void * ptr) {

  srand(time(NULL) ^ (getpid() << 16));
  pthread_t id;
  id = syscall(__NR_gettid);
  while (true) {
    while (H.job_completed >= PSIZE) continue;
    job gen(id, *((int * ) ptr), (rand() % 10) + 1, (rand() % 4) + 1, (rand() % 100000) + 1); // Create a job
    sleep(rand() % 4);

    while (accessMemory(H, 0, & gen) == -1)
      usleep(10000);

    accessMemory(H, 2);
    gen.print();
  }
}

int main() {

  clock_t st, en;
  st = clock();
  double time = 0;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init( & attr);
  pthread_mutexattr_setpshared( & attr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init( & H.m, & attr);

  int nc, np;
  cout << "Enter no of Producer and Cosumer Respectively :";
  cin >> nc >> np;

  for (int i = 0; i < nc; i++) {
    pthread_t ptid_P;
    int * param = new int[1];
    * param = i + 1;
    pthread_create( & ptid_P, NULL, consumer, param);
  }

  for (int i = 0; i < np; i++) {
    pthread_t ptid_C;
    int * param = new int[1];
    * param = i + 1;
    pthread_create( & ptid_C, NULL, producer, param);

  }

  while (true) {
    usleep(10000);
    time += 0.01;
    if (H.job_completed >= PSIZE) {
      printf("Time Executed: %.3f s\n", time + (((double)(clock() - st)) / CLOCKS_PER_SEC));
      kill(-getpid(), SIGQUIT);
    }
  }
}