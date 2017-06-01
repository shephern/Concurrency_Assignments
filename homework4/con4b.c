#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>


//sems
sem_t sem_cust;
sem_t sem_mutex;
sem_t sem_barber;
sem_t sem_custDone;
sem_t sem_barbDone;



void *customer(void *num);
void *barber(void *);
void *cutHair();
void *getHairCut();
void balk();

int chairs = 5;
int currCustomers = 0;
int custs = 25;

int hairCutTime = 0;


int main( int argc, char *argv[] )  {
	

   pthread_t bid;
   if (argc == 3) {
        chairs = atoi(argv[1]);
        custs = atoi(argv[2]);
   }
   int custsArr[custs];
   pthread_t cid[custs];
   int i;

 	for (i=0; i<custs; i++) {
    	custsArr[i] = i;
    }

	sem_init(&sem_cust, 0, 0);
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_barber, 0, 0);
    sem_init(&sem_custDone, 0, 0);
	sem_init(&sem_barbDone, 0, 0);


    pthread_create(&bid, NULL, barber, NULL);

    for (i=0; i<custs; i++) {
    	pthread_create(&cid[i], NULL, customer, (void *)&custsArr[i]);
		int rNum = rand()%5 + 1;

    	sleep(rNum);
    }

    for (i=0; i<custs; i++) {
    	pthread_join(cid[i],NULL);
    }

}


void *customer(void *b){

	int custNum = *(int *)b;

	sem_wait(&sem_mutex);
		if(currCustomers == chairs){
			printf("customer %d turned away, chairs full\n", custNum);
			sem_post(&sem_mutex);
			balk();
		}else{
			currCustomers++;
		}


		printf("customer %d arrived. Chairs Avail: %d\n", custNum, (5-currCustomers) );
		sem_post(&sem_mutex);

		sem_post(&sem_cust);
		sem_wait(&sem_barber);
			printf("customer %d getting haircut\n", custNum);
			hairCutTime = rand()%10 + 1;
			cutHair();	
		sem_post(&sem_custDone);
		sem_wait(&sem_barbDone);

		sem_wait(&sem_mutex);
			currCustomers--;
		sem_post(&sem_mutex);

		if(custNum == custs){
			printf("Barbershop closing\n");
			exit(0);
		}

}

void balk(){
	while(1){

	}
}


void *barber(void *a){

	while(1){
		sem_wait(&sem_cust);
		sem_post(&sem_barber);
		printf("barber cutting hair\n");
		getHairCut();
		printf("barber finished cutting hair\n");
		sem_wait(&sem_custDone);
		sem_post(&sem_barbDone);
	}
}

void *cutHair(){
	sleep(hairCutTime);
}

void *getHairCut(){
	sleep(hairCutTime);
}
