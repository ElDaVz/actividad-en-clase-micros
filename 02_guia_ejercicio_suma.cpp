#include <iostream>
#include <pthread.h>
#include <vector>
#include <cstdlib>
using namespace std;

struct ThreadData {
    vector<int>* arr;
    int inicio;
    int fin;
    int id;
    int suma_parcial;
    int pairNumbers;
};

void* sumaSegmento(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    data->suma_parcial = 0;
    data->pairNumbers = 0;

    for(int i = data->inicio; i < data->fin; i++) {
        data->suma_parcial += (*(data->arr))[i];

        if ((*(data->arr))[i] % 2 == 0) {
            data->pairNumbers++;
        }

    }

    cout << "Hilo " << data->id << " suma parcial: " << data->suma_parcial << endl;
    cout << "Hilo " << data->id << " números pares: " << data->pairNumbers << endl;
    return NULL;
}

pair<int, int> askInput(){
    int n, num_hilos;

    while (true) {
        cout << "Ingrese el tamaño del arreglo: ";
        cin >> n;
        cout << "Ingrese el número de hilos: ";
        cin >> num_hilos;

        if (n >= num_hilos) {
            return {n, num_hilos};
        }

        cout << "Error: los hilos no pueden ser más que el tamaño del arreglo.\n";
    }
}

int main() {
    auto input = askInput();
    int n = input.first;
    int num_hilos = input.second;

    vector<int> arr(n);

    for(int i = 0; i < n; i++)
        arr[i] = rand() % 10;

    pthread_t hilos[num_hilos];
    ThreadData datos[num_hilos];

    int segmento = n / num_hilos;

    for(int i = 0; i < num_hilos; i++) {
        datos[i].arr = &arr;
        datos[i].inicio = i * segmento;
        datos[i].fin = (i == num_hilos - 1) ? n : (i + 1) * segmento;
        datos[i].id = i;

        pthread_create(&hilos[i], NULL, sumaSegmento, &datos[i]);
    }

    int suma_total = 0;
    int totalPairNumbers = 0;

    for(int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
        suma_total += datos[i].suma_parcial;
        totalPairNumbers += datos[i].pairNumbers;
    }

    cout << "Suma total: " << suma_total << endl;
    cout << "Total de números pares: " << totalPairNumbers << endl;

    return 0;
}