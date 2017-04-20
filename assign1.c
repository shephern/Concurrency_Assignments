#include <stdio.h>
#include <string.h>

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



