#ifndef MODELO_H
#define MODELO_H

#include <string>
using namespace std;

enum Clase     { ECONOMICA, PREMIUM_CLASE, EJECUTIVA };
enum Categoria { REGULAR = 1, FRECUENTE = 2, PREMIUM = 3 };
enum Estado    { CONFIRMADA, PENDIENTE, EN_ESPERA, CHECKIN, CANCELADA };

struct Reserva {
    string idReserva;
    string dni;
    string pasajero;
    string codVuelo;
    string origen;
    string destino;
    string fecha;          // formato ISO "AAAA-MM-DD": orden alfabético = orden cronológico
    string hora;           // formato "HH:MM"
    Clase clase;
    Categoria categoria;
    double precio;
    Estado estado;
    int ordenLlegada = 0;          // para desempatar en el heap de espera
    Reserva* siguiente = nullptr;  // encadenamiento de la tabla hash
};

struct Vuelo {
    string codVuelo;
    string origen;
    string destino;
    string fecha;
    string hora;
    int capacidad;
    int asientosOcupados = 0;      // para saber si el vuelo está lleno sin recorrer reservas
};

#endif
