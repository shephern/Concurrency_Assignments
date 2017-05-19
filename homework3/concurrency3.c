#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


struct node{
        struct node *next;
        int value;
};

struct passed_args{
        long pid;
};

int size = 0;
struct node *head;

pthread_mutex_t searcher_count_mutex;
pthread_mutex_t searcher_mutex;
pthread_mutex_t inserter_mutex;
pthread_mutex_t print_mutex;
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

        if (pthread_mutex_init(&searcher_count_mutex, NULL) != 0) {
                printf("ERROR: Searcher init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&searcher_mutex, NULL) != 0) {
                printf("ERROR: Searcher init failed\n");
                exit(EXIT_FAILURE);
        }
        if (pthread_mutex_init(&inserter_mutex, NULL) != 0) {
                printf("ERROR: Inserter init failed\n");
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
        while(1 == 1) {
                index = 1 - 1 + index;
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

        struct passed_args *a = (struct passed_args*)passed_arg;
        while (1 == 1) {
                pthread_mutex_lock(&print_mutex);
                printf("Inserter is blocked\n");
                pthread_mutex_unlock(&print_mutex);

                pthread_mutex_lock(&inserter_mutex);
                
                val = rand()%200;
                
                pthread_mutex_lock(&print_mutex);
                printf("Inserter %ld inserted %d value item\n",  
                        a->pid, val);
                pthread_mutex_unlock(&print_mutex);
                
                struct node *in = (struct node *) malloc(
                        sizeof(struct node) );

                struct node *tmp = (struct node *) head;
                
                in->next = NULL;
                in->value = val;
                if (head->next == NULL) {
                        head->next = in;     
                } else {
                        while (tmp->next != NULL)
                                tmp = tmp->next;
                        tmp->next = in;
                }
                size++;
                print_list();

                pthread_mutex_unlock(&inserter_mutex);
                
                sleep(sleep_time);
        }    
        return NULL;
}

void *searcher(void *passed_arg)
{
        int val;
        long sleep_time = 1;
        struct node *tmp = (struct node *) malloc( 
                        sizeof(struct node) );

        struct passed_args *a = (struct passed_args*)passed_arg;
        int found;
        while (1 == 1) {
                pthread_mutex_lock(&print_mutex);
                printf("Searcher is blocked\n");
                pthread_mutex_unlock(&print_mutex);
                pthread_mutex_lock(&searcher_count_mutex);
                searchers += 1;
                if (searchers == 1) {
                        pthread_mutex_lock(&searcher_mutex);
                }
                pthread_mutex_unlock(&searcher_count_mutex);

                found = 0;
                val = rand()%200;
                
                tmp = (struct node *) head->next;

                while (tmp != NULL){
                        if ((val - 5 <= tmp->value)&&(tmp->value <= val+5))
                                found = 1;
                }

                pthread_mutex_lock(&print_mutex);
                if (found == 1) {
                        printf("Searcher %ld found item (%d < value < %d)\n"
                                ,a->pid, val-6, val+6);
                } else {
                        printf("Searcher %ld did not find %d value item\n",
                                a->pid, val);
                }
                pthread_mutex_unlock(&print_mutex);

                pthread_mutex_lock(&searcher_count_mutex);
                searchers -= 1;
                if (searchers == 0) {
                        pthread_mutex_unlock(&searcher_mutex);
                }
                pthread_mutex_unlock(&searcher_count_mutex);

                sleep(sleep_time);
        }    
        free(tmp);        
        return NULL;
}

void *deleter(void *passed_arg)
{
        long sleep_time = 2;

        struct passed_args *a = (struct passed_args*)passed_arg;
        while (1 == 1) {
                pthread_mutex_lock(&print_mutex);
                printf("Deleter is blocked\n");
                pthread_mutex_unlock(&print_mutex);

                pthread_mutex_unlock(&searcher_mutex);
                pthread_mutex_unlock(&inserter_mutex);
                struct node *tmp;
                struct node *prev;
                int index = 0;

                int del_index = (size == 0) ? 0 : rand()%size;
                
                pthread_mutex_lock(&print_mutex);
                if (size == 0) {
                        printf("Deleter not deleting, list empty\n");
                } else {
                        printf("Deleter %ld deleting %d index item\n",  
                                a->pid, del_index);
                }
                pthread_mutex_unlock(&print_mutex);

                tmp = (struct node *) head->next;
                prev = tmp;
                while(tmp != NULL){
                        if (index == del_index) {
                                if (del_index == 0) {
                                        head->next = tmp->next;
                                        if (tmp != NULL){
                                                tmp = NULL;
                                                //free(tmp);
                                                size--;
                                        }
                                } else {
                                        prev->next = tmp->next;
                                        if (tmp != NULL){
                                                //free(tmp);
                                                tmp = NULL;
                                                size--;
                                        }
                                }
                        } else {
                                index++;
                                prev = tmp;
                                tmp = tmp->next;
                        }
                }
                print_list();

                pthread_mutex_unlock(&inserter_mutex);
                pthread_mutex_unlock(&searcher_mutex);
                
                sleep(sleep_time);
        }    
        return NULL;
}

void print_list()
{
        struct node *tmp = (struct node *) head->next;
        pthread_mutex_lock(&print_mutex);
        printf("List is: \n");
        if (tmp == NULL) {
                printf("[]\n");
        } else {
                printf("[");
                for (int i = 0; i < size - 1; ++i) {
                        printf("%d, ", tmp->value);
                        tmp = tmp->next;
                }
                printf("%d] size: %d\n", tmp->value, size);
        }
        printf("------------------------\n");
        pthread_mutex_unlock(&print_mutex);
}
