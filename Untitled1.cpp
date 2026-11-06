#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cctype>
using namespace std;


const int RAM_TOTAL = 1024;            
const int EST_LISTO = 0;
const int EST_EJECUCION = 1;
const int EST_BLOQUEADO = 2;
const int EST_FINALIZADO = 3;
const char* ARCHIVO = "estado_sistema.txt";


string nombreEstado(int e) {
    switch (e) {
        case EST_LISTO:      return "LISTO";
        case EST_EJECUCION:  return "EJECUCION";
        case EST_BLOQUEADO:  return "BLOQUEADO";
        case EST_FINALIZADO: return "FINALIZADO";
    }
    return "DESCONOCIDO";
}

string nombrePrioridad(int p) {
    switch (p) {
        case 1: return "Alta";
        case 2: return "Media";
        case 3: return "Baja";
    }
    return "?";
}

string recortar(const string &s) {
    size_t i = 0;
    while (i < s.size() && isspace((unsigned char)s[i])) i++;
    size_t j = s.size();
    while (j > i && isspace((unsigned char)s[j - 1])) j--;
    return s.substr(i, j - i);
}

string aMinusculas(const string &s) {
    string r = s;
    for (size_t i = 0; i < r.size(); i++) r[i] = (char)tolower((unsigned char)r[i]);
    return r;
}

bool esNumero(const string &s) {
    if (s.empty() || s.size() > 9) return false;
    for (size_t i = 0; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i])) return false;
    return true;
}


int dividir(const string &s, char sep, string partes[], int max) {
    int cuenta = 0;
    string actual = "";
    for (size_t i = 0; i <= s.size(); i++) {
        if (i == s.size() || s[i] == sep) {
            if (cuenta < max) partes[cuenta] = actual;
            cuenta++;
            actual = "";
        } else {
            actual += s[i];
        }
    }
    return cuenta;
}


string leerLinea(const string &mensaje) {
    string s;
    cout << mensaje;
    if (!getline(cin, s)) {
        cout << "\n[Entrada finalizada. Cerrando el programa]\n";
        exit(0);
    }
    return recortar(s);
}

int leerEntero(const string &mensaje, int minimo, int maximo) {
    while (true) {
        string s = leerLinea(mensaje);
        if (!esNumero(s)) {
            cout << "  [!] Entrada invalida: ingrese solo numeros enteros positivos.\n";
            continue;
        }
        int v = atoi(s.c_str());
        if (v < minimo || v > maximo) {
            cout << "  [!] Valor fuera de rango (" << minimo << " a " << maximo << ").\n";
            continue;
        }
        return v;
    }
}

string leerNombre(const string &mensaje) {
    while (true) {
        string s = leerLinea(mensaje);
        if (s.empty()) { cout << "  [!] El nombre no puede estar vacio.\n"; continue; }
        if (s.size() > 30) { cout << "  [!] Maximo 30 caracteres.\n"; continue; }
        if (s.find('|') != string::npos) { cout << "  [!] El caracter '|' no esta permitido.\n"; continue; }
        return s;
    }
}

bool confirmar(const string &mensaje) {
    while (true) {
        string s = leerLinea(mensaje);
        if (s == "S" || s == "s") return true;
        if (s == "N" || s == "n") return false;
        cout << "  [!] Responda S o N.\n";
    }
}

void pausar() {
    leerLinea("\nPresione ENTER para continuar...");
}


struct Proceso {
    int id;
    string nombre;
    int prioridad;      
    int estado;         
    int memoria;        
};

struct NodoLista {
    Proceso dato;
    NodoLista* sig;
};

class ListaProcesos {
private:
    NodoLista* cabeza;
    NodoLista* ultimo;     
    int cantidad;
    ListaProcesos(const ListaProcesos &);            
    ListaProcesos& operator=(const ListaProcesos &);
public:
    ListaProcesos() : cabeza(NULL), ultimo(NULL), cantidad(0) {}
    ~ListaProcesos() { limpiar(); }

    int obtenerCantidad() const { return cantidad; }
    NodoLista* primero() const { return cabeza; }
    bool estaVacia() const { return cabeza == NULL; }

    
    void insertarFinal(const Proceso &p) {
        NodoLista* nuevo = new NodoLista;
        nuevo->dato = p;
        nuevo->sig = NULL;
        if (cabeza == NULL) {
            cabeza = nuevo;
            ultimo = nuevo;
        } else {
            ultimo->sig = nuevo;
            ultimo = nuevo;
        }
        cantidad++;
    }

    
    NodoLista* buscarPorId(int id) const {
        NodoLista* actual = cabeza;
        while (actual != NULL && actual->dato.id != id)
            actual = actual->sig;
        return actual;
    }

   
    int mostrarPorNombre(const string &fragmento) const {
        string buscado = aMinusculas(fragmento);
        int hallados = 0;
        for (NodoLista* a = cabeza; a != NULL; a = a->sig) {
            if (aMinusculas(a->dato.nombre).find(buscado) != string::npos) {
                if (hallados == 0) imprimirEncabezado();
                imprimirFila(a->dato);
                hallados++;
            }
        }
        return hallados;
    }

    
    bool eliminar(int id) {
        NodoLista* anterior = NULL;
        NodoLista* actual = cabeza;
        while (actual != NULL && actual->dato.id != id) {
            anterior = actual;
            actual = actual->sig;
        }
        if (actual == NULL) return false;
        if (anterior == NULL) cabeza = actual->sig;       
        else anterior->sig = actual->sig;
        if (actual == ultimo) ultimo = anterior;          
        delete actual;
        cantidad--;
        return true;
    }

    void limpiar() {
        while (cabeza != NULL) {
            NodoLista* temp = cabeza;
            cabeza = cabeza->sig;
            delete temp;
        }
        ultimo = NULL;
        cantidad = 0;
    }

    
    void intercambiar(ListaProcesos &o) {
        NodoLista* c = cabeza; cabeza = o.cabeza; o.cabeza = c;
        NodoLista* u = ultimo; ultimo = o.ultimo; o.ultimo = u;
        int n = cantidad; cantidad = o.cantidad; o.cantidad = n;
    }

    static void imprimirEncabezado() {
        cout << "\n  " << left << setw(7) << "ID" << setw(32) << "NOMBRE"
             << setw(11) << "PRIORIDAD" << setw(13) << "ESTADO" << "MEMORIA(MB)\n";
        cout << "  " << string(72, '-') << "\n";
    }
    static void imprimirFila(const Proceso &p) {
        cout << "  " << left << setw(7) << p.id << setw(32) << p.nombre
             << setw(11) << nombrePrioridad(p.prioridad)
             << setw(13) << nombreEstado(p.estado) << p.memoria << "\n";
    }
    void mostrar() const {
        if (cabeza == NULL) { cout << "\n  (No hay procesos registrados)\n"; return; }
        imprimirEncabezado();
        for (NodoLista* a = cabeza; a != NULL; a = a->sig) imprimirFila(a->dato);
        cout << "  " << string(72, '-') << "\n  Total: " << cantidad << " proceso(s)\n";
    }
};


struct NodoCola {
    int id;
    string nombre;
    int prioridad;
    int memoria;
    NodoCola* sig;
};

class ColaPrioridad {
private:
    NodoCola* frente;
    int cantidad;
    ColaPrioridad(const ColaPrioridad &);
    ColaPrioridad& operator=(const ColaPrioridad &);
public:
    ColaPrioridad() : frente(NULL), cantidad(0) {}
    ~ColaPrioridad() { limpiar(); }

    bool estaVacia() const { return frente == NULL; }
    int obtenerCantidad() const { return cantidad; }
    NodoCola* primero() const { return frente; }

 
    void encolar(int id, const string &nombre, int prioridad, int memoria) {
        NodoCola* nuevo = new NodoCola;
        nuevo->id = id;
        nuevo->nombre = nombre;
        nuevo->prioridad = prioridad;
        nuevo->memoria = memoria;
        nuevo->sig = NULL;
        if (frente == NULL || prioridad < frente->prioridad) {
            nuevo->sig = frente;                 
            frente = nuevo;
        } else {
            NodoCola* actual = frente;
            while (actual->sig != NULL && actual->sig->prioridad <= prioridad)
                actual = actual->sig;
            nuevo->sig = actual->sig;
            actual->sig = nuevo;
        }
        cantidad++;
    }

    
    bool desencolar(int &id, string &nombre, int &prioridad, int &memoria) {
        if (frente == NULL) return false;
        NodoCola* temp = frente;
        id = temp->id;
        nombre = temp->nombre;
        prioridad = temp->prioridad;
        memoria = temp->memoria;
        frente = frente->sig;
        delete temp;
        cantidad--;
        return true;
    }

    
    bool contiene(int id) const {
        NodoCola* a = frente;
        while (a != NULL && a->id != id) a = a->sig;
        return a != NULL;
    }

   
    bool eliminar(int id) {
        NodoCola* anterior = NULL;
        NodoCola* actual = frente;
        while (actual != NULL && actual->id != id) {
            anterior = actual;
            actual = actual->sig;
        }
        if (actual == NULL) return false;
        if (anterior == NULL) frente = actual->sig;
        else anterior->sig = actual->sig;
        delete actual;
        cantidad--;
        return true;
    }

    void limpiar() {
        while (frente != NULL) {
            NodoCola* temp = frente;
            frente = frente->sig;
            delete temp;
        }
        cantidad = 0;
    }

    void intercambiar(ColaPrioridad &o) {
        NodoCola* f = frente; frente = o.frente; o.frente = f;
        int n = cantidad; cantidad = o.cantidad; o.cantidad = n;
    }

    void mostrar() const {
        if (frente == NULL) { cout << "\n  (La cola de CPU esta vacia)\n"; return; }
        cout << "\n  " << left << setw(6) << "ORDEN" << setw(7) << "ID" << setw(32) << "NOMBRE"
             << setw(11) << "PRIORIDAD" << "MEMORIA(MB)\n";
        cout << "  " << string(66, '-') << "\n";
        int orden = 1;
        for (NodoCola* a = frente; a != NULL; a = a->sig, orden++) {
            cout << "  " << left << setw(6) << orden << setw(7) << a->id << setw(32) << a->nombre
                 << setw(11) << nombrePrioridad(a->prioridad) << a->memoria
                 << (orden == 1 ? "   <-- siguiente en CPU" : "") << "\n";
        }
        cout << "  " << string(66, '-') << "\n  Procesos en cola: " << cantidad << "\n";
    }
};


struct NodoPila {
    int idProceso;
    int tamano;
    NodoPila* sig;
};

class PilaMemoria {
private:
    NodoPila* tope;
    int usada;
    int bloques;
    PilaMemoria(const PilaMemoria &);
    PilaMemoria& operator=(const PilaMemoria &);

    static void guardarRec(const NodoPila* n, ofstream &f) {
        if (n == NULL) return;
        guardarRec(n->sig, f);                    
        f << n->idProceso << "|" << n->tamano << "\n";
    }
public:
    PilaMemoria() : tope(NULL), usada(0), bloques(0) {}
    ~PilaMemoria() { limpiar(); }

    bool estaVacia() const { return tope == NULL; }
    int memoriaUsada() const { return usada; }
    int memoriaLibre() const { return RAM_TOTAL - usada; }
    int obtenerBloques() const { return bloques; }

    /* Permite apilar un bloque de memoria (retorna false si no hay suficiente o hay desbordamiento / overflow). O(1) */
    bool push(int idProceso, int tamano) {
        if (tamano <= 0 || usada + tamano > RAM_TOTAL) return false;
        NodoPila* nuevo = new NodoPila;
        nuevo->idProceso = idProceso;
        nuevo->tamano = tamano;
        nuevo->sig = tope;
        tope = nuevo;
        usada += tamano;
        bloques++;
        return true;
    }

   
    bool pop(int &idProceso, int &tamano) {
        if (tope == NULL) return false;
        NodoPila* temp = tope;
        idProceso = temp->idProceso;
        tamano = temp->tamano;
        tope = tope->sig;
        usada -= tamano;
        bloques--;
        delete temp;
        return true;
    }

   
    NodoPila* buscar(int idProceso) const {
        NodoPila* a = tope;
        while (a != NULL && a->idProceso != idProceso) a = a->sig;
        return a;
    }

  
    bool eliminarBloque(int idProceso) {
        NodoPila* aux = NULL;
        bool hallado = false;
        while (tope != NULL && !hallado) {
            NodoPila* n = tope;
            tope = tope->sig;
            if (n->idProceso == idProceso) {
                usada -= n->tamano;
                bloques--;
                delete n;
                hallado = true;
            } else {
                n->sig = aux;
                aux = n;
            }
        }
        while (aux != NULL) {             
            NodoPila* n = aux;
            aux = aux->sig;
            n->sig = tope;
            tope = n;
        }
        return hallado;
    }

    void limpiar() {
        while (tope != NULL) {
            NodoPila* temp = tope;
            tope = tope->sig;
            delete temp;
        }
        usada = 0;
        bloques = 0;
    }

    void intercambiar(PilaMemoria &o) {
        NodoPila* t = tope; tope = o.tope; o.tope = t;
        int u = usada; usada = o.usada; o.usada = u;
        int b = bloques; bloques = o.bloques; o.bloques = b;
    }

    void guardar(ofstream &f) const { guardarRec(tope, f); }

    void mostrar() const {
        cout << "\n  ESTADO DE LA MEMORIA RAM\n  " << string(50, '=') << "\n";
        cout << "  Total : " << RAM_TOTAL << " MB\n";
        cout << "  Usada : " << usada << " MB\n";
        cout << "  Libre : " << memoriaLibre() << " MB\n";
        int pct = (usada * 100) / RAM_TOTAL;
        int llenos = (pct * 30) / 100;
        cout << "  Carga : [";
        for (int i = 0; i < 30; i++) cout << (i < llenos ? '#' : '.');
        cout << "] " << pct << "%\n";
        if (tope == NULL) { cout << "\n  (Pila vacia: no hay bloques asignados)\n"; return; }
        cout << "\n  " << left << setw(8) << "POS" << setw(10) << "PROCESO" << setw(12) << "TAMANO(MB)"
             << "RANGO (base-fin)\n  " << string(50, '-') << "\n";
        int corriente = usada;
        int pos = 1;
        for (NodoPila* a = tope; a != NULL; a = a->sig, pos++) {
            corriente -= a->tamano;
            cout << "  " << left << setw(8) << (pos == 1 ? "TOPE" : "") << setw(10) << a->idProceso
                 << setw(12) << a->tamano << corriente << " - " << (corriente + a->tamano - 1) << "\n";
        }
        cout << "  " << string(50, '-') << "\n  Bloques asignados: " << bloques << "\n";
    }
};


struct Sistema {
    ListaProcesos lista;
    ColaPrioridad cpu;
    PilaMemoria ram;
};


void registrarProceso(Sistema &S) {
    cout << "\n--- REGISTRAR PROCESO ---\n";
    int id;
    while (true) {
        id = leerEntero("ID del proceso (1-99999): ", 1, 99999);
        if (S.lista.buscarPorId(id) != NULL) cout << "  [!] Ya existe un proceso con ese ID.\n";
        else break;
    }
    Proceso p;
    p.id = id;
    p.nombre = leerNombre("Nombre del proceso: ");
    p.prioridad = leerEntero("Prioridad (1=Alta, 2=Media, 3=Baja): ", 1, 3);
    p.memoria = leerEntero("Memoria requerida en MB (1-1024): ", 1, RAM_TOTAL);
    p.estado = EST_LISTO;
    S.lista.insertarFinal(p);
    cout << "  [OK] Proceso registrado en estado LISTO.\n";
}

void eliminarProceso(Sistema &S) {
    cout << "\n--- ELIMINAR PROCESO ---\n";
    int id = leerEntero("ID del proceso a eliminar: ", 1, 99999);
    if (S.lista.buscarPorId(id) == NULL) { cout << "  [!] No existe un proceso con ese ID.\n"; return; }
    S.cpu.eliminar(id);
    S.ram.eliminarBloque(id);
    S.lista.eliminar(id);
    cout << "  [OK] Proceso eliminado (tambien de la cola de CPU y de la memoria).\n";
}

void buscarProceso(Sistema &S) {
    cout << "\n--- BUSCAR PROCESO ---\n";
    cout << "  1. Por ID\n  2. Por nombre (coincidencia parcial)\n";
    int op = leerEntero("Opcion: ", 1, 2);
    if (op == 1) {
        int id = leerEntero("ID: ", 1, 99999);
        NodoLista* n = S.lista.buscarPorId(id);
        if (n == NULL) cout << "  [!] No se encontro el proceso.\n";
        else { ListaProcesos::imprimirEncabezado(); ListaProcesos::imprimirFila(n->dato); }
    } else {
        string nom = leerNombre("Nombre o parte del nombre: ");
        if (S.lista.mostrarPorNombre(nom) == 0) cout << "  [!] Sin coincidencias.\n";
    }
}

void modificarPrioridad(Sistema &S) {
    cout << "\n--- MODIFICAR PRIORIDAD ---\n";
    int id = leerEntero("ID del proceso: ", 1, 99999);
    NodoLista* n = S.lista.buscarPorId(id);
    if (n == NULL) { cout << "  [!] No existe un proceso con ese ID.\n"; return; }
    if (n->dato.estado == EST_FINALIZADO) { cout << "  [!] El proceso ya finalizo.\n"; return; }
    cout << "  Prioridad actual: " << nombrePrioridad(n->dato.prioridad) << "\n";
    int nueva = leerEntero("Nueva prioridad (1=Alta, 2=Media, 3=Baja): ", 1, 3);
    n->dato.prioridad = nueva;
    if (S.cpu.contiene(id)) {                 
        S.cpu.eliminar(id);
        S.cpu.encolar(id, n->dato.nombre, nueva, n->dato.memoria);
        cout << "  [OK] Prioridad actualizada y proceso reubicado en la cola de CPU.\n";
    } else {
        cout << "  [OK] Prioridad actualizada.\n";
    }
}


bool transicionValida(int actual, int nuevo) {
    if (actual == EST_LISTO && nuevo == EST_BLOQUEADO) return true;
    if (actual == EST_BLOQUEADO && nuevo == EST_LISTO) return true;
    if ((actual == EST_LISTO || actual == EST_BLOQUEADO) && nuevo == EST_FINALIZADO) return true;
    return false;
}

void cambiarEstado(Sistema &S) {
    cout << "\n--- CAMBIAR ESTADO DE PROCESO ---\n";
    int id = leerEntero("ID del proceso: ", 1, 99999);
    NodoLista* n = S.lista.buscarPorId(id);
    if (n == NULL) { cout << "  [!] No existe un proceso con ese ID.\n"; return; }
    cout << "  Estado actual: " << nombreEstado(n->dato.estado) << "\n";
    cout << "  Nuevo estado: 1=LISTO  2=BLOQUEADO  3=FINALIZADO\n";
    int op = leerEntero("Opcion: ", 1, 3);
    int nuevo = (op == 1) ? EST_LISTO : (op == 2 ? EST_BLOQUEADO : EST_FINALIZADO);
    if (!transicionValida(n->dato.estado, nuevo)) {
        cout << "  [!] Transicion no permitida: " << nombreEstado(n->dato.estado)
             << " -> " << nombreEstado(nuevo) << ".\n";
        return;
    }
    if (nuevo == EST_BLOQUEADO || nuevo == EST_FINALIZADO) S.cpu.eliminar(id);
    if (nuevo == EST_FINALIZADO) S.ram.eliminarBloque(id);
    n->dato.estado = nuevo;
    cout << "  [OK] Estado actualizado a " << nombreEstado(nuevo) << ".\n";
}


void encolarProceso(Sistema &S) {
    cout << "\n--- ENCOLAR PROCESO EN CPU ---\n";
    int id = leerEntero("ID del proceso: ", 1, 99999);
    NodoLista* n = S.lista.buscarPorId(id);
    if (n == NULL) { cout << "  [!] No existe un proceso con ese ID.\n"; return; }
    if (n->dato.estado == EST_FINALIZADO) { cout << "  [!] El proceso ya finalizo.\n"; return; }
    if (S.cpu.contiene(id)) { cout << "  [!] El proceso ya esta en la cola.\n"; return; }
    if (S.ram.buscar(id) == NULL) {
        if (!S.ram.push(id, n->dato.memoria)) {
            n->dato.estado = EST_BLOQUEADO;
            cout << "  [!] Memoria insuficiente (libre: " << S.ram.memoriaLibre()
                 << " MB). El proceso pasa a BLOQUEADO.\n";
            return;
        }
        cout << "  [i] Se asignaron " << n->dato.memoria << " MB (push en la pila de memoria).\n";
    }
    n->dato.estado = EST_LISTO;
    S.cpu.encolar(id, n->dato.nombre, n->dato.prioridad, n->dato.memoria);
    cout << "  [OK] Proceso encolado segun su prioridad.\n";
}

void ejecutarUno(Sistema &S) {
    int id, prio, mem;
    string nombre;
    if (!S.cpu.desencolar(id, nombre, prio, mem)) {
        cout << "  [!] No hay procesos en la cola de CPU.\n";
        return;
    }
    NodoLista* n = S.lista.buscarPorId(id);
    if (n != NULL) n->dato.estado = EST_EJECUCION;
    cout << "\n  >> EJECUTANDO [" << id << "] " << nombre << " (prioridad " << nombrePrioridad(prio) << ")\n  [";
    for (int t = 0; t < 20; t++) cout << '#';
    cout << "] 100%\n";
    if (n != NULL) n->dato.estado = EST_FINALIZADO;
    S.ram.eliminarBloque(id);
    cout << "  >> Proceso FINALIZADO. Se liberaron " << mem << " MB de memoria.\n";
}

void ejecutarTodos(Sistema &S) {
    if (S.cpu.estaVacia()) { cout << "  [!] No hay procesos en la cola de CPU.\n"; return; }
    while (!S.cpu.estaVacia()) ejecutarUno(S);
    cout << "\n  [OK] La cola de CPU quedo vacia.\n";
}


void asignarMemoria(Sistema &S) {
    cout << "\n--- ASIGNAR MEMORIA (PUSH) ---\n";
    int id = leerEntero("ID del proceso: ", 1, 99999);
    NodoLista* n = S.lista.buscarPorId(id);
    if (n == NULL) { cout << "  [!] No existe un proceso con ese ID.\n"; return; }
    if (n->dato.estado == EST_FINALIZADO) { cout << "  [!] El proceso ya finalizo.\n"; return; }
    if (S.ram.buscar(id) != NULL) { cout << "  [!] El proceso ya tiene memoria asignada.\n"; return; }
    if (!S.ram.push(id, n->dato.memoria)) {
        cout << "  [!] Memoria insuficiente: requiere " << n->dato.memoria << " MB y hay "
             << S.ram.memoriaLibre() << " MB libres.\n";
        return;
    }
    cout << "  [OK] Bloque de " << n->dato.memoria << " MB apilado para el proceso " << id << ".\n";
}

void liberarMemoria(Sistema &S) {
    cout << "\n--- LIBERAR MEMORIA (POP) ---\n";
    int id, tam;
    if (!S.ram.pop(id, tam)) { cout << "  [!] La pila de memoria esta vacia.\n"; return; }
    cout << "  [OK] Se libero el bloque del proceso " << id << " (" << tam << " MB).\n";
    NodoLista* n = S.lista.buscarPorId(id);
    if (n != NULL && n->dato.estado != EST_FINALIZADO) {
        n->dato.estado = EST_BLOQUEADO;
        S.cpu.eliminar(id);
        cout << "  [i] El proceso " << id << " perdio su memoria: pasa a BLOQUEADO.\n";
    }
}


bool guardarEstado(const Sistema &S) {
    ofstream f(ARCHIVO);
    if (!f) return false;
    f << "RAM_TOTAL " << RAM_TOTAL << "\n";
    f << "PROCESOS " << S.lista.obtenerCantidad() << "\n";
    for (NodoLista* a = S.lista.primero(); a != NULL; a = a->sig)
        f << a->dato.id << "|" << a->dato.nombre << "|" << a->dato.prioridad << "|"
          << a->dato.estado << "|" << a->dato.memoria << "\n";
    f << "COLA " << S.cpu.obtenerCantidad() << "\n";
    for (NodoCola* c = S.cpu.primero(); c != NULL; c = c->sig)
        f << c->id << "\n";
    f << "PILA " << S.ram.obtenerBloques() << "\n";
    S.ram.guardar(f);
    f.close();
    return true;
}

bool leerEncabezado(ifstream &f, const char* etiqueta, int &n) {
    string linea;
    if (!getline(f, linea)) return false;
    string p[3];
    int k = dividir(recortar(linea), ' ', p, 3);
    if (k != 2 || p[0] != etiqueta || !esNumero(p[1])) return false;
    n = atoi(p[1].c_str());
    return true;
}

bool cargarEstado(Sistema &S) {
    ifstream f(ARCHIVO);
    if (!f) { cout << "  [!] No existe el archivo " << ARCHIVO << ".\n"; return false; }
    Sistema nuevo;                  
    int total, n;
    if (!leerEncabezado(f, "RAM_TOTAL", total) || total != RAM_TOTAL) {
        cout << "  [!] Archivo invalido (encabezado RAM_TOTAL).\n"; return false;
    }
    if (!leerEncabezado(f, "PROCESOS", n)) {
        cout << "  [!] Archivo invalido (seccion PROCESOS).\n"; return false;
    }
    for (int i = 0; i < n; i++) {
        string linea, p[6];
        if (!getline(f, linea) || dividir(recortar(linea), '|', p, 6) != 5 ||
            !esNumero(p[0]) || !esNumero(p[2]) || !esNumero(p[3]) || !esNumero(p[4])) {
            cout << "  [!] Archivo invalido (proceso #" << (i + 1) << ").\n"; return false;
        }
        Proceso pr;
        pr.id = atoi(p[0].c_str());
        pr.nombre = p[1];
        pr.prioridad = atoi(p[2].c_str());
        pr.estado = atoi(p[3].c_str());
        pr.memoria = atoi(p[4].c_str());
        if (pr.id < 1 || pr.nombre.empty() || pr.prioridad < 1 || pr.prioridad > 3 ||
            pr.estado < 0 || pr.estado > 3 || pr.memoria < 1 || pr.memoria > RAM_TOTAL ||
            nuevo.lista.buscarPorId(pr.id) != NULL) {
            cout << "  [!] Datos inconsistentes en el proceso #" << (i + 1) << ".\n"; return false;
        }
        nuevo.lista.insertarFinal(pr);
    }
    if (!leerEncabezado(f, "COLA", n)) {
        cout << "  [!] Archivo invalido (seccion COLA).\n"; return false;
    }
    for (int i = 0; i < n; i++) {
        string linea;
        if (!getline(f, linea) || !esNumero(recortar(linea))) {
            cout << "  [!] Archivo invalido (cola, linea " << (i + 1) << ").\n"; return false;
        }
        int id = atoi(recortar(linea).c_str());
        NodoLista* pr = nuevo.lista.buscarPorId(id);
        if (pr == NULL || nuevo.cpu.contiene(id)) {
            cout << "  [!] La cola referencia un proceso inexistente o repetido.\n"; return false;
        }
        nuevo.cpu.encolar(id, pr->dato.nombre, pr->dato.prioridad, pr->dato.memoria);
    }
    if (!leerEncabezado(f, "PILA", n)) {
        cout << "  [!] Archivo invalido (seccion PILA).\n"; return false;
    }
    for (int i = 0; i < n; i++) {
        string linea, p[3];
        if (!getline(f, linea) || dividir(recortar(linea), '|', p, 3) != 2 ||
            !esNumero(p[0]) || !esNumero(p[1])) {
            cout << "  [!] Archivo invalido (pila, linea " << (i + 1) << ").\n"; return false;
        }
        int id = atoi(p[0].c_str());
        int tam = atoi(p[1].c_str());
        if (nuevo.lista.buscarPorId(id) == NULL || !nuevo.ram.push(id, tam)) {
            cout << "  [!] Bloque de memoria inconsistente (linea " << (i + 1) << ").\n"; return false;
        }
    }
    f.close();
    S.lista.intercambiar(nuevo.lista);      
    S.cpu.intercambiar(nuevo.cpu);
    S.ram.intercambiar(nuevo.ram);
    return true;
}


void menuProcesos(Sistema &S) {
    int op;
    do {
        cout << "\n=========== GESTOR DE PROCESOS ===========\n"
             << " 1. Registrar proceso\n 2. Eliminar proceso\n 3. Buscar proceso\n"
             << " 4. Modificar prioridad\n 5. Cambiar estado\n 6. Listar procesos\n 0. Volver\n";
        op = leerEntero("Opcion: ", 0, 6);
        switch (op) {
            case 1: registrarProceso(S); break;
            case 2: eliminarProceso(S); break;
            case 3: buscarProceso(S); break;
            case 4: modificarPrioridad(S); break;
            case 5: cambiarEstado(S); break;
            case 6: S.lista.mostrar(); break;
        }
        if (op != 0) pausar();
    } while (op != 0);
}

void menuCPU(Sistema &S) {
    int op;
    do {
        cout << "\n=========== PLANIFICADOR DE CPU ===========\n"
             << " 1. Encolar proceso\n 2. Ejecutar siguiente proceso\n"
             << " 3. Ejecutar todos\n 4. Ver cola actual\n 0. Volver\n";
        op = leerEntero("Opcion: ", 0, 4);
        switch (op) {
            case 1: encolarProceso(S); break;
            case 2: ejecutarUno(S); break;
            case 3: ejecutarTodos(S); break;
            case 4: S.cpu.mostrar(); break;
        }
        if (op != 0) pausar();
    } while (op != 0);
}

void menuMemoria(Sistema &S) {
    int op;
    do {
        cout << "\n=========== GESTOR DE MEMORIA ===========\n"
             << " 1. Asignar memoria a proceso (push)\n 2. Liberar ultimo bloque (pop)\n"
             << " 3. Ver estado de la memoria\n 0. Volver\n";
        op = leerEntero("Opcion: ", 0, 3);
        switch (op) {
            case 1: asignarMemoria(S); break;
            case 2: liberarMemoria(S); break;
            case 3: S.ram.mostrar(); break;
        }
        if (op != 0) pausar();
    } while (op != 0);
}

void menuPersistencia(Sistema &S) {
    int op;
    do {
        cout << "\n=========== PERSISTENCIA ===========\n"
             << " 1. Guardar estado en " << ARCHIVO << "\n 2. Cargar estado desde archivo\n 0. Volver\n";
        op = leerEntero("Opcion: ", 0, 2);
        if (op == 1) {
            if (guardarEstado(S)) cout << "  [OK] Estado guardado correctamente.\n";
            else cout << "  [!] No se pudo escribir el archivo.\n";
        } else if (op == 2) {
            if (confirmar("Se reemplazara el estado actual. Continuar? (S/N): ")) {
                if (cargarEstado(S)) cout << "  [OK] Estado cargado correctamente.\n";
            }
        }
        if (op != 0) pausar();
    } while (op != 0);
}

int main() {
    Sistema S;
    cout << "************************************************\n"
         << "*   SISTEMA DE GESTION DE PROCESOS (Simulador) *\n"
         << "*   Estructura de Datos - U. Continental       *\n"
         << "************************************************\n";

    ifstream prueba(ARCHIVO);
    bool existe = prueba.good();
    prueba.close();
    if (existe) {
        cout << "\nSe detecto un estado guardado previamente.\n";
        if (confirmar("Desea cargarlo? (S/N): ")) {
            if (cargarEstado(S)) cout << "  [OK] Estado cargado.\n";
        }
    }

    int op;
    do {
        cout << "\n=============== MENU PRINCIPAL ===============\n"
             << " 1. Gestor de Procesos (lista enlazada)\n"
             << " 2. Planificador de CPU (cola de prioridad)\n"
             << " 3. Gestor de Memoria (pila)\n"
             << " 4. Persistencia (guardar / cargar)\n"
             << " 0. Salir\n";
        op = leerEntero("Opcion: ", 0, 4);
        switch (op) {
            case 1: menuProcesos(S); break;
            case 2: menuCPU(S); break;
            case 3: menuMemoria(S); break;
            case 4: menuPersistencia(S); break;
        }
    } while (op != 0);

    if (confirmar("Guardar el estado antes de salir? (S/N): ")) {
        if (guardarEstado(S)) cout << "  [OK] Estado guardado.\n";
        else cout << "  [!] No se pudo guardar.\n";
    }
    cout << "\nHasta pronto.\n";
    return 0;
}
