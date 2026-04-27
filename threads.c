#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

void *calculos(void* argument){

    intptr_t input = (intptr_t) argument;
    intptr_t output = input * 2;

    return (void*) output;
}

int main(){
    printf("\n--- Iniciando Cálculos ---\n");
    
    pthread_t thread_id;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    void *exit_value;

    for(int i = 0; i < 100; i++){
        pthread_create(&thread_id, &attr, calculos, (void*)(intptr_t)i);
        
        pthread_join(thread_id, &exit_value);

        intptr_t result_val = (intptr_t) exit_value;
        printf("Resultado multiplicación es: %ld\n", (long)result_val);
    }

    pthread_attr_destroy(&attr);
    return 0;
}