// ArbolAVL.h
// Arbol AVL de reservas ordenado por la clave compuesta
// (fecha, hora, idReserva).
//
// IMPORTANTE (propiedad de memoria):
//   El arbol guarda PUNTEROS a Reserva; no es dueno de las reservas.
//   La tabla hash es la duena: crea la Reserva con new y la borra con delete.
//   El destructor del arbol solo libera sus propios nodos (NodoAVL).
//   Para una eliminacion fisica: primero arbol.eliminar(r), luego hash.eliminar(id).
#ifndef ARBOL_AVL_H
#define ARBOL_AVL_H

#include <string>
#include "modelo.h"

struct NodoAVL {
    Reserva* dato;
    NodoAVL* izq;
    NodoAVL* der;
    int      altura;   // hoja = 1

    NodoAVL(Reserva* r) : dato(r), izq(nullptr), der(nullptr), altura(1) {}
};

// Funcion que se aplica a cada reserva en un recorrido (p. ej. estadisticas).
// "contexto" permite pasar cualquier estructura de acumuladores.
typedef void (*VisitaReserva)(Reserva* r, void* contexto);

class ArbolAVL {
public:
    ArbolAVL();
    ~ArbolAVL();

    // Inserta la reserva. Devuelve false si ya existia esa clave.
    bool insertar(Reserva* r);

    // Elimina el nodo cuya clave coincide con la de r.
    // Devuelve false si no estaba. NO hace delete de la Reserva.
    bool eliminar(Reserva* r);

    // Busca por clave compuesta completa. nullptr si no existe.
    Reserva* buscar(const std::string& fecha, const std::string& hora,
                    const std::string& id) const;

    // Funcionalidad 4: copia en "salida" las primeras "max" reservas
    // en orden cronologico. Devuelve cuantas copio.
    int primerasN(Reserva** salida, int max) const;

    // Funcionalidad 5: cuenta TODAS las reservas con fechaIni <= fecha <= fechaFin
    // (inclusive) y copia en "muestra" hasta "maxMuestra" de ellas, en orden.
    // Descarta las ramas fuera del rango.
    int rangoFechas(const std::string& fechaIni, const std::string& fechaFin,
                    Reserva** muestra, int maxMuestra, int& enMuestra) const;

    // Recorrido en orden completo (para estadisticas, funcionalidad 10).
    void recorrerEnOrden(VisitaReserva f, void* contexto) const;

    int  altura() const;
    int  tamanio() const;
    bool vacio() const;

    // Para pruebas: verifica orden BST, alturas y factor de balance en todo el arbol.
    bool verificar() const;

    // Imprime el arbol de lado (solo para arboles pequenos).
    void imprimir() const;

private:
    NodoAVL* raiz;
    int      cantidad;

    // Comparacion de claves: <0 si a<b, 0 si iguales, >0 si a>b.
    static int comparar(const Reserva* a, const Reserva* b);
    static int comparar(const std::string& fa, const std::string& ha, const std::string& ia,
                        const Reserva* b);

    static int  alt(NodoAVL* n);
    static int  balance(NodoAVL* n);
    static void actualizarAltura(NodoAVL* n);
    static NodoAVL* rotarDerecha(NodoAVL* y);
    static NodoAVL* rotarIzquierda(NodoAVL* x);
    static NodoAVL* rebalancear(NodoAVL* n);

    NodoAVL* insertarRec(NodoAVL* n, Reserva* r, bool& insertado);
    NodoAVL* eliminarRec(NodoAVL* n, Reserva* r, bool& eliminado);
    static NodoAVL* minimo(NodoAVL* n);

    static void primerasNRec(NodoAVL* n, Reserva** salida, int max, int& k);
    static void rangoRec(NodoAVL* n, const std::string& ini, const std::string& fin,
                         Reserva** muestra, int maxMuestra, int& enMuestra, int& total);
    static void enOrdenRec(NodoAVL* n, VisitaReserva f, void* contexto);
    static void liberar(NodoAVL* n);
    static bool verificarRec(NodoAVL* n, const Reserva* minR, const Reserva* maxR, int& h);
    static void imprimirRec(NodoAVL* n, int nivel);
};

#endif
