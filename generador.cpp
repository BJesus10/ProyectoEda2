// generador.cpp - genera vuelos.csv, reservas_pequeno.csv y reservas_masivo.csv.
// Compilar: g++ -std=c++11 -O2 generador.cpp -o generador   (ejecutar en la carpeta de datos)
//
// Condiciones del enunciado que cumple (semilla fija => archivos reproducibles):
//  - 50 vuelos con capacidad de 150 a 300 asientos.
//  - 10 000 reservas en el masivo; 4 vuelos llenos con 6 a 9 pasajeros en espera
//    de categorias mezcladas; varias reservas con igual fecha y hora (todas las de un vuelo).
//  - 51 IDs que caen en la MISMA posicion de la tabla hash (con TAM_TABLA_HASH de config.h).
//  - Premium, Frecuente y Regular en cantidad suficiente.
// No usa contenedores de la STL (solo arreglos y string).
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include "config.h"
#include "hash_util.h"
using namespace std;

// ---- generador pseudoaleatorio propio (igual resultado en cualquier plataforma) ----
static unsigned int semilla = 2026;
static unsigned int aleatorio() { semilla = semilla * 1664525u + 1013904223u; return semilla >> 8; }
static int az(int n) { return (int)(aleatorio() % (unsigned int)n); }

static string rellenar(int x, int ancho) {
    string s = to_string(x);
    while ((int)s.size() < ancho) s = "0" + s;
    return s;
}

// dia 0 = 2026-12-01 ... dia 89 = 2027-02-28
static string fechaDeDia(int d) {
    if (d < 31) return "2026-12-" + rellenar(d + 1, 2);
    if (d < 62) return "2027-01-" + rellenar(d - 30, 2);
    return "2027-02-" + rellenar(d - 61, 2);
}

static const int NV = 50;
static const int N_MASIVO = 10000;
static const int N_CANCELADAS = 800;
static const int N_COLISIONES = 50;

struct VueloG { string cod, orig, dest, fecha, hora; int cap; int asientos; int espera; };
struct ResG {
    string dni, nombre, cod, orig, dest, fecha, hora, clase, cat, estado;
    double precio; bool colision;
};

static const char* CIUDADES[10] = {"Cusco","Arequipa","Trujillo","Piura","Iquitos","Chiclayo","Tacna","Pucallpa","Juliaca","Cajamarca"};
static const char* HORAS[6] = {"06:45","09:20","12:10","15:30","18:05","21:40"};
static const char* NOMBRES[20] = {"Ana","Luis","Maria","Carlos","Rosa","Jorge","Lucia","Pedro","Elena","Miguel","Sofia","Diego","Carmen","Raul","Paula","Victor","Julia","Andres","Marta","Hugo"};
static const char* APELLIDOS[20] = {"Torres","Ramos","Soto","Quispe","Flores","Rojas","Vargas","Castro","Mendoza","Huaman","Salazar","Paredes","Chavez","Ortiz","Silva","Reyes","Cruz","Diaz","Vega","Lopez"};

static string precioTexto(double p) { char b[32]; snprintf(b, sizeof b, "%.2f", p); return string(b); }

static void llenarReserva(ResG& r, const VueloG& v, const char* clase, const char* cat, const char* estado) {
    r.cod = v.cod; r.orig = v.orig; r.dest = v.dest; r.fecha = v.fecha; r.hora = v.hora;
    r.clase = clase; r.cat = cat; r.estado = estado; r.colision = false;
    r.dni = to_string(10000000 + az(89999999));
    r.nombre = string(NOMBRES[az(20)]) + " " + APELLIDOS[az(20)];
    int base = (string(clase) == "Economica") ? 120 + az(230) : (string(clase) == "Premium" ? 300 + az(200) : 500 + az(600));
    r.precio = base + az(100) / 100.0;
}

static const char* sorteaClase() { int x = az(100); return x < 70 ? "Economica" : (x < 90 ? "Premium" : "Ejecutiva"); }
static const char* sorteaCategoria() { int x = az(100); return x < 60 ? "Regular" : (x < 88 ? "Frecuente" : "Premium"); }
static const char* sorteaEstadoAsiento() { int x = az(100); return x < 60 ? "Confirmada" : (x < 85 ? "Pendiente" : "CheckIn"); }

static void escribirReserva(ofstream& f, const string& id, const ResG& r) {
    f << id << "," << r.dni << "," << r.nombre << "," << r.cod << "," << r.orig << "," << r.dest << ","
      << r.fecha << "," << r.hora << "," << r.clase << "," << r.cat << "," << precioTexto(r.precio) << "," << r.estado << "\n";
}

static void mezclar(ResG* a, int n) {
    for (int i = n - 1; i > 0; i--) { int j = az(i + 1); ResG t = a[i]; a[i] = a[j]; a[j] = t; }
}

int main() {
    VueloG v[NV];
    // ---------------- vuelos ----------------
    const int llenos[4] = {3, 17, 29, 41};
    const int capLlenos[4] = {150, 160, 170, 180};
    const int espLlenos[4] = {8, 7, 6, 9};
    for (int i = 0; i < NV; i++) {
        string pref = (i % 3 == 0) ? "LA" : (i % 3 == 1 ? "AV" : "H2");
        v[i].cod = pref + to_string(100 + i * 13);
        const char* ciudad = CIUDADES[(i / 2) % 10];
        if (i % 2 == 0) { v[i].orig = "Lima"; v[i].dest = ciudad; } else { v[i].orig = ciudad; v[i].dest = "Lima"; }
        v[i].fecha = fechaDeDia(az(90));
        v[i].hora = HORAS[az(6)];
        v[i].cap = 150 + 10 * az(16);
        v[i].asientos = 0; v[i].espera = 0;
    }
    for (int k = 0; k < 4; k++) { v[llenos[k]].cap = capLlenos[k]; v[llenos[k]].espera = espLlenos[k]; }

    ofstream fv("vuelos.csv");
    fv << "codigoVuelo,origen,destino,fecha,hora,capacidad\n";
    for (int i = 0; i < NV; i++)
        fv << v[i].cod << "," << v[i].orig << "," << v[i].dest << "," << v[i].fecha << "," << v[i].hora << "," << v[i].cap << "\n";
    fv.close();

    // ---------------- plan de asientos por vuelo ----------------
    int totalLlenos = 0, totalEspera = 0, capOtros = 0;
    for (int i = 0; i < NV; i++) {
        bool lleno = false;
        for (int k = 0; k < 4; k++) if (llenos[k] == i) lleno = true;
        if (lleno) { v[i].asientos = v[i].cap; totalLlenos += v[i].cap; totalEspera += v[i].espera; }
        else capOtros += v[i].cap;
    }
    int asientosOtros = N_MASIVO - N_CANCELADAS - totalLlenos - totalEspera;
    if (asientosOtros * 100 > capOtros * 92) { cerr << "Error: capacidad insuficiente para el plan.\n"; return 1; }
    int asignados = 0;
    for (int i = 0; i < NV; i++) {
        if (v[i].asientos == v[i].cap) continue;
        v[i].asientos = (int)((long long)v[i].cap * asientosOtros / capOtros);
        asignados += v[i].asientos;
    }
    for (int i = 0; asignados < asientosOtros; i = (i + 1) % NV) {      // reparte el resto
        if (v[i].asientos == v[i].cap) continue;
        if (v[i].asientos < v[i].cap - 5) { v[i].asientos++; asignados++; }
    }

    // ---------------- reservas del masivo ----------------
    ResG* res = new ResG[N_MASIVO];
    int n = 0;
    for (int i = 0; i < NV; i++) {
        for (int j = 0; j < v[i].asientos; j++) llenarReserva(res[n++], v[i], sorteaClase(), sorteaCategoria(), sorteaEstadoAsiento());
        for (int j = 0; j < v[i].espera; j++) {
            const char* cat = (j % 3 == 0) ? "Regular" : (j % 3 == 1 ? "Frecuente" : "Premium");   // categorias mezcladas
            llenarReserva(res[n++], v[i], sorteaClase(), cat, "EnEspera");
        }
    }
    for (int j = 0; j < N_CANCELADAS; j++) llenarReserva(res[n++], v[az(NV)], sorteaClase(), sorteaCategoria(), "Cancelada");
    if (n != N_MASIVO) { cerr << "Error: se generaron " << n << " reservas.\n"; return 1; }

    // 50 reservas (de las que tienen asiento) llevaran IDs que colisionan en la hash
    int marcadas = 0;
    while (marcadas < N_COLISIONES) {
        int k = az(N_MASIVO);
        string e = res[k].estado;
        if (!res[k].colision && (e == "Confirmada" || e == "Pendiente" || e == "CheckIn")) { res[k].colision = true; marcadas++; }
    }
    mezclar(res, N_MASIVO);

    // IDs que colisionan con "RES-000001" (posicion objetivo de la hash)
    const int objetivo = hashId("RES-000001", TAM_TABLA_HASH);
    string idsCol[N_COLISIONES];
    int hallados = 0;
    for (int x = 100000; x <= 999999 && hallados < N_COLISIONES; x++) {
        string id = "RES-" + rellenar(x, 6);
        if (hashId(id, TAM_TABLA_HASH) == objetivo) idsCol[hallados++] = id;
    }
    if (hallados < N_COLISIONES) { cerr << "Error: no se hallaron IDs que colisionen.\n"; return 1; }

    ofstream fm("reservas_masivo.csv");
    int sec = 0, col = 0;
    for (int i = 0; i < N_MASIVO; i++) {
        string id = res[i].colision ? idsCol[col++] : "RES-" + rellenar(++sec, 6);
        escribirReserva(fm, id, res[i]);
    }
    fm.close();

    // ---------------- archivo pequeno (verificacion manual) ----------------
    ofstream fp("reservas_pequeno.csv");
    ResG pq[30];
    int m = 0;
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 5; j++) {
            const char* est = (m == 7 || m == 22) ? "Cancelada" : sorteaEstadoAsiento();
            llenarReserva(pq[m++], v[i], sorteaClase(), sorteaCategoria(), est);
        }
    mezclar(pq, 30);
    for (int i = 0; i < 30; i++) escribirReserva(fp, "RES-" + rellenar(i + 1, 6), pq[i]);
    fp.close();

    cout << "Archivos generados: vuelos.csv (" << NV << " vuelos), reservas_masivo.csv (" << N_MASIVO
         << " reservas), reservas_pequeno.csv (30 reservas)\n";
    cout << "Vuelos llenos con espera:";
    for (int k = 0; k < 4; k++) cout << " " << v[llenos[k]].cod << "(cap " << v[llenos[k]].cap << ", espera " << v[llenos[k]].espera << ")";
    cout << "\nIDs que colisionan en la posicion " << objetivo << " de la hash (tamano " << TAM_TABLA_HASH << "): "
         << (N_COLISIONES + 1) << " (incluye RES-000001)\n";
    delete[] res;
    return 0;
}
