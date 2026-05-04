/**
 *----------------------------------------
 * parallelCompiler.cpp
 * ---------------------------------------
 * UNIVERSIDAD DEL VALLE DE GUATEMALA
 * CC3086 - Programación de microprocesadores
 * Fecha: 2026/04/12
 * Autor: Daniel Vásquez :)
 * ---------------------------------------
 *
 * OPCODE (bits 6-4):
 *   000 -> Suma           (A + B)
 *   001 -> Resta          (A - B)
 *   010 -> Multiplicacion (A * B)
 *   011 -> Division entera(A / B)
 *   101 -> Potencia       (A^B)
 *   110 -> Modulo         (A mod B)
 *
 * Distribucion:
 *   Proceso padre    -> instruccion 1
 *   Primer hijo      -> instruccion 2
 *   Segundo hijo     -> instruccion 3
 *   ...
 *   (n-1)-esimo hijo -> instruccion n
 *
 * Librerias utilizadas y justificacion:
 *   <iostream>    - Entrada/salida estandar (cin, cout, cerr).
 *   <sstream>     - istringstream: parsear la linea de entrada en tokens
 *                   separados por espacio sin manipulacion manual de indices.
 *   <vector>      - Almacenar dinamicamente las instrucciones leidas;
 *                   su tamaño no se conoce en tiempo de compilacion.
 *   <string>      - Manejar cadenas de instrucciones binarias.
 *   <cmath>       - pow(): calcular potencias en el OPCODE 101.
 *   <chrono>      - high_resolution_clock: medir tiempo de ejecucion
 *                   de cada proceso con precision de microsegundos.
 *   <sys/types.h> - pid_t: tipo para identificadores de proceso POSIX.
 *   <sys/wait.h>  - wait(): bloquear el padre hasta que un hijo termine,
 *                   evitando procesos zombie y liberando recursos.
 *   <unistd.h>    - fork(): crear procesos hijo; getpid(): obtener PID.
 *   <cstdlib>     - EXIT_SUCCESS, EXIT_FAILURE: codigos de salida estandar.
 * ----------------------------------------
 */
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <pthread.h>

using namespace std;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
struct DatosHilo {
    string instruccion;
    int numInstruccion;
};

bool esInstruccionValida(const string& instruccion);
int binarioADecimal(const string& bits);

void* funcionHilo(void* arg) {

    DatosHilo* datos = (DatosHilo*)arg;
    string instruccion = datos->instruccion;
    int numInstruccion = datos->numInstruccion;

    auto inicio = chrono::high_resolution_clock::now();

    string opcodeBits    = instruccion.substr(0, 3);
    string operandoABits = instruccion.substr(3, 2);
    string operandoBBits = instruccion.substr(5, 2);

    int opcode    = binarioADecimal(opcodeBits);
    int operandoA = binarioADecimal(operandoABits);
    int operandoB = binarioADecimal(operandoBBits);

    string nombreOp;
    bool opcodeValido = true;

    switch (opcode)
    {
        case 0: nombreOp = "Suma";           break;
        case 1: nombreOp = "Resta";          break;
        case 2: nombreOp = "Multiplicacion"; break;
        case 3: nombreOp = "Division";       break;
        case 5: nombreOp = "Potencia";       break;
        case 6: nombreOp = "Modulo";         break;
        default:
            nombreOp = "OPCODE invalido";
            opcodeValido = false;
            break;
    }

    int validas = 0;
    int invalidas = 0;
    long long resultado = 0;

    bool esValida = true;

    if (!opcodeValido)
    {
        esValida = false;
    }
    else if (opcode == 3 && operandoB == 0)
    {
        esValida = false;
    }
    else if (opcode == 6 && operandoB == 0)
    {
        esValida = false;
    }
    else
    {
        switch (opcode)
        {
            case 0: resultado = operandoA + operandoB;               break;
            case 1: resultado = operandoA - operandoB;               break;
            case 2: resultado = operandoA * operandoB;               break;
            case 3: resultado = operandoA / operandoB;               break;
            case 5: resultado = (long long)pow(operandoA, operandoB); break;
            case 6: resultado = operandoA % operandoB;               break;
        }
    }

    if (esValida) validas++;
    else invalidas++;

    auto fin      = chrono::high_resolution_clock::now();
    auto duracion = chrono::duration_cast<chrono::microseconds>(fin - inicio).count();

    // 🔒 BLOQUE CRÍTICO (todo el cout protegido)
    pthread_mutex_lock(&lock);

    cout << "\nHilo TID: " << pthread_self()
         << " ejecutando instruccion " << numInstruccion << endl;

    cout << "  Instruccion " << numInstruccion << ": " << instruccion << endl;
    cout << "   OPCODE: " << opcode << " (" << nombreOp << ")" << endl;
    cout << "   A: " << operandoA << endl;
    cout << "   B: " << operandoB << endl;

    if (!esValida)
    {
        if (!opcodeValido)
            cout << "   Resultado: Error. OPCODE no reconocido." << endl;
        else if (opcode == 3 && operandoB == 0)
            cout << "   Resultado: Error. Division por cero." << endl;
        else if (opcode == 6 && operandoB == 0)
            cout << "   Resultado: Error. Modulo por cero." << endl;

        cout << "Estado: NO VALIDO" << endl;
    }
    else
    {
        cout << "   Resultado: " << resultado << endl;
        cout << "Estado: VALIDO" << endl;
    }

    cout << "Resumen hilo " << numInstruccion
         << ": Validas=" << validas
         << ", Invalidas=" << invalidas << endl;

    cout << " - Tiempo de ejecucion: " << duracion << " microsegundos." << endl;

    pthread_mutex_unlock(&lock);

    delete datos;
    pthread_exit(NULL);
}

bool esInstruccionValida(const string& instruccion)
{
    if (instruccion.size() != 7)
    {
        return false;
    }
    for (char c : instruccion)
    {
        if (c != '0' && c != '1')
        {
            return false;
        }
    }
    return true;
}

int binarioADecimal(const string& bits)
{
    int resultado = 0;
    for (char c : bits)
    {
        resultado = resultado * 2 + (c - '0');
    }
    return resultado;
}

void ejecutarInstruccion(const string& instruccion, int numInstruccion, bool esPadre)
{

    auto inicio = chrono::high_resolution_clock::now();

    string opcodeBits    = instruccion.substr(0, 3); // bits 6-4
    string operandoABits = instruccion.substr(3, 2); // bits 3-2
    string operandoBBits = instruccion.substr(5, 2); // bits 1-0

    int opcode    = binarioADecimal(opcodeBits);
    int operandoA = binarioADecimal(operandoABits);
    int operandoB = binarioADecimal(operandoBBits);

    // Mapear OPCODE
    string nombreOp;
    switch (opcode)
    {
        case 0: nombreOp = "Suma";           break;
        case 1: nombreOp = "Resta";          break;
        case 2: nombreOp = "Multiplicacion"; break;
        case 3: nombreOp = "Division";       break;
        case 5: nombreOp = "Potencia";       break;
        case 6: nombreOp = "Modulo";         break;
        default:
            nombreOp = "OPCODE invalido";    break;
    }

    if (esPadre)
        cout << "\nProceso padre ejecutando instruccion " << numInstruccion
            << ", TID: " << pthread_self() << endl;
    else
        cout << "\nProceso hijo ejecutando instruccion " << numInstruccion
            << ", TID: " << pthread_self() << endl;

    cout << "  Instruccion " << numInstruccion << ": " << instruccion << endl;
    cout << "   OPCODE: " << opcode << " (" << nombreOp << ")" << endl;
    cout << "   A: " << operandoA << endl;
    cout << "   B: " << operandoB << endl;

    if (nombreOp == "OPCODE invalido")
    {
        cout << "   Resultado: Error. OPCODE " << opcode << " no reconocido." << endl;
    }
    else if (opcode == 3 && operandoB == 0)
    {
        cout << "   Resultado: Error. Division por cero." << endl;
    }
    else if (opcode == 6 && operandoB == 0)
    {
        cout << "   Resultado: Error. Modulo por cero." << endl;
    }
    else
    {
        long long resultado = 0;
        switch (opcode)
        {
            case 0: resultado = operandoA + operandoB;               break;
            case 1: resultado = operandoA - operandoB;               break;
            case 2: resultado = operandoA * operandoB;               break;
            case 3: resultado = operandoA / operandoB;               break;
            case 5: resultado = (long long)pow(operandoA, operandoB); break;
            case 6: resultado = operandoA % operandoB;               break;
        }
        cout << "   Resultado: " << resultado << endl;
    }

    auto fin      = chrono::high_resolution_clock::now();
    auto duracion = chrono::duration_cast<chrono::microseconds>(fin - inicio).count();
    cout << "\nTID: " << pthread_self() << " - Tiempo de ejecucion: "
         << duracion << " microsegundos." << endl;
}

int main()
{
    string linea;
    vector<string> instrucciones;

    cout << "Ingresa instrucciones binarias (separadas por espacios): ";
    getline(cin, linea);

    // istringstream permite leer la cadena como si fuera cin,
    // extrayendo tokens separados por espacios en blanco.
    istringstream ss(linea);
    string token;
    bool hayInvalidas = false;

    while (ss >> token)
    {
        if (!esInstruccionValida(token))
        {
            cerr << "Error: \"" << token
                 << "\" no es valida. Debe tener exactamente 7 bits (solo 0 y 1)." << endl;
            hayInvalidas = true;
        }
        else
        {
            instrucciones.push_back(token);
        }
    }

    if (hayInvalidas)
    {
        cerr << "Instrucciones invalidas detectadas. Abortando ejecucion." << endl;
        return EXIT_FAILURE;
    }

    int n = (int)instrucciones.size();

    if (n < 3)
    {
        cerr << "Error: se requieren al menos 3 instrucciones validas. "
             << "Se ingresaron: " << n << "." << endl;
        return EXIT_FAILURE;
    }

    cout << "\nCreando instrucciones para procesos e hilos..." << endl;

    vector<pthread_t> hilos(n);

    for (int i = 0; i < n; i++)
    {
        DatosHilo* datos = new DatosHilo{instrucciones[i], i + 1}; // Crear struct con datos para el hilo
        if (pthread_create(&hilos[i], NULL, funcionHilo, datos) != 0)
        {
            cerr << "Error: no se pudo crear el hilo para la instruccion "
                 << i + 1 << "." << endl;
            delete datos; // Liberar memoria si no se pudo crear el hilo
            continue;
        }
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(hilos[i], NULL);
    }

    cout << "\nTodos los procesos han finalizado. Compilador paralelo terminado." << endl;

    return EXIT_SUCCESS;
}