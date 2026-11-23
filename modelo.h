// modelo.h - tipos comunes: Reserva, Vuelo y sus enumeraciones.
#ifndef MODELO_H
#define MODELO_H

#include <string>
using namespace std;

enum Clase     { ECONOMICA, PREMIUM_CLASE, EJECUTIVA };
enum Categoria { REGULAR = 1, FRECUENTE = 2, PREMIUM = 3 };   // mayor valor = mayor prioridad
enum Estado    { CONFIRMADA, PENDIENTE, EN_ESPERA, CHECKIN, CANCELADA };

struct Reserva {
    string idReserva;
    string dni;
    string pasajero;
    string codigoVuelo;
    string origen;
    string destino;
    string fecha;                 // AAAA-MM-DD: el orden alfabetico = orden cronologico
    string hora;                  // HH:MM
    Clase clase = ECONOMICA;
    Categoria categoria = REGULAR;
    double precio = 0.0;
    Estado estado = PENDIENTE;
    int ordenLlegada = 0;         // desempate en el heap de espera
    Reserva* siguiente = nullptr; // encadenamiento de la tabla hash
};

struct Vuelo {
    string codigoVuelo;
    string origen;
    string destino;
    string fecha;
    string hora;
    int capacidad = 0;
    int asientosOcupados = 0;     // reservas Confirmada, Pendiente o CheckIn
};

// ---- conversiones texto <-> enum (formato de los archivos .csv) ----
inline const char* textoEstado(Estado e) {
    switch (e) {
        case CONFIRMADA: return "Confirmada";
        case PENDIENTE:  return "Pendiente";
        case EN_ESPERA:  return "EnEspera";
        case CHECKIN:    return "CheckIn";
        default:         return "Cancelada";
    }
}
inline bool estadoDesdeTexto(const string& s, Estado& e) {
    if (s == "Confirmada") e = CONFIRMADA;
    else if (s == "Pendiente") e = PENDIENTE;
    else if (s == "EnEspera")  e = EN_ESPERA;
    else if (s == "CheckIn")   e = CHECKIN;
    else if (s == "Cancelada") e = CANCELADA;
    else return false;
    return true;
}
inline const char* textoClase(Clase c) {
    return c == ECONOMICA ? "Economica" : (c == PREMIUM_CLASE ? "Premium" : "Ejecutiva");
}
inline bool claseDesdeTexto(const string& s, Clase& c) {
    if (s == "Economica") c = ECONOMICA;
    else if (s == "Premium") c = PREMIUM_CLASE;
    else if (s == "Ejecutiva") c = EJECUTIVA;
    else return false;
    return true;
}
inline const char* textoCategoria(Categoria c) {
    return c == PREMIUM ? "Premium" : (c == FRECUENTE ? "Frecuente" : "Regular");
}
inline bool categoriaDesdeTexto(const string& s, Categoria& c) {
    if (s == "Premium") c = PREMIUM;
    else if (s == "Frecuente") c = FRECUENTE;
    else if (s == "Regular") c = REGULAR;
    else return false;
    return true;
}
inline bool ocupaAsiento(Estado e) {
    return e == CONFIRMADA || e == PENDIENTE || e == CHECKIN;
}

#endif
