#include <iostream>
#include <string>
using namespace std;

enum Clase     { ECONOMICA, PREMIUM_CLASE, EJECUTIVA };
enum Categoria { REGULAR = 1, FRECUENTE = 2, PREMIUM = 3 }; 
enum Estado    { CONFIRMADA, PENDIENTE, EN_ESPERA, CHECKIN, CANCELADA };
int const CAPACIDAD = 100000;    

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
    Reserva* siguiente;          //Puntero a la siguiente reserva (para listas enlazadas)
};

class TablaHash {
private:
    Reserva* tabla[CAPACIDAD];
    
    int hash(int valor){ //Reemplazar valor por una variable que me permita calcular el hash.

    }
public:
    TablaHash(){
        for(int i =0; i < CAPACIDAD; i++){
            tabla[i] = nullptr;
        }   
    }

    void insertar(Reserva* reserva){
        int idx = hash(reserva->precio);       //Reemplazar idReserva por una variable que me permita calcular el hash.
        tabla[idx] = reserva;
    }

    bool buscar(string idReserva) {            //Se debe hacer la búsqueda usando el idReserva
        int idx = hash(CAPACIDAD);             
        Reserva* actual = tabla[idx];

        while (actual != nullptr) {
            if (actual->idReserva == idReserva) return true;
            actual = actual->siguiente;
        }
        return false;
    }
};
