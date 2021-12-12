#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

const int INT_MAX=2147483647 / 10000000;
const int THREAD_COUNT = 2;

int fib(int i);
void * waste_time(void * arg);

// This is intended to be a bad algorithm
// Just make lots of noice
int main() {
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_attr_t *attr = (pthread_attr_t *) malloc(sizeof(pthread_attr_t));
    pthread_attr_init(attr);
    pthread_t *t = (pthread_t *) malloc(sizeof(pthread_t));
    void *arg = malloc(sizeof(int));
    int * id = (int*) arg;
    *id = i;
    pthread_create(t, attr, &waste_time, arg);
  }
  void *arg = malloc(sizeof(int));
  int * id = (int*) arg;
  *id = -1;
  waste_time(arg);
}

void * waste_time(void * arg) {
  int *id = (int *) arg;
  
	for (int i = 1; i < INT_MAX; i++) {
    int a = fib(i);
		//printf("ID %d Fib %d = %d\n", *id, i, a);
	}


}

int fib(int i) {
	if (i <= 0) {
		return 0;
	}
	if (i <= 2) {
		return 1;
	}
	if (i > 2) {
		return fib(i - 2) + fib(i - 1);
	}
	return 0;
}
