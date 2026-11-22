#ifndef TABLAHASH_H
#define TABLAHASH_H

#include "modelo.h"

class TablaHash {
private:
    Reserva** tabla;   
    int tam;           
    int cantidad;     

    int hash(const string& clave) const {
        unsigned long h = 5381;
        for (size_t i = 0; i < clave.size(); i++) {
            h = h * 33 + (unsigned char)clave[i];
        }
        return (int)(h % tam);
    }

public:
    TablaHash(int tam) {
        this->tam = tam;
        cantidad = 0;
        tabla = new Reserva*[tam];
        for (int i = 0; i < tam; i++) tabla[i] = nullptr;
    }

    ~TablaHash() {
        delete[] tabla;
    }

    void insertar(Reserva* r) {
        int idx = hash(r->idReserva);
        r->siguiente = tabla[idx];   
        tabla[idx] = r;              
        cantidad++;
    }

    Reserva* buscar(const string& idReserva) const {
        Reserva* actual = tabla[hash(idReserva)];
        while (actual != nullptr) {
            if (actual->idReserva == idReserva) return actual;
            actual = actual->siguiente;
        }
        return nullptr;
    }

    bool eliminar(const string& idReserva) {
        int idx = hash(idReserva);
        Reserva* actual = tabla[idx];
        Reserva* anterior = nullptr;

        while (actual != nullptr) {
            if (actual->idReserva == idReserva) {
                if (anterior == nullptr) tabla[idx] = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                actual->siguiente = nullptr;
                cantidad--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    double factorCarga() const {
        return (double)cantidad / tam;
    }

    int cadenaMasLarga() const {
        int max = 0;
        for (int i = 0; i < tam; i++) {
            int largo = 0;
            for (Reserva* p = tabla[i]; p != nullptr; p = p->siguiente) largo++;
            if (largo > max) max = largo;
        }
        return max;
    }
};

#endif
