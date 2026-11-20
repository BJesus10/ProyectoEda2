#include <iostream>
#include <string>
using namespace std;

enum Clase     { ECONOMICA, PREMIUM_CLASE, EJECUTIVA };
enum Categoria { REGULAR = 1, FRECUENTE = 2, PREMIUM = 3 }; 
enum Estado    { CONFIRMADA, PENDIENTE, EN_ESPERA, CHECKIN, CANCELADA };


struct Reserva {                //Estructura de la reserva y sus atributos
    string idReserva;        
    string dni;
    string pasajero;
    string codVuelo;
    string origen;
    string destino;
    string fecha;
    string hora;
    Clase clase;
    Categoria categoría;
    double precio;
    Estado estado;
};
