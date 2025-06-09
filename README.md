#include <iostream>
#include <string>

using namespace std;

// --- Estructuras base ---

struct Proceso {
    int pid;
    string nombre;
    int prioridad;
    int tiempoRestante;
    Proceso* siguiente;

    Proceso(int id, const string& nom, int prio, int tiempo)
        : pid(id), nombre(nom), prioridad(prio), tiempoRestante(tiempo), siguiente(NULL) {}
};

// --- Gestor de Procesos (lista enlazada) ---

struct GestorProcesos {
    Proceso* cabeza = NULL;

    void insertarProceso(int pid, const string& nombre, int prioridad, int tiempo) {
        Proceso* nuevo = new Proceso(pid, nombre, prioridad, tiempo);
        if (cabeza == NULL) {
            cabeza = nuevo;
        } else {
            Proceso* temp = cabeza;
            while (temp->siguiente != NULL)
                temp = temp->siguiente;
            temp->siguiente = nuevo;
        }
        cout << "Proceso insertado: " << nombre << " (PID " << pid << ")\n";
    }

    Proceso* buscarPorID(int pid) {
        Proceso* temp = cabeza;
        while (temp != NULL) {
            if (temp->pid == pid)
                return temp;
            temp = temp->siguiente;
        }
        return NULL;
    }

    Proceso* buscarPorNombre(const string& nombre) {
        Proceso* temp = cabeza;
        while (temp != NULL) {
            if (temp->nombre == nombre)
                return temp;
            temp = temp->siguiente;
        }
        return NULL;
    }

    bool eliminarProceso(int pid) {
        if (cabeza == NULL) return false;
        if (cabeza->pid == pid) {
            Proceso* aEliminar = cabeza;
            cabeza = cabeza->siguiente;
            delete aEliminar;
            return true;
        }
        Proceso* temp = cabeza;
        while (temp->siguiente != NULL && temp->siguiente->pid != pid)
            temp = temp->siguiente;
        if (temp->siguiente == NULL) return false;
        Proceso* aEliminar = temp->siguiente;
        temp->siguiente = aEliminar->siguiente;
        delete aEliminar;
        return true;
    }

    bool modificarPrioridad(int pid, int nuevaPrioridad) {
        Proceso* p = buscarPorID(pid);
        if (p != NULL) {
            p->prioridad = nuevaPrioridad;
            return true;
        }
        return false;
    }

    void mostrarProcesos() {
        cout << "\nLista de Procesos Registrados:\n";
        Proceso* temp = cabeza;
        if (!temp) {
            cout << "No hay procesos registrados.\n";
            return;
        }
        while (temp != NULL) {
            cout << "PID: " << temp->pid 
                 << " | Nombre: " << temp->nombre 
                 << " | Prioridad: " << temp->prioridad 
                 << " | Tiempo restante: " << temp->tiempoRestante << "\n";
            temp = temp->siguiente;
        }
    }
};

// --- Nodo para cola de prioridad ---

struct NodoCola {
    Proceso* proceso;
    NodoCola* siguiente;
    NodoCola(Proceso* p) : proceso(p), siguiente(NULL) {}
};

// --- Planificador CPU: cola de prioridad ---

struct PlanificadorCPU {
    NodoCola* frente = NULL;

    void encolar(Proceso* p) {
        NodoCola* nuevo = new NodoCola(p);
        if (frente == NULL || p->prioridad > frente->proceso->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            NodoCola* temp = frente;
            while (temp->siguiente != NULL && temp->siguiente->proceso->prioridad >= p->prioridad)
                temp = temp->siguiente;
            nuevo->siguiente = temp->siguiente;
            temp->siguiente = nuevo;
        }
        cout << "Proceso PID " << p->pid << " encolado con prioridad " << p->prioridad << ".\n";
    }

    Proceso* desencolar() {
        if (frente == NULL) return NULL;
        NodoCola* temp = frente;
        Proceso* p = temp->proceso;
        frente = frente->siguiente;
        delete temp;
        return p;
    }

    bool vacia() {
        return frente == NULL;
    }

    void mostrarCola() {
        cout << "\nCola de Prioridad de Procesos:\n";
        if (frente == NULL) {
            cout << "Cola vac�a.\n";
            return;
        }
        NodoCola* temp = frente;
        while (temp != NULL) {
            cout << "PID: " << temp->proceso->pid 
                 << " | Nombre: " << temp->proceso->nombre 
                 << " | Prioridad: " << temp->proceso->prioridad 
                 << " | Tiempo restante: " << temp->proceso->tiempoRestante << "\n";
            temp = temp->siguiente;
        }
    }
};

// --- Gestor de Memoria (pila) ---

struct BloqueMemoria {
    int pid;   // proceso asignado
    int tam;   // tama�o memoria asignada
    BloqueMemoria* siguiente;

    BloqueMemoria(int pid_, int tam_) : pid(pid_), tam(tam_), siguiente(NULL) {}
};

struct GestorMemoria {
    BloqueMemoria* cima = NULL;

    void asignarMemoria(int pid, int tam) {
        BloqueMemoria* nuevo = new BloqueMemoria(pid, tam);
        nuevo->siguiente = cima;
        cima = nuevo;
        cout << "Asignado bloque de " << tam << " a proceso PID " << pid << ".\n";
    }

    bool liberarMemoria() {
        if (cima == NULL) return false;
        BloqueMemoria* temp = cima;
        cout << "Liberando bloque de tama�o " << cima->tam << " asignado a PID " << cima->pid << ".\n";
        cima = cima->siguiente;
        delete temp;
        return true;
    }

    void mostrarEstado() {
        cout << "\nEstado actual de la memoria (Pila de bloques):\n";
        if (cima == NULL) {
            cout << "No hay bloques asignados.\n";
            return;
        }
        BloqueMemoria* temp = cima;
        while (temp != NULL) {
            cout << "PID: " << temp->pid << " | Tama�o bloque: " << temp->tam << "\n";
            temp = temp->siguiente;
        }
    }
};

// --- Programa principal con men� ---

int main() {
    GestorProcesos gestor;
    PlanificadorCPU planificador;
    GestorMemoria memoria;

    int pidCounter = 1;
    int opcion;

    do {
        cout << "\n--- Menu Principal ---\n";
        cout << "1. Insertar nuevo proceso\n";
        cout << "2. Eliminar proceso\n";
        cout << "3. Buscar proceso (por ID o nombre)\n";
        cout << "4. Modificar prioridad de proceso\n";
        cout << "5. Encolar proceso en planificador CPU\n";
        cout << "6. Ejecutar siguiente proceso (Planificador CPU)\n";
        cout << "7. Mostrar cola de procesos en planificador\n";
        cout << "8. Asignar bloque de memoria a proceso\n";
        cout << "9. Liberar bloque de memoria\n";
        cout << "10. Mostrar estado actual de memoria\n";
        cout << "11. Mostrar todos los procesos\n";
        cout << "12. Salir\n";
        cout << "Ingrese opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1: {
                string nombre;
                int prioridad, tiempo;
                cout << "Nombre proceso: "; cin >> nombre;
                cout << "Prioridad (entero mayor es mayor prioridad): "; cin >> prioridad;
                cout << "Tiempo de ejecucion: "; cin >> tiempo;
                gestor.insertarProceso(pidCounter++, nombre, prioridad, tiempo);
                break;
            }
            case 2: {
                int pid;
                cout << "Ingrese PID a eliminar: "; cin >> pid;
                if (gestor.eliminarProceso(pid)) cout << "Proceso eliminado.\n";
                else cout << "Proceso no encontrado.\n";
                break;
            }
            case 3: {
                int subop;
                cout << "Buscar por: 1) PID  2) Nombre : "; cin >> subop;
                if (subop == 1) {
                    int pid; 
                    cout << "Ingrese PID: "; 
                    cin >> pid;
                    Proceso* p = gestor.buscarPorID(pid);
                    if (p) 
                        cout << "Encontrado: PID " << p->pid 
                             << " Nombre: " << p->nombre 
                             << " Prioridad: " << p->prioridad 
                             << " Tiempo: " << p->tiempoRestante << endl;
                    else
                        cout << "Proceso no encontrado.\n";
                } else if (subop == 2) {
                    string nombre;
                    cout << "Ingrese nombre: "; 
                    cin >> nombre;
                    Proceso* p = gestor.buscarPorNombre(nombre);
                    if (p) 
                        cout << "Encontrado: PID " << p->pid 
                             << " Nombre: " << p->nombre 
                             << " Prioridad: " << p->prioridad 
                             << " Tiempo: " << p->tiempoRestante << endl;
                    else
                        cout << "Proceso no encontrado.\n";
                } else {
                    cout << "Opcion invalida.\n";
                }
                break;
            }
            case 4: {
                int pid, nuevaPrioridad;
                cout << "Ingrese PID del proceso a modificar prioridad: "; cin >> pid;
                cout << "Ingrese nueva prioridad: "; cin >> nuevaPrioridad;
                if (gestor.modificarPrioridad(pid, nuevaPrioridad))
                    cout << "Prioridad modificada.\n";
                else
                    cout << "Proceso no encontrado.\n";
                break;
            }
            case 5: {
                int pid;
                cout << "Ingrese PID del proceso para encolar: "; cin >> pid;
                Proceso* p = gestor.buscarPorID(pid);
                if (p) {
                    planificador.encolar(p);
                } else {
                    cout << "Proceso no encontrado.\n";
                }
                break;
            }
            case 6: {
                if (planificador.vacia()) {
                    cout << "No hay procesos en cola para ejecutar.\n";
                } else {
                    Proceso* p = planificador.desencolar();
                    cout << "Ejecutando proceso PID " << p->pid << " (" << p->nombre << "). Tiempo restante antes: " << p->tiempoRestante << "\n";
                    p->tiempoRestante--;
                    if (p->tiempoRestante > 0) {
                        cout << "Proceso PID " << p->pid << " no ha terminado. Tiempo restante: " << p->tiempoRestante << ". Reencolando.\n";
                        planificador.encolar(p);
                    } else {
                        cout << "Proceso PID " << p->pid << " ha terminado su ejecuci�n.\n";
                        // Opcional: eliminar de la lista procesos si quieres.
                    }
                }
                break;
            }
            case 7: {
                planificador.mostrarCola();
                break;
            }
            case 8: {
                int pid, tam;
                cout << "Ingrese PID del proceso para asignar memoria: "; cin >> pid;
                Proceso* p = gestor.buscarPorID(pid);
                if (p) {
                    cout << "Ingrese tama�o del bloque de memoria a asignar: "; cin >> tam;
                    memoria.asignarMemoria(pid, tam);
                } else {
                    cout << "Proceso no encontrado.\n";
                }
                break;
            }
            case 9: {
                if (!memoria.liberarMemoria())
                    cout << "No hay bloques de memoria para liberar.\n";
                break;
            }
            case 10: {
                memoria.mostrarEstado();
                break;
            }
            case 11: {
                gestor.mostrarProcesos();
                break;
            }
            case 12: {
                cout << "Saliendo del programa...\n";
                break;
            }
            default:
                cout << "Opcion inv�lida. Intente nuevamente.\n";
        }

    } while (opcion != 12);

    return 0;
}

