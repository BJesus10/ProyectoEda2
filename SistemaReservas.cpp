// SistemaReservas.cpp
#include "SistemaReservas.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

using namespace std;

// Divide una linea CSV en campos (sin comillas ni comas dentro de los campos).
static int dividir(const string& linea, string* campos, int maxCampos) {
    int n = 0;
    size_t ini = 0;
    while (n < maxCampos) {
        size_t p = linea.find(',', ini);
        if (p == string::npos) { campos[n++] = linea.substr(ini); break; }
        campos[n++] = linea.substr(ini, p - ini);
        ini = p + 1;
    }
    return n;
}

static void quitarRetorno(string& s) {
    if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
}

static bool construirReserva(const string* c, Reserva& r) {
    r.idReserva = c[0]; r.dni = c[1]; r.pasajero = c[2]; r.codigoVuelo = c[3];
    r.origen = c[4]; r.destino = c[5]; r.fecha = c[6]; r.hora = c[7];
    if (!claseDesdeTexto(c[8], r.clase)) return false;
    if (!categoriaDesdeTexto(c[9], r.categoria)) return false;
    r.precio = atof(c[10].c_str());
    if (!estadoDesdeTexto(c[11], r.estado)) return false;
    return !r.idReserva.empty();
}

SistemaReservas::SistemaReservas(int tamTabla) : hash(tamTabla), nVuelos(0), contadorLlegada(0) {}

// Orden de destruccion (inverso al de declaracion): heaps, arbol y por ultimo la hash,
// que es la unica que libera las Reserva.
SistemaReservas::~SistemaReservas() {}

int SistemaReservas::vueloIndice(const string& codigo) const {
    for (int i = 0; i < nVuelos; i++)
        if (vuelos[i].datos.codigoVuelo == codigo) return i;
    return -1;
}

// ---------------------------------------------------------------- 1. carga
bool SistemaReservas::cargarVuelos(const string& archivo, int& cargados) {
    ifstream in(archivo.c_str());
    if (!in) return false;
    cargados = 0;
    string linea, c[6];
    while (getline(in, linea)) {
        quitarRetorno(linea);
        if (linea.empty() || linea.compare(0, 11, "codigoVuelo") == 0) continue;
        if (nVuelos >= MAX_VUELOS) break;
        if (dividir(linea, c, 6) != 6) continue;
        Vuelo& v = vuelos[nVuelos].datos;
        v.codigoVuelo = c[0]; v.origen = c[1]; v.destino = c[2];
        v.fecha = c[3]; v.hora = c[4];
        v.capacidad = atoi(c[5].c_str());
        v.asientosOcupados = 0;
        nVuelos++;
        cargados++;
    }
    return true;
}

bool SistemaReservas::cargarReservas(const string& archivo, ResumenCarga& res) {
    ifstream in(archivo.c_str());
    if (!in) return false;
    res = ResumenCarga();
    string linea, c[12];
    while (getline(in, linea)) {
        quitarRetorno(linea);
        if (linea.empty() || linea.compare(0, 9, "idReserva") == 0) continue;
        res.leidas++;
        if (dividir(linea, c, 12) != 12) { res.rechazadas++; continue; }

        Reserva* r = new Reserva();
        int iv = -1;
        if (!construirReserva(c, *r) || hash.buscar(r->idReserva) != nullptr ||
            (iv = vueloIndice(r->codigoVuelo)) < 0) {
            delete r;
            res.rechazadas++;
            continue;
        }

        hash.insertar(r);
        if (!arbol.insertar(r)) {          // clave duplicada: deshacer
            hash.eliminar(r->idReserva);   // (libera r)
            res.rechazadas++;
            continue;
        }
        res.enHash++;
        res.enArbol++;

        if (r->estado == EN_ESPERA) {
            r->ordenLlegada = ++contadorLlegada;
            vuelos[iv].espera.insertar(r);
            res.enHeaps++;
        } else if (ocupaAsiento(r->estado)) {
            vuelos[iv].datos.asientosOcupados++;
        }
    }
    return true;
}

// ------------------------------------------------------------ 2. registro
SistemaReservas::ResultadoRegistro SistemaReservas::registrar(
        const string& id, const string& dni, const string& pasajero, const string& codigoVuelo,
        Clase clase, Categoria cat, double precio, Estado estadoSiHayAsiento, Reserva** creada) {
    if (creada) *creada = nullptr;
    if (id.empty() || precio < 0) return REG_DATOS_INVALIDOS;
    if (hash.buscar(id) != nullptr) return REG_ID_EXISTE;
    int iv = vueloIndice(codigoVuelo);
    if (iv < 0) return REG_VUELO_NO_EXISTE;

    EntradaVuelo& ev = vuelos[iv];
    Reserva* r = new Reserva();
    r->idReserva = id; r->dni = dni; r->pasajero = pasajero; r->codigoVuelo = codigoVuelo;
    r->origen = ev.datos.origen; r->destino = ev.datos.destino;
    r->fecha = ev.datos.fecha;   r->hora = ev.datos.hora;
    r->clase = clase; r->categoria = cat; r->precio = precio;

    bool hayAsiento = ev.datos.asientosOcupados < ev.datos.capacidad;
    r->estado = hayAsiento ? (estadoSiHayAsiento == PENDIENTE ? PENDIENTE : CONFIRMADA) : EN_ESPERA;

    hash.insertar(r);
    if (!arbol.insertar(r)) {
        hash.eliminar(id);
        return REG_DATOS_INVALIDOS;
    }
    if (hayAsiento) {
        ev.datos.asientosOcupados++;
    } else {
        r->ordenLlegada = ++contadorLlegada;
        ev.espera.insertar(r);
    }
    if (creada) *creada = r;
    return hayAsiento ? REG_ASIENTO : REG_ESPERA;
}

// ------------------------------------------------------- 6. actualizacion
SistemaReservas::ResultadoActualizacion SistemaReservas::actualizarEstado(const string& id, Estado nuevo) {
    Reserva* r = hash.buscar(id);
    if (r == nullptr) return ACT_NO_EXISTE;
    bool permitida = (r->estado == PENDIENTE && nuevo == CONFIRMADA) ||
                     (r->estado == CONFIRMADA && nuevo == CHECKIN);
    if (!permitida) return ACT_TRANSICION_INVALIDA;
    r->estado = nuevo;   // el arbol y los heaps comparten el puntero: no hay que tocarlos
    return ACT_OK;
}

// --------------------------------------------------------- 7. cancelacion
SistemaReservas::ResultadoCancelacion SistemaReservas::cancelar(const string& id, Reserva** promovida) {
    if (promovida) *promovida = nullptr;
    Reserva* r = hash.buscar(id);
    if (r == nullptr) return CAN_NO_EXISTE;
    if (r->estado == CANCELADA) return CAN_YA_CANCELADA;

    int iv = vueloIndice(r->codigoVuelo);
    EntradaVuelo& ev = vuelos[iv];

    if (r->estado == EN_ESPERA) {              // caso B
        ev.espera.eliminar(r);
        r->estado = CANCELADA;
        return CAN_OK_ESPERA;
    }
    // caso A: ocupaba asiento
    r->estado = CANCELADA;
    ev.datos.asientosOcupados--;
    Reserva* p = ev.espera.extraerMax();
    if (p != nullptr) {
        p->estado = CONFIRMADA;
        ev.datos.asientosOcupados++;
        if (promovida) *promovida = p;
    }
    return CAN_OK_ASIENTO;
}

// -------------------------------------------------- 8. eliminacion fisica
SistemaReservas::ResultadoEliminacion SistemaReservas::eliminarFisico(const string& id) {
    Reserva* r = hash.buscar(id);
    if (r == nullptr) return ELI_NO_EXISTE;
    if (r->estado != CANCELADA) return ELI_NO_CANCELADA;
    arbol.eliminar(r);    // 1) arbol (necesita la reserva viva para calcular su clave)
    hash.eliminar(id);    // 2) hash: quita y libera la reserva
    return ELI_OK;
}

// ---------------------------------------------------------- 10. estadisticas
struct ContextoStats { Estadisticas* e; };

static void visitar(Reserva* r, void* ctx) {
    Estadisticas* e = ((ContextoStats*)ctx)->e;
    e->total++;
    e->promedio += r->precio;                       // suma; se divide al final
    if (r->estado == CONFIRMADA || r->estado == CHECKIN) e->ingresos += r->precio;
    int k = 0;
    while (k < e->nDestinos && e->destino[k] != r->destino) k++;
    if (k == e->nDestinos && e->nDestinos < MAX_DESTINOS) { e->destino[k] = r->destino; e->nDestinos++; }
    if (k < e->nDestinos) e->porDestino[k]++;
    e->porClase[r->clase]++;
    e->porEstado[r->estado]++;
    if (e->mayor == nullptr || r->precio > e->mayor->precio) e->mayor = r;
    if (e->menor == nullptr || r->precio < e->menor->precio) e->menor = r;
}

Estadisticas SistemaReservas::estadisticas() const {
    Estadisticas e;
    ContextoStats ctx = { &e };
    arbol.recorrerEnOrden(visitar, &ctx);
    if (e.total > 0) e.promedio /= e.total;
    for (int i = 0; i < nVuelos; i++) {
        if (vuelos[i].espera.tamanio() > e.maxEspera) {
            e.maxEspera = vuelos[i].espera.tamanio();
            e.vueloMasEspera = i;
        }
    }
    return e;
}

// ------------------------------------------------------------- impresion
void SistemaReservas::imprimirReserva(const Reserva* r) {
    cout << r->idReserva << " | " << r->dni << " | " << r->pasajero << " | " << r->codigoVuelo
         << " | " << r->origen << "->" << r->destino << " | " << r->fecha << " " << r->hora
         << " | " << textoClase(r->clase) << " | " << textoCategoria(r->categoria)
         << " | " << fixed << setprecision(2) << r->precio << " | " << textoEstado(r->estado) << "\n";
}

void SistemaReservas::imprimirEstadisticas(const SistemaReservas& s, const Estadisticas& e) {
    cout << "Total de reservas: " << e.total << "\n";
    cout << fixed << setprecision(2);
    cout << "Ingresos (Confirmada + CheckIn): " << e.ingresos << "\n";
    cout << "Precio promedio: " << e.promedio << "\n";
    cout << "Por destino:";
    for (int i = 0; i < e.nDestinos; i++) cout << " " << e.destino[i] << "=" << e.porDestino[i];
    cout << "\nPor clase:";
    for (int i = 0; i < 3; i++) cout << " " << textoClase((Clase)i) << "=" << e.porClase[i];
    cout << "\nPor estado:";
    for (int i = 0; i < 5; i++) cout << " " << textoEstado((Estado)i) << "=" << e.porEstado[i];
    cout << "\nMayor precio: "; if (e.mayor) imprimirReserva(e.mayor); else cout << "-\n";
    cout << "Menor precio: "; if (e.menor) imprimirReserva(e.menor); else cout << "-\n";
    if (e.vueloMasEspera >= 0)
        cout << "Vuelo con mas pasajeros en espera: " << s.vuelo(e.vueloMasEspera).codigoVuelo
             << " (" << e.maxEspera << ")\n";
    else
        cout << "Ningun vuelo tiene lista de espera.\n";
}
