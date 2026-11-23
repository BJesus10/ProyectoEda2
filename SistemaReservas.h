// SistemaReservas.h - coordina hash, arbol AVL y heaps de espera.
// Toda alta, baja o cambio de estado pasa por aqui para mantener la consistencia:
//   - toda reserva esta en la hash y en el arbol;
//   - toda reserva EnEspera esta ademas en el heap de su vuelo.
#ifndef SISTEMA_RESERVAS_H
#define SISTEMA_RESERVAS_H

#include <string>
#include "modelo.h"
#include "config.h"
#include "TablaHash.h"
#include "ArbolAVL.h"
#include "HeapEspera.h"

const int MAX_DESTINOS = 64;

struct ResumenCarga {
    int leidas = 0, enHash = 0, enArbol = 0, enHeaps = 0, rechazadas = 0;
};

struct Estadisticas {
    int total = 0;
    double ingresos = 0.0;      // suma de precios de Confirmada y CheckIn
    double promedio = 0.0;      // precio promedio de todas las reservas
    int nDestinos = 0;
    string destino[MAX_DESTINOS];
    int porDestino[MAX_DESTINOS];
    int porClase[3];
    int porEstado[5];
    Reserva* mayor = nullptr;
    Reserva* menor = nullptr;
    int vueloMasEspera = -1;    // indice de vuelo
    int maxEspera = 0;
    Estadisticas() {
        for (int i = 0; i < MAX_DESTINOS; i++) porDestino[i] = 0;
        for (int i = 0; i < 3; i++) porClase[i] = 0;
        for (int i = 0; i < 5; i++) porEstado[i] = 0;
    }
};

class SistemaReservas {
public:
    enum ResultadoRegistro { REG_ASIENTO, REG_ESPERA, REG_ID_EXISTE, REG_VUELO_NO_EXISTE, REG_DATOS_INVALIDOS };
    enum ResultadoActualizacion { ACT_OK, ACT_NO_EXISTE, ACT_TRANSICION_INVALIDA };
    enum ResultadoCancelacion { CAN_OK_ASIENTO, CAN_OK_ESPERA, CAN_NO_EXISTE, CAN_YA_CANCELADA };
    enum ResultadoEliminacion { ELI_OK, ELI_NO_EXISTE, ELI_NO_CANCELADA };

    explicit SistemaReservas(int tamTabla = TAM_TABLA_HASH);
    ~SistemaReservas();

    // 1. Carga masiva
    bool cargarVuelos(const string& archivo, int& cargados);
    bool cargarReservas(const string& archivo, ResumenCarga& resumen);

    // 2. Registro manual (fecha, hora, origen y destino se toman del vuelo)
    ResultadoRegistro registrar(const string& id, const string& dni, const string& pasajero,
                                const string& codigoVuelo, Clase clase, Categoria cat,
                                double precio, Estado estadoSiHayAsiento, Reserva** creada = nullptr);

    // 3. Busqueda por ID (hash)
    Reserva* buscarPorId(const string& id) const { return hash.buscar(id); }

    // 4. Listado cronologico (arbol): copia hasta "max" reservas en orden
    int listadoCronologico(Reserva** salida, int max) const { return arbol.primerasN(salida, max); }

    // 5. Rango de fechas (arbol, con poda). Devuelve el total encontrado.
    int consultaRango(const string& ini, const string& fin, Reserva** muestra, int maxMuestra, int& enMuestra) const {
        return arbol.rangoFechas(ini, fin, muestra, maxMuestra, enMuestra);
    }

    // 6. Actualizacion de estado: solo Pendiente->Confirmada y Confirmada->CheckIn
    ResultadoActualizacion actualizarEstado(const string& id, Estado nuevo);

    // 7. Cancelacion. Caso A: libera asiento y promueve al primero de la espera.
    //    Caso B: si estaba EnEspera, se retira del heap desde cualquier posicion.
    ResultadoCancelacion cancelar(const string& id, Reserva** promovida = nullptr);

    // 8. Eliminacion fisica (solo Cancelada): primero el arbol, luego la hash.
    ResultadoEliminacion eliminarFisico(const string& id);

    // 9. Lista de espera de un vuelo (copia ordenada; no altera el heap)
    int listaEspera(int vueloIdx, Reserva** salida, int max) const { return vuelos[vueloIdx].espera.copiaOrdenada(salida, max); }
    const Reserva* siguienteEnEspera(int vueloIdx) const { return vuelos[vueloIdx].espera.maximo(); }

    // 10. Estadisticas generales
    Estadisticas estadisticas() const;

    // ---- consultas auxiliares (pruebas y mediciones) ----
    int vueloIndice(const string& codigo) const;
    int numVuelos() const { return nVuelos; }
    const Vuelo& vuelo(int i) const { return vuelos[i].datos; }
    int esperaTam(int i) const { return vuelos[i].espera.tamanio(); }
    bool heapValido(int i) const { return vuelos[i].espera.verificar(); }
    bool enHeap(int i, const Reserva* r) const { return vuelos[i].espera.contiene(r); }

    int tamanioHash() const { return hash.cantidad(); }
    int tamanioArbol() const { return arbol.tamanio(); }
    int alturaArbol() const { return arbol.altura(); }
    bool arbolValido() const { return arbol.verificar(); }
    double factorCarga() const { return hash.factorCarga(); }
    int cadenaMasLarga() const { return hash.cadenaMasLarga(); }
    int tamTabla() const { return hash.tamanio(); }
    Reserva* buscarEnArbol(const string& f, const string& h, const string& id) const { return arbol.buscar(f, h, id); }
    int copiarTodas(Reserva** salida, int max) const { return arbol.primerasN(salida, max); }

    static void imprimirReserva(const Reserva* r);
    static void imprimirEstadisticas(const SistemaReservas& s, const Estadisticas& e);

private:
    struct EntradaVuelo {
        Vuelo datos;
        HeapEspera espera;
    };

    TablaHash hash;
    ArbolAVL arbol;
    EntradaVuelo vuelos[MAX_VUELOS];
    int nVuelos;
    int contadorLlegada;   // orden de llegada global a las listas de espera
};

#endif
