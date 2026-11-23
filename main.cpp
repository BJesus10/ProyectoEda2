// main.cpp - menu del sistema de reservas aereas.
// Compilar: g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp main.cpp -o reservas
// Ejecutar en la carpeta que contiene vuelos.csv y reservas_masivo.csv.
#include <iostream>
#include <string>
#include <cstdlib>
#include "SistemaReservas.h"
using namespace std;

// ---------------------------------------------------------------- entrada
// Lee una linea mostrando el mensaje. Devuelve false si se acabo la entrada.
static bool leer(const string& mensaje, string& destino) {
    cout << mensaje;
    if (!getline(cin, destino)) return false;
    if (!destino.empty() && destino[destino.size() - 1] == '\r') destino.erase(destino.size() - 1);
    return true;
}

static int aEntero(const string& s) { return s.empty() ? 0 : atoi(s.c_str()); }

static bool fechaValida(const string& f) {       // formato AAAA-MM-DD
    if (f.size() != 10 || f[4] != '-' || f[7] != '-') return false;
    for (size_t i = 0; i < f.size(); i++)
        if (i != 4 && i != 7 && (f[i] < '0' || f[i] > '9')) return false;
    return true;
}

static void linea() { cout << "------------------------------------------------------------\n"; }

// ------------------------------------------------------------ funcionalidades
static void opcion1(SistemaReservas& s, bool& cargado) {
    if (cargado) { cout << "Los datos ya fueron cargados.\n"; return; }
    string av, ar;
    if (!leer("Archivo de vuelos [vuelos.csv]: ", av)) return;
    if (!leer("Archivo de reservas [reservas_masivo.csv]: ", ar)) return;
    if (av.empty()) av = "vuelos.csv";
    if (ar.empty()) ar = "reservas_masivo.csv";
    int nv = 0;
    if (!s.cargarVuelos(av, nv)) { cout << "No se pudo abrir " << av << "\n"; return; }
    ResumenCarga rc;
    if (!s.cargarReservas(ar, rc)) { cout << "No se pudo abrir " << ar << "\n"; return; }
    cargado = true;
    cout << "Vuelos cargados: " << nv << "\n";
    cout << "Reservas leidas: " << rc.leidas << " (rechazadas: " << rc.rechazadas << ")\n";
    cout << "Insertadas en la hash : " << rc.enHash << "\n";
    cout << "Insertadas en el arbol: " << rc.enArbol << "\n";
    cout << "Insertadas en heaps   : " << rc.enHeaps << "  (listas de espera)\n";
    for (int i = 0; i < s.numVuelos(); i++)
        if (s.esperaTam(i) > 0) cout << "   Heap del vuelo " << s.vuelo(i).codigoVuelo << ": " << s.esperaTam(i) << " pasajeros\n";
}

static void opcion2(SistemaReservas& s) {
    string id, dni, nom, cod, c, k, p, e;
    if (!leer("ID de la reserva: ", id) || !leer("DNI: ", dni) || !leer("Pasajero: ", nom) || !leer("Codigo de vuelo: ", cod)) return;
    if (!leer("Clase (1=Economica, 2=Premium, 3=Ejecutiva): ", c) || !leer("Categoria (1=Regular, 2=Frecuente, 3=Premium): ", k)) return;
    if (!leer("Precio: ", p) || !leer("Estado si hay asiento (1=Confirmada, 2=Pendiente): ", e)) return;
    int ci = aEntero(c), ki = aEntero(k), ei = aEntero(e);
    double precio = atof(p.c_str());
    if (ci < 1 || ci > 3 || ki < 1 || ki > 3 || (ei != 1 && ei != 2) || precio <= 0) { cout << "Datos invalidos.\n"; return; }
    Clase clase = ci == 1 ? ECONOMICA : (ci == 2 ? PREMIUM_CLASE : EJECUTIVA);
    Categoria cat = ki == 1 ? REGULAR : (ki == 2 ? FRECUENTE : PREMIUM);
    Reserva* r = nullptr;
    switch (s.registrar(id, dni, nom, cod, clase, cat, precio, ei == 1 ? CONFIRMADA : PENDIENTE, &r)) {
        case SistemaReservas::REG_ASIENTO:
            cout << "Reserva registrada con asiento. Estado: " << textoEstado(r->estado) << "\n"; break;
        case SistemaReservas::REG_ESPERA:
            cout << "El vuelo esta lleno: la reserva quedo EnEspera (llegada #" << r->ordenLlegada << ").\n"; break;
        case SistemaReservas::REG_ID_EXISTE:
            cout << "Error: ya existe una reserva con ese ID.\n"; break;
        case SistemaReservas::REG_VUELO_NO_EXISTE:
            cout << "Error: el vuelo " << cod << " no existe.\n"; break;
        default:
            cout << "Datos invalidos.\n";
    }
    if (r) SistemaReservas::imprimirReserva(r);
}

static void opcion3(SistemaReservas& s) {
    string id;
    if (!leer("ID de la reserva: ", id)) return;
    Reserva* r = s.buscarPorId(id);
    if (r) SistemaReservas::imprimirReserva(r); else cout << "No existe una reserva con el ID " << id << "\n";
}

static void opcion4(SistemaReservas& s) {
    string t;
    if (!leer("Cuantas reservas mostrar [20]: ", t)) return;
    int n = t.empty() ? 20 : aEntero(t);
    if (n < 1) { cout << "Cantidad invalida.\n"; return; }
    if (n > 500) n = 500;
    Reserva** buf = new Reserva*[n];
    int k = s.listadoCronologico(buf, n);
    for (int i = 0; i < k; i++) { cout << (i + 1) << ". "; SistemaReservas::imprimirReserva(buf[i]); }
    delete[] buf;
}

static void opcion5(SistemaReservas& s) {
    string a, b;
    if (!leer("Fecha inicial (AAAA-MM-DD): ", a) || !leer("Fecha final   (AAAA-MM-DD): ", b)) return;
    if (!fechaValida(a) || !fechaValida(b) || a > b) { cout << "Fechas invalidas (formato AAAA-MM-DD, inicial <= final).\n"; return; }
    Reserva* muestra[10];
    int enM = 0;
    int total = s.consultaRango(a, b, muestra, 10, enM);
    cout << "Reservas encontradas entre " << a << " y " << b << ": " << total << "\n";
    if (enM > 0) cout << "Muestra (primeras " << enM << "):\n";
    for (int i = 0; i < enM; i++) { cout << "  "; SistemaReservas::imprimirReserva(muestra[i]); }
}

static void opcion6(SistemaReservas& s) {
    string id, e;
    if (!leer("ID de la reserva: ", id)) return;
    Reserva* r = s.buscarPorId(id);
    if (!r) { cout << "No existe una reserva con el ID " << id << "\n"; return; }
    cout << "Estado actual: " << textoEstado(r->estado) << "\n";
    if (!leer("Nuevo estado (1=Confirmada, 2=CheckIn): ", e)) return;
    int ei = aEntero(e);
    if (ei != 1 && ei != 2) { cout << "Opcion invalida.\n"; return; }
    if (s.actualizarEstado(id, ei == 1 ? CONFIRMADA : CHECKIN) == SistemaReservas::ACT_OK)
        cout << "Estado actualizado a " << textoEstado(r->estado) << ".\n";
    else
        cout << "Transicion no permitida (solo Pendiente->Confirmada y Confirmada->CheckIn).\n";
}

static void opcion7(SistemaReservas& s) {
    string id;
    if (!leer("ID de la reserva a cancelar: ", id)) return;
    Reserva* promovida = nullptr;
    switch (s.cancelar(id, &promovida)) {
        case SistemaReservas::CAN_NO_EXISTE:     cout << "No existe una reserva con el ID " << id << "\n"; break;
        case SistemaReservas::CAN_YA_CANCELADA:  cout << "La reserva ya estaba cancelada.\n"; break;
        case SistemaReservas::CAN_OK_ESPERA:     cout << "Reserva cancelada (estaba EnEspera; se retiro del heap).\n"; break;
        case SistemaReservas::CAN_OK_ASIENTO:
            cout << "Reserva cancelada; se libero un asiento.\n";
            if (promovida) { cout << "Promovido desde la lista de espera:\n  "; SistemaReservas::imprimirReserva(promovida); }
            else cout << "El vuelo no tenia lista de espera.\n";
            break;
    }
}

static void opcion8(SistemaReservas& s) {
    string id;
    if (!leer("ID de la reserva a eliminar fisicamente: ", id)) return;
    switch (s.eliminarFisico(id)) {
        case SistemaReservas::ELI_OK:           cout << "Reserva eliminada de la hash y del arbol.\n"; break;
        case SistemaReservas::ELI_NO_EXISTE:    cout << "No existe una reserva con el ID " << id << "\n"; break;
        case SistemaReservas::ELI_NO_CANCELADA: cout << "Solo se pueden eliminar reservas Canceladas.\n"; break;
    }
}

static void opcion9(SistemaReservas& s) {
    string cod;
    if (!leer("Codigo de vuelo: ", cod)) return;
    int iv = s.vueloIndice(cod);
    if (iv < 0) { cout << "El vuelo " << cod << " no existe.\n"; return; }
    const Vuelo& v = s.vuelo(iv);
    cout << "Vuelo " << v.codigoVuelo << " " << v.origen << "->" << v.destino << " " << v.fecha << " " << v.hora
         << " | asientos " << v.asientosOcupados << "/" << v.capacidad << " | en espera: " << s.esperaTam(iv) << "\n";
    int n = s.esperaTam(iv);
    if (n == 0) { cout << "La lista de espera esta vacia.\n"; return; }
    cout << "Siguiente a atender: "; SistemaReservas::imprimirReserva(s.siguienteEnEspera(iv));
    Reserva** lista = new Reserva*[n];
    int k = s.listaEspera(iv, lista, n);
    cout << "Lista completa en orden de prioridad:\n";
    for (int i = 0; i < k; i++)
        cout << "  " << (i + 1) << ". " << lista[i]->idReserva << " | " << lista[i]->pasajero << " | "
             << textoCategoria(lista[i]->categoria) << " | llegada #" << lista[i]->ordenLlegada << "\n";
    delete[] lista;
}

static void opcion11(const SistemaReservas& s) {
    cout << "Reservas en la hash: " << s.tamanioHash() << " | en el arbol: " << s.tamanioArbol() << "\n";
    cout << "Factor de carga de la hash: " << s.factorCarga() << " (tabla de " << s.tamTabla() << ")\n";
    cout << "Cadena mas larga de la hash: " << s.cadenaMasLarga() << "\n";
    cout << "Altura del AVL: " << s.alturaArbol() << " (valido: " << (s.arbolValido() ? "si" : "NO") << ")\n";
}

// ------------------------------------------------------------------- menu
static void mostrarMenu() {
    linea();
    cout << " SISTEMA DE RESERVAS AEREAS\n";
    linea();
    cout << " 1. Carga masiva de vuelos y reservas\n"
         << " 2. Registrar nueva reserva\n"
         << " 3. Buscar reserva por ID\n"
         << " 4. Listado cronologico\n"
         << " 5. Consulta por rango de fechas\n"
         << " 6. Actualizar estado de una reserva\n"
         << " 7. Cancelar reserva\n"
         << " 8. Eliminar fisicamente una reserva cancelada\n"
         << " 9. Ver lista de espera de un vuelo\n"
         << "10. Estadisticas generales\n"
         << "11. Metricas de las estructuras\n"
         << " 0. Salir\n";
    linea();
}

int main() {
    SistemaReservas sistema;
    bool cargado = false;
    string entrada;
    while (true) {
        mostrarMenu();
        if (!leer("Opcion: ", entrada)) break;
        int op = aEntero(entrada);
        if (op == 0 && entrada != "0") { cout << "Opcion invalida.\n"; continue; }
        if (op == 0) break;
        if (op != 1 && !cargado) { cout << "Primero cargue los datos (opcion 1).\n"; continue; }
        cout << "\n";
        switch (op) {
            case 1:  opcion1(sistema, cargado); break;
            case 2:  opcion2(sistema); break;
            case 3:  opcion3(sistema); break;
            case 4:  opcion4(sistema); break;
            case 5:  opcion5(sistema); break;
            case 6:  opcion6(sistema); break;
            case 7:  opcion7(sistema); break;
            case 8:  opcion8(sistema); break;
            case 9:  opcion9(sistema); break;
            case 10: SistemaReservas::imprimirEstadisticas(sistema, sistema.estadisticas()); break;
            case 11: opcion11(sistema); break;
            default: cout << "Opcion invalida.\n";
        }
        cout << "\n";
    }
    cout << "Hasta luego.\n";
    return 0;
}
