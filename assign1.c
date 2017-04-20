#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct args{
        long tid;
        long value;
        long sleep_time;
};

void *consume(void *tid)
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

void *produce(void *tid)
{
        /*
         * Function to be created::
         * * Produces struct and adds it to the buffer
         * * If buffer is full wait until the consumer removes
         * * If accessing the buffer, lock other threads out
         * * Wait a random time (3-7 sec) before producing again
        */
}

int main(int argc, char **argv)
{
        unsigned int eax;
        unsigned int ebx;
        unsigned int ecx;
        unsigned int edx;

        eax = 0x01;

        __asm__ __volatile__(
                             "cpuid;"
                             : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                             : "a"(eax)
                             );

        if(ecx & 0x40000000){
                //Use rdrand
                __asm__ __volatile__(
                                     "rdrand %eax;"
                                     : "=a"(eax)
                                     : "a"(eax)
                                     );
                printf("%d", eax);
        } else {
                //Use Mersenne Twister
        }
        return 0;

}



