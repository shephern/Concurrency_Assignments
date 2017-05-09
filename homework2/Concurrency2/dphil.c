#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



pthread_t philosopher[5];
pthread_mutex_t forks[5];

int status_forks[5] = {1,1,1,1,1};


void getForks(int n){
   pthread_mutex_lock(&forks[n]);
   pthread_mutex_lock(&forks[(n+1)%5]);
   status_forks[n] = 0;
}

void putForks(int n){
   pthread_mutex_unlock(&forks[n]);
   pthread_mutex_unlock(&forks[(n+1)%5]);
   status_forks[n] = 1;
}

void think(int n){
   printf ("Philosopher %d is thinking\n",n);
   int rNum = rand()%20 + 1;
   sleep(rNum);
}  

void eat(int n){
   printf ("Philosopher %d is eating\n",n);
   printForkStatus();
   int rNum = rand()%9 + 2;
   sleep(rNum);
}  

void printForkStatus(){
   int i;
   printf("Forks:");
   for(i=0;i<5;i++){
      printf(" %d ", status_forks[i] );
   }
   printf("\n");

}

void *din(int n)
   {   
      while(1){
      think(n);
      getForks(n);
      eat(n);
      putForks(n);
   }
}

void start_din(){
   int i;

   for(i=0;i<5;i++){
      pthread_mutex_init(&forks[i],NULL);
   }
   
   for(i=0;i<5;i++){
      pthread_create(&philosopher[i],NULL,(void *)din,(void *)i);
   }
    
   for(i=0;i<5;i++){
      pthread_join(philosopher[i],NULL);
   }
}


int main()
{

  start_din();
  return 0;

}