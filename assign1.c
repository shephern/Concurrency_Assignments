#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "mt.h"


pthread_mutex_t buffer_mutex;

struct item{
        long value;
        long sleep_time;
};

struct args{
        long tid;
        struct item *buffer;
        int *buf_size;
};

void *consume(void *passed_arg);
void *produce(void *passed_arg);
unsigned int get_random(int limit);

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("USAGE: assign1 num_producers num_consumers");
                exit(EXIT_FAILURE);
        }

        unsigned long init[4] = {0x142, 0x253, 0x364, 0x475};
        unsigned long length = 4;
        init_by_array(init, length);

        int num_prod = atoi(argv[1]);
        int num_cons = atoi(argv[2]);
        int num_thrd = num_prod + num_cons;
        struct item buffer[32];
        unsigned int buf_size = 0;

        pthread_t threads[num_thrd];
        struct args a[num_thrd];

        if (pthread_mutex_init(&buffer_mutex, NULL) != 0) {
                printf("ERROR: Mutex init failed");
                exit(EXIT_FAILURE);
        }
        
        a->buffer = &buffer[0];
        a->buf_size = &buf_size;

        for (long i = 0; i < num_prod; ++i) {
                a->tid = i;

                pthread_create(&(threads[i]),
                               NULL,
                               produce,
                               (void*)&a[i]);
        }
        
        return 0;
}

void *consume(void *passed_arg)
{
        struct args *a = (struct args*)passed_arg;
        
        printf("Hello from thread %d!", *(a->buf_size));
        return NULL;
        /*
         * TODO: consumer function
         * * Consumes struct by removing it from the buffer
         * * If the buffer is empty wait until producer adds
         * * If accessing buffer, lock out other threads
         * * Print value and sleep given time
        */
}

void *produce(void *passed_arg)
{
        struct args *a = (struct args*)passed_arg;
        while(1 == 1) {
                while(*(a->buf_size) >= 32)
                        sleep(0.01);

                pthread_mutex_lock(&buffer_mutex);
        
                struct item *tmp = malloc(sizeof(struct item));
                tmp->value = get_random(20);
                tmp->sleep_time = get_random(8) + 2;

                a->buffer[*(a->buf_size) - 1] = *tmp;
                (int *)a->buf_size++;

                pthread_mutex_unlock(&buffer_mutex);
                
                sleep(get_random(5) + 3);
        }

        /*
         * TODO: producer function
         * * Produces struct and adds it to the buffer
         * * If buffer is full wait until the consumer removes
         * * If accessing the buffer, lock other threads out
         * * Wait a random time (3-7 sec) before producing again
        */
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

