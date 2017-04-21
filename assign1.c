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
        struct item (*buffer);
        int *buf_size;
};

void *consume(void *tid, void *buffer)
{
        struct args *a = (struct args*)tid;
        sleep(a->sleep_time);
        return (void*)printf("Hello from thread %ld!", a->tid);
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

        while((int *)a->buf_size >= 32)
                sleep(0.01);
        

        pthread_mutex_lock(&buffer_mutex);

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
                                     "rdrand %eax;"
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

int main(int argc, char **argv)
{
        if (argc != 3) {
                printf("USAGE: assign1 num_producers num_consumers");
                exit(EXIT_FAILURE);
        }

        unsigned long init[4] = {0x142, 0x253, 0x364, 0x475}
        unsigned long length = 4;
        init_by_array(init, length);

        int num_prod = atoi(argv[1]);
        int num_cons = atoi(argv[2]);
        int num_thrd = num_producer + num_consumer;
        struct item buffer[32];
        unsigned int buf_size = 0;

        pthread_t threads[num_thrd];
        struct args a[num_thrd];

        if (pthread_mutex_init(&buffer_lock, NULL) != 0) {
                printf("ERROR: Mutex init failed");
                exit(EXIT_FAILURE);
        }
        
        a->buffer = &buffer;
        a->buf_size = &buf_size;

        for (long i = 0; i < num_prod; ++i) {
                a->tid = i;

                pthread_create(&(threads[i]),
                               NULL,
                               produce,
                               (void*)&a[i]);
        }
}



