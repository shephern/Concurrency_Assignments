#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

pthread_mutex_t buffer_mutex;
pthread_mutex_t print_mutex;

struct node{
        struct node *next;
        int value;
};

struct passed_args{
        long pid;
};

int size = 0;
struct node *head;

sem_t deleter_mutex;
sem_t searcher_mutex;
sem_t inserter_mutex;
sem_t mutex;
long int searchers = 0;

void *searcher(void *passed_arg);
void *deleter(void *passed_arg);
void *inserter(void *passed_arg);

void print_list();

int main(int argc, char **argv)
{
        if (argc != 4) {
                printf(
                        "USAGE: concurrency3 searchers deleters inserters\n"
                );
                exit(EXIT_FAILURE);
        }


        int num_search = atoi(argv[1]);
        int num_delete = atoi(argv[2]);
        int num_insert = atoi(argv[3]);
        int num_thread = num_search + num_delete + num_insert;
        
        long index = 0;
        pthread_t threads[num_thread];

        struct passed_args a[num_thread];
        srand(time(NULL));
        head = (struct node *)malloc( sizeof(struct node) );
        head->next = NULL;
        head->value = -1;

        if (sem_init(&deleter_mutex, 0, 1) != 0) {
                printf("ERROR: Deleter init failed\n");
                exit(EXIT_FAILURE);
        }
        if (sem_init(&searcher_mutex, 0, num_search) != 0) {
                printf("ERROR: Searcher init failed\n");
                exit(EXIT_FAILURE);
        }
        if (sem_init(&inserter_mutex, 0, 1) != 0) {
                printf("ERROR: Inserter init failed\n");
                exit(EXIT_FAILURE);
        }
        if (sem_init(&mutex, 0, 1) != 0) {
                printf("ERROR: Mutex init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&print_mutex, NULL) != 0) {
                printf("ERROR: Print Mutex Init failed\n");
                exit(EXIT_FAILURE);
        }
        
        //Create inserter threads
        for (long i = 0; i < num_insert; ++i) {
                a[index].pid = i;
                pthread_create(&(threads[index]),
                                NULL,
                                inserter,
                                (void *)&a[index]);
                index++;
        }

        //Create searcher threads
        for (long i = 0; i < num_search; ++i) {
                a[index].pid = i;
                pthread_create(&(threads[index]),
                                NULL,
                                searcher,
                                (void *)&a[index]);
                index++;
        }
        //Create deleter threads
        for (long i = 0; i < num_delete; ++i) {
                a[index].pid = i;
                pthread_create(&(threads[index]),
                                NULL,
                                deleter,
                                (void *)&a[index]);
                index++;
        }
        
        for (long i = 0; i < num_thread; ++i) {
                pthread_join(threads[i], NULL);
        }

        return 0;
}

void *inserter(void *passed_arg)
{
        int val;
        long sleep_time = 1;
        struct node *tmp = (struct node *) malloc( 
                        sizeof(struct node) );
        struct node *in = (struct node *) malloc(
                        sizeof(struct node) );

        struct passed_args *a = (struct passed_args*)passed_arg;
        while (1 == 1) {
                sem_wait(&inserter_mutex);
                sem_wait(&mutex);
                
                val = rand()%200;
                
                pthread_mutex_lock(&print_mutex);
                printf("Inserter %d inserted %d value item\n",  
                        a->pid, val);
                pthread_mutex_unlock(&print_mutex);

                tmp = (struct node *) head;
                in->next = NULL;
                in->value = val;

                while(tmp->next != NULL){
                        tmp = tmp->next;
                }
                tmp->next = in;
                size++;
                print_list();

                sem_post(&mutex);
                sem_post(&inserter_mutex);
                
                sleep(sleep_time);
        }    
        free(tmp);        
        return NULL;
}

void *searcher(void *passed_arg)
{
        int val;
        long sleep_time = 3;
        struct node *tmp = (struct node *) malloc( 
                        sizeof(struct node) );

        struct passed_args *a = (struct passed_args*)passed_arg;
        int found;
        while (1 == 1) {
                sem_wait(&mutex);
                searchers += 1;
                if (searchers == 1) {
                        sem_wait(&searcher_mutex);
                }
                sem_post(&mutex);

                found = 0;
                val = rand()%200;
                
                tmp = (struct node *) head;

                while (tmp->next != NULL){
                        if (tmp->value == val)
                                found = 1;
                }

                pthread_mutex_lock(&print_mutex);
                if (found == 1) {
                        printf("Searcher %d found %d value item\n",
                                a->pid, val);
                } else {
                        printf("Searcher %d did not find %d value item\n",  
                                a->pid, val);
                }
                pthread_mutex_unlock(&print_mutex);

                sem_wait(&mutex);
                searchers -= 1;
                if (searchers == 0) {
                        sem_post(&searcher_mutex);
                }
                sem_post(&mutex);

                sleep(sleep_time);
        }    
        free(tmp);        
        return NULL;
}

void *deleter(void *passed_arg)
{
        int val;
        long sleep_time = 1;

        struct passed_args *a = (struct passed_args*)passed_arg;
        while (1 == 1) {
                sem_wait(&searcher_mutex);
                sem_wait(&inserter_mutex);

                struct node *tmp;
                struct node *prev;
                int index = 0;

                int del_index = (size == 0)? 0 : rand()%size;
                
                pthread_mutex_lock(&print_mutex);
                printf("Deleter %d deleting %d index item\n",  
                        a->pid, val);
                pthread_mutex_unlock(&print_mutex);

                tmp = (struct node *) head;
                prev = tmp;
                while(tmp->next != NULL){
                        if (index == del_index) {
                                if (del_index == 0) {
                                        head = tmp->next;
                                        free(tmp);
                                        size--;
                                } else {
                                        prev->next = tmp->next;
                                        free(tmp);
                                        size--;
                                }
                        } else {
                                prev = tmp;
                                tmp = tmp->next;
                        }
                }

                sem_post(&inserter_mutex);
                sem_post(&searcher_mutex);
                
                sleep(sleep_time);
        }    
        return NULL;
}

void print_list()
{
        struct node *tmp = (struct node *) head;
        pthread_mutex_lock(&print_mutex);
        if (size == 0) {
                printf("List is empty, size: 0");
        } else {
                printf("List is [");
                while (tmp->next != NULL) {
                        printf("%d, ", tmp->value);
                }
                printf("%d] size: %d\n", tmp->value, size);
        }
        pthread_mutex_unlock(&print_mutex);
}
