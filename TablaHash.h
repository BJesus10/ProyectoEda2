// TablaHash.h - tabla hash con encadenamiento separado, indexada por idReserva.
// La tabla es DUENA de las Reserva: las libera en eliminar() y en el destructor.
// (El arbol y los heaps solo guardan punteros y nunca hacen delete.)
#ifndef TABLAHASH_H
#define TABLAHASH_H

#include "modelo.h"
#include "hash_util.h"

class TablaHash {
private:
    Reserva** tabla;   // cada posicion es el inicio de una cadena
    int tam;
    int cant;

public:
    explicit TablaHash(int t) : tabla(new Reserva*[t]), tam(t), cant(0) {
        for (int i = 0; i < tam; i++) tabla[i] = nullptr;
    }

    ~TablaHash() {
        for (int i = 0; i < tam; i++) {
            Reserva* p = tabla[i];
            while (p != nullptr) {
                Reserva* sig = p->siguiente;
                delete p;
                p = sig;
            }
        }
        delete[] tabla;
    }

    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;

    int posicion(const string& id) const { return hashId(id, tam); }

    // Inserta al inicio de la cadena: O(1). Quien llama garantiza que el id no existe.
    void insertar(Reserva* r) {
        int idx = posicion(r->idReserva);
        r->siguiente = tabla[idx];
        tabla[idx] = r;
        cant++;
    }

    // nullptr si no existe. O(1 + alfa) promedio.
    Reserva* buscar(const string& id) const {
        for (Reserva* p = tabla[posicion(id)]; p != nullptr; p = p->siguiente)
            if (p->idReserva == id) return p;
        return nullptr;
    }

    // Quita la reserva de la cadena Y la libera (delete). true si existia.
    bool eliminar(const string& id) {
        int idx = posicion(id);
        Reserva* actual = tabla[idx];
        Reserva* anterior = nullptr;
        while (actual != nullptr) {
            if (actual->idReserva == id) {
                if (anterior == nullptr) tabla[idx] = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                delete actual;
                cant--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    int cantidad() const { return cant; }
    int tamanio() const { return tam; }
    double factorCarga() const { return (double)cant / tam; }

    int longitudCadena(int pos) const {
        int n = 0;
        for (Reserva* p = tabla[pos]; p != nullptr; p = p->siguiente) n++;
        return n;
    }
    int cadenaMasLarga() const {
        int max = 0;
        for (int i = 0; i < tam; i++) {
            int l = longitudCadena(i);
            if (l > max) max = l;
        }
        return max;
    }
};

#endif
