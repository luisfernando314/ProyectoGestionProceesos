#include <iostream>
#include <cstring>
#include <fstream>

using namespace std;

// Función para detectar si una cadena representa un número entero
bool esNumero(const char* s) {
    if (s == nullptr || *s == '\0') return false;
    int i = 0;
    if (s[0] == '-' && s[1] != '\0') i = 1;
    for (; s[i] != '\0'; ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

// Estructura de un proceso
struct Proceso {
    int pid;
    char nombre[32];
    int prioridad;
    int memRequerida;
    int tiempoRestante;
};

// Nodo para la lista enlazada de procesos
struct NodoProceso {
    Proceso* dato;
    NodoProceso* siguiente;
};

// Nodo para la cola de prioridad
struct NodoCola {
    Proceso* dato;
    NodoCola* siguiente;
};

// Estructura de Memoria
struct BloqueMemoria {
    Proceso* proceso;
    int inicio;
    int tamano;
    bool libre;
};

struct NodoMemoria {
    BloqueMemoria dato;
    NodoMemoria* siguiente;
};

NodoMemoria* pilaMemoria = nullptr;
const int MEMORIA_TOTAL = 40960;  // 40KB de memoria total
int memoriaDisponible = MEMORIA_TOTAL;

// --------------------------
// Funciones para Lista (Procesos)
// --------------------------

// Crear un nuevo nodo de lista
NodoProceso* crearNodoLista(Proceso* p) {
    return new NodoProceso{p, nullptr};
}

// Insertar un proceso al final de la lista
void insertarEnLista(NodoProceso*& cabeza, Proceso* p) {
    NodoProceso* nodo = crearNodoLista(p);
    if (!cabeza) {
        cabeza = nodo;
    } else {
        NodoProceso* it = cabeza;
        while (it->siguiente) it = it->siguiente;
        it->siguiente = nodo;
    }
}

// Buscar proceso en lista por ID o nombre
NodoProceso* buscarEnLista(NodoProceso* cabeza, const char* entrada) {
    bool numero = esNumero(entrada);
    NodoProceso* it = cabeza;
    while (it) {
        if (numero && it->dato->pid == stoi(entrada)) {
            return it;
        } else if (!numero && strcmp(it->dato->nombre, entrada) == 0) {
            return it;
        }
        it = it->siguiente;
    }
    return nullptr;
}

// Eliminar proceso de la lista por puntero
bool eliminarDeLista(NodoProceso*& cabeza, Proceso* procAEliminar) {
    if (!cabeza || !procAEliminar) return false;

    NodoProceso* ant = nullptr;
    NodoProceso* it = cabeza;
    
    while (it && it->dato != procAEliminar) {
        ant = it;
        it = it->siguiente;
    }
    
    if (!it) return false; // No se encontró el proceso
    
    if (!ant) cabeza = it->siguiente;
    else ant->siguiente = it->siguiente;
    
    delete it;
    return true;
}

// Mostrar todos los procesos en la lista
void mostrarLista(NodoProceso* cabeza) {
    cout << "PID\tNombre\t\tPrioridad\tMemoria\t\tTiempo Restante" << endl;
    NodoProceso* it = cabeza;
    while (it) {
        Proceso* p = it->dato;
        cout << p->pid << "\t" << p->nombre
             << "\t\t" << p->prioridad
             << "\t\t" << p->memRequerida
             << "\t\t" << p->tiempoRestante << endl;
        it = it->siguiente;
    }
}

// Liberar toda la lista
void liberarLista(NodoProceso*& cabeza) {
    while (cabeza) {
        NodoProceso* sig = cabeza->siguiente;
        delete cabeza->dato;
        delete cabeza;
        cabeza = sig;
    }
}

// --------------------------
// Funciones para Cola de Prioridad
// --------------------------

// Encolar según prioridad (lista ordenada creciente)
void encolarPrioridad(NodoCola*& frente, Proceso* p) {
    NodoCola* nodo = new NodoCola{p, nullptr};
    if (!frente || p->prioridad < frente->dato->prioridad) {
        nodo->siguiente = frente; 
        frente = nodo;
    } else {
        NodoCola* it = frente;
        while (it->siguiente && it->siguiente->dato->prioridad <= p->prioridad)
            it = it->siguiente;
        nodo->siguiente = it->siguiente;
        it->siguiente = nodo;
    }
}

// Desencolar: extraer frente
Proceso* desencolarPrioridad(NodoCola*& frente) {
    if (!frente) return nullptr;
    Proceso* p = frente->dato;
    NodoCola* aux = frente;
    frente = frente->siguiente;
    delete aux;
    return p;
}

// Mostrar cola de prioridad
void mostrarCola(NodoCola* frente) {
    cout << "PID\tNombre\t\tPrioridad\t\tTiempo Restante" << endl;
    for (NodoCola* it = frente; it; it = it->siguiente) {
        Proceso* p = it->dato;
        cout << p->pid << "\t" << p->nombre << "\t\t" << p->prioridad << "\t\t" << p->tiempoRestante << endl;
    }
}

// Liberar cola de prioridad
void liberarCola(NodoCola*& frente) {
    while (frente) {
        NodoCola* sig = frente->siguiente;
        // No eliminamos el proceso, solo el nodo de la cola
        delete frente;
        frente = sig;
    }
}

// Reconstruir cola desde la lista principal
void reconstruirCola(NodoCola*& frente, NodoProceso* lista) {
    liberarCola(frente);
    NodoProceso* it = lista;
    while (it) {
        // Ahora usamos el mismo proceso, no una copia
        encolarPrioridad(frente, it->dato);
        it = it->siguiente;
    }
}

// --------------------------
// Funciones para Pila de Memoria
// --------------------------

// Asignar memoria a un proceso (push en pila)
void pushMemoria(Proceso* p) {
    int inicio = MEMORIA_TOTAL - memoriaDisponible;
    NodoMemoria* nodo = new NodoMemoria{{p, inicio, p->memRequerida, false}, nullptr};
    nodo->dato.libre = false;
    nodo->siguiente = pilaMemoria;
    pilaMemoria = nodo;
    memoriaDisponible -= p->memRequerida;
}

// Liberar memoria de un proceso (marcar como libre)
void liberarMemoria(Proceso* p) {
    for (NodoMemoria* it = pilaMemoria; it; it = it->siguiente) {
        if (it->dato.proceso == p && !it->dato.libre) {
            it->dato.libre = true;
            memoriaDisponible += it->dato.tamano;
            return;
        }
    }
}

// Mostrar estado de la memoria
void mostrarEstadoMemoria() {
    cout << "--- Estado de Memoria ---" << endl;
    cout << "Proceso\tInicio\tTamano\tEstado" << endl;
    for (NodoMemoria* it = pilaMemoria; it; it = it->siguiente) {
        auto& b = it->dato;
        cout << b.proceso->nombre << "\t" << b.inicio << "\t" << b.tamano << "\t"
             << (b.libre ? "Libre" : "Ocupado") << endl;
    }
    cout << "Memoria disponible: " << memoriaDisponible << " / " << MEMORIA_TOTAL << endl;
}

// --------------------------
// Funciones auxiliares
// --------------------------

// Crear un nuevo proceso
Proceso* crearProceso(int pid, const char* nombre, int prioridad, int memReq, int tiempo) {
    Proceso* p = new Proceso;
    p->pid = pid;
    strncpy(p->nombre, nombre, sizeof(p->nombre) - 1);
    p->nombre[sizeof(p->nombre) - 1] = '\0';
    p->prioridad = prioridad;
    p->memRequerida = memReq;
    p->tiempoRestante = tiempo;  // Nuevo campo
    return p;
}

// Modificar prioridad de un proceso
bool modificarPrioridad(NodoProceso* cabeza, NodoCola*& cola, int pid, int nuevaPrioridad) {
    NodoProceso* it = cabeza;
    while (it) {
        if (it->dato->pid == pid) {
            it->dato->prioridad = nuevaPrioridad;
            reconstruirCola(cola, cabeza);
            return true;
        }
        it = it->siguiente;
    }
    return false;
}

// Eliminar proceso por PID
bool eliminarPorPID(NodoProceso*& lista, NodoCola*& cola, int pid) {
    NodoProceso* it = lista;
    while (it && it->dato->pid != pid) {
        it = it->siguiente;
    }
    if (!it) return false;

    Proceso* procAEliminar = it->dato;
    liberarMemoria(procAEliminar);
    eliminarDeLista(lista, procAEliminar);
    reconstruirCola(cola, lista);
    return true;
}

// --------------------------
// Funciones para guardar y cargar datos
// --------------------------

void guardarDatos(NodoProceso* lista) {
    ofstream archivo("procesos.dat");
    if (!archivo) {
        cerr << "Error al abrir el archivo para guardar." << endl;
        return;
    }

    NodoProceso* it = lista;
    while (it) {
        Proceso* p = it->dato;
        archivo << p->pid << " " 
                << p->nombre << " " 
                << p->prioridad << " " 
                << p->memRequerida << " " 
                << p->tiempoRestante << "\n";
        it = it->siguiente;
    }

    archivo.close();
    cout << "Datos guardados correctamente en 'procesos.dat'" << endl;
}

void cargarDatos(NodoProceso*& lista, NodoCola*& cola, int& pidCounter) {
    ifstream archivo("procesos.dat");
    if (!archivo) {
        cout << "No se encontró archivo de datos. Se iniciará con datos nuevos." << endl;
        return;
    }

    // Limpiar estructuras existentes
    liberarLista(lista);
    liberarCola(cola);
    lista = nullptr;
    cola = nullptr;
    memoriaDisponible = MEMORIA_TOTAL;
    pilaMemoria = nullptr;

    int pid, prioridad, memReq, tiempo;
    char nombre[32];
    pidCounter = 1;

    while (archivo >> pid >> nombre >> prioridad >> memReq >> tiempo) {
        // Actualizar el contador de PID si es necesario
        if (pid >= pidCounter) {
            pidCounter = pid + 1;
        }

        Proceso* p = crearProceso(pid, nombre, prioridad, memReq, tiempo);
        insertarEnLista(lista, p);
        encolarPrioridad(cola, p);
        pushMemoria(p);
    }

    archivo.close();
    cout << "Datos cargados correctamente desde 'procesos.dat'" << endl;
}

// Menú principal
int main() {
    NodoProceso* lista = nullptr;
    NodoCola* cola = nullptr;
    int opcion;
    char entrada[32];
    int pidCounter = 1;

    // Cargar datos al iniciar
    cargarDatos(lista, cola, pidCounter);

    do {
        cout << "\n--- GESTOR DE PROCESOS ---" << endl;
        cout << "1. Insertar nuevo proceso" << endl;
        cout << "2. Mostrar todos los procesos" << endl;
        cout << "3. Buscar proceso (por ID o nombre)" << endl;
        cout << "4. Eliminar proceso por ID" << endl;
        cout << "5. Modificar prioridad" << endl;
        cout << "6. Ejecutar Proceso Prioritario" << endl;
        cout << "7. Visualizar Procesos por Prioridad" << endl;
        cout << "8. Verificar Estado de Memoria" << endl;
        cout << "9. Guardar Datos Localmente" << endl;
        cout << "10. Salir" << endl;
        cout << "Opcion: ";
        cin.getline(entrada, sizeof(entrada));

        opcion = (esNumero(entrada) ? stoi(entrada) : -1);

        switch (opcion) {
            case 1: {
                cout << "Ingrese nombre: ";
                char nombre[32];
                cin.getline(nombre, sizeof(nombre));

                cout << "Ingrese prioridad (0 = alta): ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { cout << "Prioridad no válida.\n"; break; }
                int prioridad = stoi(entrada);

                cout << "Ingrese memoria requerida: ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { cout << "Memoria no válida.\n"; break; }
                int memoria = stoi(entrada);

                cout << "Ingrese tiempo de ejecución requerido: ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { /*...*/ }
                int tiempo = stoi(entrada);

                if (memoria > memoriaDisponible) {
                    cout << "Error: No hay suficiente memoria disponible (" 
                        << memoriaDisponible << " bytes libres de " << MEMORIA_TOTAL << ")\n";
                    break;
                }
                int pid = pidCounter++;
                
                Proceso* nuevoProceso = crearProceso(pid, nombre, prioridad, memoria, tiempo);
                insertarEnLista(lista, nuevoProceso);
                encolarPrioridad(cola, nuevoProceso);
                pushMemoria(nuevoProceso);
                cout << "Proceso agregado. PID asignado: " << pid << "\n";
                break;
            }
            case 2:
                mostrarLista(lista);
                break;
            case 3: {
                cout << "Ingrese ID o nombre: ";
                cin.getline(entrada, sizeof(entrada));
                NodoProceso* encontrado = buscarEnLista(lista, entrada);
                if (encontrado) {
                    Proceso* p = encontrado->dato;
                    cout << "Encontrado: PID " << p->pid
                         << ", Nombre: " << p->nombre
                         << ", Prioridad: " << p->prioridad
                         << ", Memoria: " << p->memRequerida << endl;
                } else cout << "No se encontró el proceso.\n";
                break;
            }
            case 4: {
                cout << "Ingrese PID a eliminar: ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { cout << "PID no válido.\n"; break; }
                int pid = stoi(entrada);
                NodoProceso* nodo = buscarEnLista(lista, entrada);
                if (nodo) liberarMemoria(nodo->dato);
                cout << (eliminarPorPID(lista, cola, pid) ? "Proceso eliminado.\n" : "No se encontró el proceso.\n");
                break;
            }
            case 5: {
                cout << "Ingrese PID a modificar: ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { cout << "PID no válido.\n"; break; }
                int pid = stoi(entrada);

                cout << "Nueva prioridad: ";
                cin.getline(entrada, sizeof(entrada));
                if (!esNumero(entrada)) { cout << "Prioridad no válida.\n"; break; }
                int nueva = stoi(entrada);

                cout << (modificarPrioridad(lista, cola, pid, nueva) ? "Prioridad actualizada.\n" : "No se encontró el proceso.\n");
                break;
            }
            case 6: {
                Proceso* p = desencolarPrioridad(cola);
                if (p) {
                    // Simular ejecución: reducir tiempo restante
                    p->tiempoRestante--;
                    cout << "Ejecutando PID " << p->pid << " N:" << p->nombre 
                        << ". Tiempo restante: " << p->tiempoRestante << "\n";

                    if (p->tiempoRestante > 0) {
                        // Reencolar si aún queda tiempo
                        encolarPrioridad(cola, p);
                        cout << "Proceso reencolado.\n";
                    } else {
                        // Finalizar proceso si no queda tiempo
                        liberarMemoria(p);
                        eliminarDeLista(lista, p);
                        cout << "Proceso completado. Liberando recursos.\n";
                    }
                }
                else cout << "Cola vacía\n";
                break; 
            }
            case 7: 
                mostrarCola(cola); 
                break;
            case 8:
                mostrarEstadoMemoria();
                break;
            case 9:
                guardarDatos(lista);
                break;
            case 10: 
                guardarDatos(lista); // Guardar antes de salir
                liberarCola(cola); 
                liberarLista(lista); 
                cout << "Saliendo\n"; 
                break;
            default: cout << "Opcion invalida\n";
        }
    } while (opcion != 10);
    return 0;
}