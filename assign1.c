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
        if (pthread_mutex_init(&print_mutex, NULL) != 0) {
                printf("ERROR: Print Mutex Init failed");
                exit(EXIT_FAILURE);
        }
        
        for (long i = 0; i < num_prod; ++i) {
                a[i].tid = i;
                a[i].buf_size = &buf_size;
                a[i].buffer = &buffer[0];

                pthread_create(&(threads[i]),
                               NULL,
                               produce,
                               (void*)&a[i]);
        }

        for (long i = num_prod-1; i < num_thrd; ++i) {
                a[i].tid = i;
                a[i].buf_size = &buf_size;
                a[i].buffer = &buffer[0];

                pthread_create(&(threads[i]),
                               NULL,
                               consume,
                               (void*)&a[i]);
        }
        
        //Added a sleep because for whatever reason join quits before anything
        //can get done
        sleep(30);
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
                while(*(a->buf_size) == 0)
                        sleep(0.01);

                pthread_mutex_lock(&buffer_mutex);
                struct item *tmp = &a->buffer[*(a->buf_size) - 1];
                *(a->buf_size)--;
                pthread_mutex_unlock(&buffer_mutex);

                val = tmp->value;
                sleep_time = tmp->sleep_time;
                pthread_mutex_lock(&print_mutex);
                printf("Consumer %d consumed %ld value item sleeping for %ld\n",                        a->tid, val, sleep_time);
                pthread_mutex_unlock(&print_mutex);
                sleep(sleep_time);
        }
                
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
                int wait_time = get_random(5) + 3;

                a->buffer[*(a->buf_size) - 1] = *tmp;
                *(a->buf_size)++;

                pthread_mutex_lock(&print_mutex);
                printf("Producer %d created %ld value item sleeping for %ld\n",                        a->tid, tmp->value, wait_time);
                pthread_mutex_unlock(&print_mutex);
                pthread_mutex_unlock(&buffer_mutex);
                sleep(tmp->sleep_time);

                free(tmp);
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

