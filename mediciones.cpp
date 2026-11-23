// mediciones.cpp - tiempos y metricas del sistema completo para el informe.
// Compilar: g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp mediciones.cpp -o mediciones
// Ejecutar en la carpeta con vuelos.csv y reservas_masivo.csv.
// Cada medicion se repite 3 veces (sobre un sistema recien cargado) y se informa la mediana.
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include "SistemaReservas.h"
using namespace std;

typedef chrono::steady_clock reloj;
static double us(reloj::time_point a, reloj::time_point b) { return chrono::duration<double, micro>(b - a).count(); }
static double ms(reloj::time_point a, reloj::time_point b) { return chrono::duration<double, milli>(b - a).count(); }

static double mediana3(double a, double b, double c) {
    if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
    if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
    return c;
}

static SistemaReservas* nuevoSistema() {
    SistemaReservas* s = new SistemaReservas();
    int nv; ResumenCarga rc;
    s->cargarVuelos("vuelos.csv", nv);
    s->cargarReservas("reservas_masivo.csv", rc);
    return s;
}

static Reserva* busquedaSecuencial(Reserva** a, int n, const string& id) {
    for (int i = 0; i < n; i++) if (a[i]->idReserva == id) return a[i];
    return nullptr;
}

static volatile long sumidero = 0;   // evita que el compilador elimine las busquedas

// ------------------------------------------------------------------ mediciones
static double medirCarga() {
    reloj::time_point t0 = reloj::now();
    SistemaReservas* s = nuevoSistema();
    reloj::time_point t1 = reloj::now();
    delete s;
    return ms(t0, t1);   // incluye leer vuelos.csv y reservas_masivo.csv
}

static void medirBusquedas(double& hashUs, double& secUs) {
    SistemaReservas* s = nuevoSistema();
    int N = s->tamanioHash();
    Reserva** todas = new Reserva*[N];
    s->copiarTodas(todas, N);
    string ids[1000];
    for (int i = 0; i < 1000; i++) ids[i] = todas[i * (N / 1000)]->idReserva;   // 1 000 IDs repartidos

    reloj::time_point t0 = reloj::now();
    for (int i = 0; i < 1000; i++) if (s->buscarPorId(ids[i]) != nullptr) sumidero = sumidero + 1;
    reloj::time_point t1 = reloj::now();
    hashUs = us(t0, t1) / 1000;

    t0 = reloj::now();
    for (int i = 0; i < 1000; i++) if (busquedaSecuencial(todas, N, ids[i]) != nullptr) sumidero = sumidero + 1;
    t1 = reloj::now();
    secUs = us(t0, t1) / 1000;
    delete[] todas;
    delete s;
}

static double medirInsercionIndividual() {
    SistemaReservas* s = nuevoSistema();
    // 100 vuelos-destino repartidos entre los vuelos con asientos libres (sin llenarlos)
    int destinos[100], nd = 0;
    for (int rondas = 0; nd < 100 && rondas < 100; rondas++)
        for (int i = 0; i < s->numVuelos() && nd < 100; i++)
            if (s->vuelo(i).asientosOcupados + rondas < s->vuelo(i).capacidad - 1) destinos[nd++] = i;
    reloj::time_point t0 = reloj::now();
    for (int i = 0; i < 100; i++) {
        string id = "INS-" + to_string(100000 + i);
        s->registrar(id, "70000000", "Prueba", s->vuelo(destinos[i]).codigoVuelo, ECONOMICA, REGULAR, 150.0, CONFIRMADA);
    }
    reloj::time_point t1 = reloj::now();
    delete s;
    return us(t0, t1) / 100;
}

static void medirConsultas(double& primeras50, double& mes, double& semana, double& vacio, int& nMes, int& nSemana) {
    SistemaReservas* s = nuevoSistema();
    Reserva* buf[50]; int enM = 0, total = 0;
    reloj::time_point t0 = reloj::now();
    for (int r = 0; r < 100; r++) s->listadoCronologico(buf, 50);
    reloj::time_point t1 = reloj::now();
    primeras50 = us(t0, t1) / 100;
    t0 = reloj::now(); for (int r = 0; r < 100; r++) total = s->consultaRango("2027-01-01", "2027-01-31", buf, 50, enM); t1 = reloj::now();
    mes = us(t0, t1) / 100; nMes = total;
    t0 = reloj::now(); for (int r = 0; r < 100; r++) total = s->consultaRango("2026-12-01", "2026-12-07", buf, 50, enM); t1 = reloj::now();
    semana = us(t0, t1) / 100; nSemana = total;
    t0 = reloj::now(); for (int r = 0; r < 100; r++) total = s->consultaRango("2030-01-01", "2030-01-31", buf, 50, enM); t1 = reloj::now();
    vacio = us(t0, t1) / 100;
    delete s;
}

// 10 reservas, 10 repeticiones: cada repeticion usa un sistema recien cargado.
static double medirActualizacion() {
    double suma = 0;
    for (int rep = 0; rep < 10; rep++) {
        SistemaReservas* s = nuevoSistema();
        int N = s->tamanioHash();
        Reserva** todas = new Reserva*[N];
        s->copiarTodas(todas, N);
        string ids[10]; int n = 0;
        for (int i = 0; i < N && n < 10; i++) if (todas[i]->estado == PENDIENTE) ids[n++] = todas[i]->idReserva;
        reloj::time_point t0 = reloj::now();
        for (int i = 0; i < 10; i++) s->actualizarEstado(ids[i], CONFIRMADA);
        reloj::time_point t1 = reloj::now();
        suma += us(t0, t1) / 10;
        delete[] todas; delete s;
    }
    return suma / 10;
}

static double medirCancelacionConPromocion() {
    double suma = 0;
    for (int rep = 0; rep < 10; rep++) {
        SistemaReservas* s = nuevoSistema();
        int N = s->tamanioHash();
        Reserva** todas = new Reserva*[N];
        s->copiarTodas(todas, N);
        string ids[10]; int n = 0;       // reservas con asiento en vuelos llenos con espera
        int usadas[MAX_VUELOS];          // a lo sumo tantas por vuelo como pasajeros en espera haya,
        for (int i = 0; i < MAX_VUELOS; i++) usadas[i] = 0;   // para que cada cancelacion promueva a alguien
        for (int i = 0; i < N && n < 10; i++) {
            int iv = s->vueloIndice(todas[i]->codigoVuelo);
            if (ocupaAsiento(todas[i]->estado) && s->vuelo(iv).asientosOcupados == s->vuelo(iv).capacidad && usadas[iv] < s->esperaTam(iv)) {
                ids[n++] = todas[i]->idReserva;
                usadas[iv]++;
            }
        }
        int promovidas = 0;
        Reserva* p = nullptr;
        reloj::time_point t0 = reloj::now();
        for (int i = 0; i < 10; i++) { s->cancelar(ids[i], &p); if (p) promovidas++; }
        reloj::time_point t1 = reloj::now();
        if (promovidas != 10) cerr << "Aviso: solo " << promovidas << " promociones\n";
        suma += us(t0, t1) / 10;
        delete[] todas; delete s;
    }
    return suma / 10;
}

static double medirEliminacionFisica() {
    double suma = 0;
    for (int rep = 0; rep < 10; rep++) {
        SistemaReservas* s = nuevoSistema();
        int N = s->tamanioHash();
        Reserva** todas = new Reserva*[N];
        s->copiarTodas(todas, N);
        string ids[10]; int n = 0;
        for (int i = 0; i < N && n < 10; i++) if (todas[i]->estado == CANCELADA) ids[n++] = todas[i]->idReserva;
        reloj::time_point t0 = reloj::now();
        for (int i = 0; i < 10; i++) s->eliminarFisico(ids[i]);   // ya estaban Canceladas en el archivo
        reloj::time_point t1 = reloj::now();
        suma += us(t0, t1) / 10;
        delete[] todas; delete s;
    }
    return suma / 10;
}

static string fechaDeDia(int d) {
    int dia = d < 31 ? d + 1 : d - 30;
    string s = to_string(dia);
    if (s.size() < 2) s = "0" + s;
    return (d < 31 ? "2026-12-" : "2027-01-") + s;
}

int main() {
    cout << fixed << setprecision(3);
    cout << "Mediciones con reservas_masivo.csv (mediana de 3 ejecuciones)\n\n";

    double c[3];
    for (int i = 0; i < 3; i++) c[i] = medirCarga();
    double carga = mediana3(c[0], c[1], c[2]);

    double h[3], q[3];
    for (int i = 0; i < 3; i++) medirBusquedas(h[i], q[i]);
    double busHash = mediana3(h[0], h[1], h[2]), busSec = mediana3(q[0], q[1], q[2]);

    double ins[3];
    for (int i = 0; i < 3; i++) ins[i] = medirInsercionIndividual();

    double p50[3], mes[3], sem[3], vac[3]; int nMes = 0, nSem = 0;
    for (int i = 0; i < 3; i++) medirConsultas(p50[i], mes[i], sem[i], vac[i], nMes, nSem);

    double act[3], can[3], eli[3];
    for (int i = 0; i < 3; i++) { act[i] = medirActualizacion(); can[i] = medirCancelacionConPromocion(); eli[i] = medirEliminacionFisica(); }

    cout << left << setw(58) << "Operacion" << setw(10) << "Datos" << setw(8) << "Reps" << "Tiempo (mediana)\n";
    cout << string(100, '-') << "\n";
    cout << setw(58) << "Carga masiva (leer archivos + hash + arbol + heaps)" << setw(10) << "10000" << setw(8) << 1 << carga << " ms\n";
    cout << setw(58) << "Busqueda por ID (hash)" << setw(10) << "10000" << setw(8) << 1000 << busHash << " us\n";
    cout << setw(58) << "Busqueda secuencial del mismo ID" << setw(10) << "10000" << setw(8) << 1000 << busSec << " us\n";
    cout << setw(58) << "Insercion individual (hash + arbol + heap si corresponde)" << setw(10) << "10000" << setw(8) << 100 << mediana3(ins[0], ins[1], ins[2]) << " us\n";
    cout << setw(58) << "Consulta ordenada (primeras 50)" << setw(10) << "10000" << setw(8) << 100 << mediana3(p50[0], p50[1], p50[2]) << " us\n";
    cout << setw(58) << ("Consulta por rango (un mes; " + to_string(nMes) + " resultados)") << setw(10) << "10000" << setw(8) << 100 << mediana3(mes[0], mes[1], mes[2]) << " us\n";
    cout << setw(58) << ("Consulta por rango (una semana; " + to_string(nSem) + " resultados)") << setw(10) << "10000" << setw(8) << 100 << mediana3(sem[0], sem[1], sem[2]) << " us\n";
    cout << setw(58) << "Consulta por rango (sin resultados)" << setw(10) << "10000" << setw(8) << 100 << mediana3(vac[0], vac[1], vac[2]) << " us\n";
    cout << setw(58) << "Actualizacion de estado (por reserva)" << setw(10) << "10" << setw(8) << 10 << mediana3(act[0], act[1], act[2]) << " us\n";
    cout << setw(58) << "Cancelacion con promocion desde el heap (por reserva)" << setw(10) << "10" << setw(8) << 10 << mediana3(can[0], can[1], can[2]) << " us\n";
    cout << setw(58) << "Eliminacion fisica (arbol + hash, por reserva)" << setw(10) << "10" << setw(8) << 10 << mediana3(eli[0], eli[1], eli[2]) << " us\n";

    // ---- metricas estructurales ----
    SistemaReservas* s = nuevoSistema();
    cout << "\nMetricas de las estructuras\n" << string(100, '-') << "\n";
    cout << setprecision(4);
    cout << setw(58) << "Factor de carga de la hash (10000 / " + to_string(s->tamTabla()) + ")" << s->factorCarga() << "\n";
    cout << setw(58) << "Longitud de la cadena mas larga de la hash" << s->cadenaMasLarga() << "\n";
    cout << setw(58) << "Altura del AVL con 10000 reservas" << s->alturaArbol() << "  (valido: " << (s->arbolValido() ? "si" : "NO") << ")\n";
    delete s;

    Reserva* rs = new Reserva[1000];
    ArbolAVL a;
    for (int i = 0; i < 1000; i++) {
        string id = to_string(i + 1);
        while (id.size() < 6) id = "0" + id;
        rs[i].idReserva = "CRE-" + id;
        rs[i].fecha = fechaDeDia(i / 24);
        string hh = to_string(i % 24);
        rs[i].hora = (hh.size() < 2 ? "0" + hh : hh) + ":00";
        a.insertar(&rs[i]);
    }
    cout << setw(62) << "Altura del AVL tras 1000 inserciones con fechas crecientes" << a.altura() << "  (valido: " << (a.verificar() ? "si" : "NO") << ")\n";
    delete[] rs;
    return 0;
}
