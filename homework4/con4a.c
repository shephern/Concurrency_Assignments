#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>


struct passed_args{
        long pid;
};


pthread_mutex_t count_mutex;
pthread_mutex_t mutex;
pthread_mutex_t print_mutex;
long int accessers = 0;

void *accesser(void *passed_arg);

int main(int argc, char **argv)
{
        if (argc != 2) {
                printf(
                        "USAGE: p1 accessers\n"
                );
                exit(EXIT_FAILURE);
        }


        int num_thread = atoi(argv[1]);
        
        pthread_t threads[num_thread];

        struct passed_args a[num_thread];

        if (pthread_mutex_init(&count_mutex, NULL) != 0) {
                printf("ERROR: Count init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&mutex, NULL) != 0) {
                printf("ERROR: Mutex init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&print_mutex, NULL) != 0) {
                printf("ERROR: Print Mutex Init failed\n");
                exit(EXIT_FAILURE);
        }
        
        //Create threads
        for (long i = 0; i < num_thread; ++i) {
                a[i].pid = i;
                pthread_create(&(threads[i]),
                                NULL,
                                accesser,
                                (void *)&a[i]);
        }

        while(1 == 1) {
                sleep(10);
        } 
        for (long i = 0; i < num_thread; ++i) {
                pthread_join(threads[i], NULL);
        }

        return 0;
}

void *accesser(void *passed_arg)
{
        struct passed_args *a = (struct passed_args*) passed_arg;
        while (1 == 1) {
                pthread_mutex_lock(&count_mutex);
                accessers += 1;
                if (accessers > 3) {
                        pthread_mutex_lock(&mutex);
                        pthread_mutex_lock(&print_mutex);
                        printf("Accesser %ld waiting\n",
                                a->pid);
                        pthread_mutex_unlock(&print_mutex);
                }
                pthread_mutex_unlock(&count_mutex);


                pthread_mutex_lock(&print_mutex);
                printf("Accesser %ld accessing resource\n",
                        a->pid);
                pthread_mutex_unlock(&print_mutex);
                sleep(1);

                pthread_mutex_lock(&count_mutex);
                accessers -= 1;
                if (accessers <= 3) {
                        pthread_mutex_unlock(&mutex);
                }
                pthread_mutex_unlock(&count_mutex);

        }    
        return NULL;
}

