// HeapEspera.h - max-heap (en arreglo dinamico) de la lista de espera de UN vuelo.
// Prioridad: mayor categoria primero (Premium > Frecuente > Regular);
// en empate, menor ordenLlegada (el que llego antes).
// El heap NO es dueno de las Reserva; solo guarda punteros.
#ifndef HEAP_ESPERA_H
#define HEAP_ESPERA_H

#include "modelo.h"

class HeapEspera {
private:
    Reserva** a;
    int n;
    int cap;

    // true si x tiene MAS prioridad que y
    static bool mayor(const Reserva* x, const Reserva* y) {
        if (x->categoria != y->categoria) return x->categoria > y->categoria;
        return x->ordenLlegada < y->ordenLlegada;
    }
    static void intercambiar(Reserva** arr, int i, int j) {
        Reserva* t = arr[i]; arr[i] = arr[j]; arr[j] = t;
    }
    static void subir(Reserva** arr, int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (!mayor(arr[i], arr[p])) break;
            intercambiar(arr, i, p);
            i = p;
        }
    }
    static void bajar(Reserva** arr, int tam, int i) {
        while (true) {
            int izq = 2 * i + 1, der = izq + 1, m = i;
            if (izq < tam && mayor(arr[izq], arr[m])) m = izq;
            if (der < tam && mayor(arr[der], arr[m])) m = der;
            if (m == i) break;
            intercambiar(arr, i, m);
            i = m;
        }
    }
    void crecer() {
        int nueva = cap * 2;
        Reserva** b = new Reserva*[nueva];
        for (int i = 0; i < n; i++) b[i] = a[i];
        delete[] a;
        a = b;
        cap = nueva;
    }

public:
    HeapEspera() : a(new Reserva*[8]), n(0), cap(8) {}
    ~HeapEspera() { delete[] a; }
    HeapEspera(const HeapEspera&) = delete;
    HeapEspera& operator=(const HeapEspera&) = delete;

    int tamanio() const { return n; }
    bool vacio() const { return n == 0; }
    Reserva* maximo() const { return n > 0 ? a[0] : nullptr; }

    // O(log m)
    void insertar(Reserva* r) {
        if (n == cap) crecer();
        a[n] = r;
        subir(a, n);
        n++;
    }

    // O(log m). nullptr si esta vacio.
    Reserva* extraerMax() {
        if (n == 0) return nullptr;
        Reserva* raiz = a[0];
        n--;
        if (n > 0) {
            a[0] = a[n];
            bajar(a, n, 0);
        }
        return raiz;
    }

    // Retira r desde una posicion cualquiera. Busqueda secuencial O(m) + reordenar O(log m).
    bool eliminar(const Reserva* r) {
        int i = 0;
        while (i < n && a[i] != r) i++;
        if (i == n) return false;
        n--;
        if (i != n) {
            a[i] = a[n];       // el ultimo ocupa el hueco
            subir(a, i);       // puede tener mas prioridad que su nuevo padre...
            bajar(a, n, i);    // ...o menos que sus nuevos hijos
        }
        return true;
    }

    bool contiene(const Reserva* r) const {
        for (int i = 0; i < n; i++) if (a[i] == r) return true;
        return false;
    }

    // Copia en "salida" hasta "max" reservas en orden de prioridad, SIN alterar el heap
    // (trabaja sobre una copia del arreglo). O(m log m).
    int copiaOrdenada(Reserva** salida, int max) const {
        Reserva** tmp = new Reserva*[n > 0 ? n : 1];
        for (int i = 0; i < n; i++) tmp[i] = a[i];
        int m = n, k = 0;
        while (m > 0 && k < max) {
            salida[k++] = tmp[0];
            m--;
            tmp[0] = tmp[m];
            bajar(tmp, m, 0);
        }
        delete[] tmp;
        return k;
    }

    // Para pruebas: ningun hijo tiene mas prioridad que su padre.
    bool verificar() const {
        for (int i = 1; i < n; i++)
            if (mayor(a[i], a[(i - 1) / 2])) return false;
        return true;
    }
};

#endif
