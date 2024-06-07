#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <unistd.h> 
#include <conio.h> 
#include <stdbool.h> 
#include <semaphore.h> 
#include <stdint.h>                                                                 


void* CreateCar(void* arg);
void ShowTheBridge();
bool CheckDirectionPriority(char direction);
bool IsTheBridgeFull();
bool IsDirectionOK(char direction);
void* CrossTheBridge(void* arg);

 
pthread_t* north_cars = NULL; //These pointers will point the arrays of cars(threads) that will cross the bridge.
pthread_t* south_cars = NULL; 
int n_of_crossed_cars_from_north = 0; //This variable will store the number of cars(threads) that crossed the bridge from the north.
int n_of_crossed_cars_from_south = 0; //from the south.
int cars_on_the_bridge = 0; 
char direction_of_the_cars = ' '; //direction of the cars(threads) that crossing the bridge.

//This variables will store the number of cars(threads) waiting to cross the bridge.
int n_north_cars = 0; 
int n_south_cars = 0; 


sem_t semaphore; //This semaphore will be used to allow the cars to cross the bridge.
sem_t semaphore2; //For ShowTheBridge function.
sem_t semaphore3; //For decreasing the number of cars on the bridge.


int main() {

    pthread_t thread;
    sem_init(&semaphore, 0, 1); //Initialize the semaphore.
    sem_init(&semaphore2, 0, 1); 
    sem_init(&semaphore3, 0, 1); 
    pthread_create(&thread, NULL, CreateCar, NULL); //This thread will create other cars(threads).
    pthread_join(thread, NULL); //Wait for the thread to finish.

    //Free the memory allocated for the thread arrays.
    free(north_cars); 
    free(south_cars); 

    sem_destroy(&semaphore); //Destroy the semaphore.
    sem_destroy(&semaphore2);
    sem_destroy(&semaphore3);

    return 0;
}


void* CreateCar(void* arg) {

    printf("\n");

    while (1) {
        
        if (!kbhit()) { //If a key is not pressed.
            
            int random_num = rand() % 100; 

            if (random_num % 2 == 0) { 
            
                n_north_cars++;
                printf("A car from the North is created.\n");
                north_cars = (pthread_t*)realloc(north_cars, n_north_cars * sizeof(pthread_t)); //Reallocate the memory for the array of cars.
                pthread_create(&north_cars[n_north_cars - 1], NULL, CrossTheBridge, (void*)'N'); //Create a car(thread) from the North.

            } else {
            
                n_south_cars++;
                printf("A car from the South is created.\n");
                south_cars = (pthread_t*)realloc(south_cars, n_south_cars * sizeof(pthread_t)); //Reallocate the memory for the array of cars.
                pthread_create(&south_cars[n_south_cars - 1], NULL, CrossTheBridge, (void*)'S'); //Create a car(thread) from the South.
            }

            sleep(1); //Wait for 1 second.
            
        } else {
            break;
        }
    }
    return NULL;
}



void ShowTheBridge(){

    sem_wait(&semaphore2); //Wait for the semaphore to be available.

    printf("    -->                           <--\n");
    printf("   North         BRIDGE          South\n");
    printf("  =======   =================   =======\n");

    if(direction_of_the_cars == 'N'){
        printf("     %d            %d -->            %d\n\n", n_north_cars, cars_on_the_bridge, n_south_cars);
        printf("Press a key to stop the program.\n\n");
        return;

    } else if(direction_of_the_cars == 'S'){
        printf("     %d            %d <--            %d\n\n", n_north_cars, cars_on_the_bridge, n_south_cars);
        printf("Press a key to stop the program.\n\n");
        return;

    } else {
        printf("     %d              %d              %d\n\n", n_north_cars, cars_on_the_bridge, n_south_cars);
        printf("Press a key to stop the program.\n\n");
        return;
    }   
}


bool CheckDirectionPriority(char direction){

    if(direction == 'N' && n_of_crossed_cars_from_north < 10){  //Max 10 cars can cross the bridge one after the other from the same direction.
        return true;
    
    } else if(direction == 'S' && n_of_crossed_cars_from_south < 10){
        return true;
    
    } else {
        return false;
    } 
}


bool IsDirectionOK(char direction){

    if(direction == 'N' && (direction_of_the_cars == 'N' || direction_of_the_cars == ' ')){
        return true;
    
    } else if(direction == 'S' && (direction_of_the_cars == 'S' || direction_of_the_cars == ' ')){
        return true;
    
    } else {
        return false;
    }
}


bool IsTheBridgeFull(){

    if(cars_on_the_bridge < 3){ //Max 3 cars can cross the bridge at the same time.
        return false;
    
    } else {
        return true;
    }
}


void* CrossTheBridge(void* arg) {

    ShowTheBridge();
    sem_post(&semaphore2); //Release the semaphore.

    char direction = (char)(uintptr_t)arg; // Cast pointer to uintptr_t and then to char

    sem_wait(&semaphore); //Wait for the semaphore to be available.

    while (1) {
        
        if(CheckDirectionPriority(direction)){

            if(IsDirectionOK(direction)){

                if(!IsTheBridgeFull()){

                    direction_of_the_cars = direction;
                    cars_on_the_bridge++;

                    if(direction == 'N'){
                        n_north_cars--;
                        n_of_crossed_cars_from_north++;
                        n_of_crossed_cars_from_south = 0;
                    } else {
                        n_south_cars--;
                        n_of_crossed_cars_from_south++;
                        n_of_crossed_cars_from_north = 0;
                    }

                    ShowTheBridge();
                    sem_post(&semaphore2); 
                    
                    sem_post(&semaphore); 
                    
                    sleep(2); //A car is crossing the bridge.
                
                    sem_wait(&semaphore3); 
                    cars_on_the_bridge--;
                    ShowTheBridge();
                    sem_post(&semaphore2); 
                    sem_post(&semaphore3); 

                    if(cars_on_the_bridge == 0){
                        direction_of_the_cars = ' ';
                    }
                      
                    pthread_exit(NULL); //Exit the thread.
                    break;       
                }
            }
        }
    }

    return NULL;
}