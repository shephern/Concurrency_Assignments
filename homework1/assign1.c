#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "mt.h"

//One locks the buffer while it is being edited
//the other locks printf, so that things get printed properly
pthread_mutex_t buffer_mutex;
pthread_mutex_t print_mutex;

struct item{
        long value;
        long sleep_time;
};

struct args{
        long tid;
};

//Spec change due to time constraints
int buffer_size=0;
struct item buffer[32];

void *consume(void *passed_arg);
void *produce(void *passed_arg);
unsigned int get_random(int limit);

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("USAGE: assign1 num_producers num_consumers\n");
                exit(EXIT_FAILURE);
        }

        unsigned long init[4] = {0x142, 0x253, 0x364, 0x475};
        unsigned long length = 4;
        init_by_array(init, length);

        int num_prod = atoi(argv[1]);
        int num_cons = atoi(argv[2]);
        int num_thrd = num_prod + num_cons;
        int print_counter = 0;
        long index = 0;
        pthread_t threads[num_thrd];
        struct args a[num_thrd];

        if (pthread_mutex_init(&buffer_mutex, NULL) != 0) {
                printf("ERROR: Mutex init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&print_mutex, NULL) != 0) {
                printf("ERROR: Print Mutex Init failed\n");
                exit(EXIT_FAILURE);
        }
        
        //Create producer threads
        for (long i = 0; i < num_prod; ++i) {
                a[index].tid = index;

                pthread_create(&(threads[index]),
                               NULL,
                               produce,
                               (void*)&a[index]);
                index++;
        }

        //Create consumer threads
        for (long i = 0; i < num_cons; ++i) {
                a[index].tid = i;

                pthread_create(&(threads[index]),
                               NULL,
                               consume,
                               (void*)&a[index]);
                index++;
        }

        //Added a sleep because for whatever reason join quits before anything
        //can get done
        for (long i = 0; i < num_thrd; ++i) {
                pthread_join(threads[i], NULL);
        }
        return 0;
}

void *consume(void *passed_arg)
{
        long val;
        long sleep_time;

        struct args *a = (struct args*)passed_arg;
        while (1 == 1) {
                while(buffer_size <= 0)
                        sleep(0.01);

                pthread_mutex_lock(&buffer_mutex);

                val = buffer[buffer_size - 1].value;
                sleep_time = buffer[buffer_size - 1].sleep_time;
                buffer_size--;  //Not actually deleting, but decrementing
                
                pthread_mutex_unlock(&buffer_mutex);
                
                pthread_mutex_lock(&print_mutex);
                printf("Consumer %d consumed %ld value item sleeping for %ld\n",                        a->tid, val, sleep_time);
                pthread_mutex_unlock(&print_mutex);
                sleep(sleep_time);
        }                
        return NULL;
}

void *produce(void *passed_arg)
{
        struct args *a = (struct args*)passed_arg;
        while(1 == 1) {
                while(buffer_size >= 32)
                        sleep(0.01);

                pthread_mutex_lock(&buffer_mutex);
        
                buffer[buffer_size].value = get_random(20);
                buffer[buffer_size].sleep_time = get_random(8) + 2;
                int wait_time = get_random(5) + 3;
                buffer_size++;
                pthread_mutex_unlock(&buffer_mutex);

                pthread_mutex_lock(&print_mutex);
                printf("Producer %d created %ld value item sleeping for %ld\n",                        a->tid, buffer[buffer_size-1], wait_time);
                pthread_mutex_unlock(&print_mutex);
                sleep(wait_time);

        }
        return NULL;
}

unsigned int get_random(int limit)
{
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;
        unsigned int random;

        eax = 0x01;

        __asm__ __volatile__(
                             "cpuid;"
                             : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                             : "a"(eax)
                             );
        if (ecx & 0x40000000) {
                //Use rdrand
                __asm__ __volatile__(
                                     "rdrand %0;"
                                     : "=a"(eax)
                                     : "a"(eax)
                                    );
                random = eax % limit;
        } else {
                //Use mersenne twister
                random = genrand_int32() % limit;
        }
        return random;
}

