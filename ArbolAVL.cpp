// ArbolAVL.cpp
#include "ArbolAVL.h"
#include <iostream>

using std::string;

ArbolAVL::ArbolAVL() : raiz(nullptr), cantidad(0) {}

ArbolAVL::~ArbolAVL() {
    liberar(raiz);   // solo nodos; las Reserva las libera la hash
    raiz = nullptr;
}

void ArbolAVL::liberar(NodoAVL* n) {
    if (n == nullptr) return;
    liberar(n->izq);
    liberar(n->der);
    delete n;
}

// ---------------------------------------------------------------
// Clave compuesta: fecha, luego hora, luego idReserva.
// Como fecha es AAAA-MM-DD y hora es HH:MM (con ceros a la izquierda),
// la comparacion lexicografica de string coincide con el orden cronologico.
// idReserva desempata reservas con la misma fecha y hora, asi la clave es unica.
// ---------------------------------------------------------------
int ArbolAVL::comparar(const string& fa, const string& ha, const string& ia,
                       const Reserva* b) {
    int c = fa.compare(b->fecha);
    if (c != 0) return c;
    c = ha.compare(b->hora);
    if (c != 0) return c;
    return ia.compare(b->idReserva);
}

int ArbolAVL::comparar(const Reserva* a, const Reserva* b) {
    return comparar(a->fecha, a->hora, a->idReserva, b);
}

// ---------------------------------------------------------------
// Utilidades de altura y balance
// ---------------------------------------------------------------
int ArbolAVL::alt(NodoAVL* n) { return n ? n->altura : 0; }

int ArbolAVL::balance(NodoAVL* n) { return n ? alt(n->izq) - alt(n->der) : 0; }

void ArbolAVL::actualizarAltura(NodoAVL* n) {
    int hi = alt(n->izq), hd = alt(n->der);
    n->altura = 1 + (hi > hd ? hi : hd);
}

// Rotacion simple a la derecha (caso LL):
//   antes:  y tiene hijo izq x;  x tiene hijos A (izq) y B (der);  y tiene hijo der C
//   despues: x es la raiz;  A queda a la izq de x;  y a la der de x;  B y C cuelgan de y
NodoAVL* ArbolAVL::rotarDerecha(NodoAVL* y) {
    NodoAVL* x = y->izq;
    y->izq = x->der;
    x->der = y;
    actualizarAltura(y);
    actualizarAltura(x);
    return x;
}

NodoAVL* ArbolAVL::rotarIzquierda(NodoAVL* x) {
    NodoAVL* y = x->der;
    x->der = y->izq;
    y->izq = x;
    actualizarAltura(x);
    actualizarAltura(y);
    return y;
}

// Corrige los 4 casos (LL, LR, RR, RL). Sirve para insercion y eliminacion.
NodoAVL* ArbolAVL::rebalancear(NodoAVL* n) {
    actualizarAltura(n);
    int fb = balance(n);

    if (fb > 1) {                         // cargado a la izquierda
        if (balance(n->izq) < 0)          // caso LR
            n->izq = rotarIzquierda(n->izq);
        return rotarDerecha(n);           // caso LL
    }
    if (fb < -1) {                        // cargado a la derecha
        if (balance(n->der) > 0)          // caso RL
            n->der = rotarDerecha(n->der);
        return rotarIzquierda(n);         // caso RR
    }
    return n;
}

// ---------------------------------------------------------------
// Insercion
// ---------------------------------------------------------------
bool ArbolAVL::insertar(Reserva* r) {
    if (r == nullptr) return false;
    bool insertado = false;
    raiz = insertarRec(raiz, r, insertado);
    if (insertado) cantidad++;
    return insertado;
}

NodoAVL* ArbolAVL::insertarRec(NodoAVL* n, Reserva* r, bool& insertado) {
    if (n == nullptr) {
        insertado = true;
        return new NodoAVL(r);
    }
    int c = comparar(r, n->dato);
    if (c < 0)      n->izq = insertarRec(n->izq, r, insertado);
    else if (c > 0) n->der = insertarRec(n->der, r, insertado);
    else            return n;             // clave duplicada: no se inserta

    return rebalancear(n);
}

// ---------------------------------------------------------------
// Eliminacion
// ---------------------------------------------------------------
bool ArbolAVL::eliminar(Reserva* r) {
    if (r == nullptr) return false;
    bool eliminado = false;
    raiz = eliminarRec(raiz, r, eliminado);
    if (eliminado) cantidad--;
    return eliminado;
}

NodoAVL* ArbolAVL::minimo(NodoAVL* n) {
    while (n->izq != nullptr) n = n->izq;
    return n;
}

NodoAVL* ArbolAVL::eliminarRec(NodoAVL* n, Reserva* r, bool& eliminado) {
    if (n == nullptr) return nullptr;

    int c = comparar(r, n->dato);
    if (c < 0) {
        n->izq = eliminarRec(n->izq, r, eliminado);
    } else if (c > 0) {
        n->der = eliminarRec(n->der, r, eliminado);
    } else {
        eliminado = true;
        if (n->izq == nullptr || n->der == nullptr) {
            // 0 o 1 hijo
            NodoAVL* hijo = n->izq ? n->izq : n->der;
            delete n;
            return hijo;            // puede ser nullptr
        }
        // 2 hijos: se copia el sucesor (minimo del subarbol derecho)
        // y luego se elimina el sucesor de ese subarbol.
        NodoAVL* suc = minimo(n->der);
        n->dato = suc->dato;
        bool aux = false;
        n->der = eliminarRec(n->der, suc->dato, aux);
    }
    return rebalancear(n);
}

// ---------------------------------------------------------------
// Busqueda exacta
// ---------------------------------------------------------------
Reserva* ArbolAVL::buscar(const string& fecha, const string& hora,
                          const string& id) const {
    NodoAVL* n = raiz;
    while (n != nullptr) {
        int c = comparar(fecha, hora, id, n->dato);
        if (c == 0) return n->dato;
        n = (c < 0) ? n->izq : n->der;
    }
    return nullptr;
}

// ---------------------------------------------------------------
// Funcionalidad 4: primeras N en orden (corta apenas llena el arreglo)
// ---------------------------------------------------------------
int ArbolAVL::primerasN(Reserva** salida, int max) const {
    int k = 0;
    primerasNRec(raiz, salida, max, k);
    return k;
}

void ArbolAVL::primerasNRec(NodoAVL* n, Reserva** salida, int max, int& k) {
    if (n == nullptr || k >= max) return;
    primerasNRec(n->izq, salida, max, k);
    if (k < max) salida[k++] = n->dato;
    primerasNRec(n->der, salida, max, k);
}

// ---------------------------------------------------------------
// Funcionalidad 5: rango de fechas [ini, fin] inclusive.
// Poda:
//   - si fecha del nodo < ini, todo su subarbol izquierdo tambien es < ini
//     -> solo se baja a la derecha.
//   - si fecha del nodo > fin, todo su subarbol derecho tambien es > fin
//     -> solo se baja a la izquierda.
// Solo se compara la fecha, porque el rango es por dias completos.
// Costo: O(log n + k), k = reservas dentro del rango.
// ---------------------------------------------------------------
int ArbolAVL::rangoFechas(const string& fechaIni, const string& fechaFin,
                          Reserva** muestra, int maxMuestra, int& enMuestra) const {
    int total = 0;
    enMuestra = 0;
    if (fechaIni.compare(fechaFin) > 0) return 0;   // rango invalido
    rangoRec(raiz, fechaIni, fechaFin, muestra, maxMuestra, enMuestra, total);
    return total;
}

void ArbolAVL::rangoRec(NodoAVL* n, const string& ini, const string& fin,
                        Reserva** muestra, int maxMuestra, int& enMuestra, int& total) {
    if (n == nullptr) return;
    const string& f = n->dato->fecha;

    if (f.compare(ini) < 0) {           // nodo antes del rango
        rangoRec(n->der, ini, fin, muestra, maxMuestra, enMuestra, total);
        return;
    }
    if (f.compare(fin) > 0) {           // nodo despues del rango
        rangoRec(n->izq, ini, fin, muestra, maxMuestra, enMuestra, total);
        return;
    }
    // nodo dentro del rango: en orden para que la muestra salga cronologica
    rangoRec(n->izq, ini, fin, muestra, maxMuestra, enMuestra, total);
    total++;
    if (muestra != nullptr && enMuestra < maxMuestra) muestra[enMuestra++] = n->dato;
    rangoRec(n->der, ini, fin, muestra, maxMuestra, enMuestra, total);
}

// ---------------------------------------------------------------
// Recorrido completo (estadisticas)
// ---------------------------------------------------------------
void ArbolAVL::recorrerEnOrden(VisitaReserva f, void* contexto) const {
    enOrdenRec(raiz, f, contexto);
}

void ArbolAVL::enOrdenRec(NodoAVL* n, VisitaReserva f, void* contexto) {
    if (n == nullptr) return;
    enOrdenRec(n->izq, f, contexto);
    f(n->dato, contexto);
    enOrdenRec(n->der, f, contexto);
}

// ---------------------------------------------------------------
int  ArbolAVL::altura()  const { return alt(raiz); }
int  ArbolAVL::tamanio() const { return cantidad; }
bool ArbolAVL::vacio()   const { return raiz == nullptr; }

// ---------------------------------------------------------------
// Verificacion (pruebas): orden, altura almacenada y |balance| <= 1
// ---------------------------------------------------------------
bool ArbolAVL::verificar() const {
    int h = 0;
    return verificarRec(raiz, nullptr, nullptr, h);
}

bool ArbolAVL::verificarRec(NodoAVL* n, const Reserva* minR, const Reserva* maxR, int& h) {
    if (n == nullptr) { h = 0; return true; }
    if (minR && comparar(n->dato, minR) <= 0) return false;
    if (maxR && comparar(n->dato, maxR) >= 0) return false;
    int hi, hd;
    if (!verificarRec(n->izq, minR, n->dato, hi)) return false;
    if (!verificarRec(n->der, n->dato, maxR, hd)) return false;
    h = 1 + (hi > hd ? hi : hd);
    if (n->altura != h) return false;
    int fb = hi - hd;
    return fb >= -1 && fb <= 1;
}

void ArbolAVL::imprimir() const { imprimirRec(raiz, 0); }

void ArbolAVL::imprimirRec(NodoAVL* n, int nivel) {
    if (n == nullptr) return;
    imprimirRec(n->der, nivel + 1);
    for (int i = 0; i < nivel; i++) std::cout << "        ";
    std::cout << n->dato->fecha << " " << n->dato->hora << " "
              << n->dato->idReserva << " (h=" << n->altura << ")\n";
    imprimirRec(n->izq, nivel + 1);
}
