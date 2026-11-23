// pruebas.cpp - escenarios de prueba del enunciado, con verificacion automatica.
// Compilar: g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp pruebas.cpp -o pruebas
// Ejecutar en la carpeta que contiene vuelos.csv, reservas_masivo.csv y reservas_pequeno.csv.
#include <iostream>
#include <fstream>
#include <string>
#include "SistemaReservas.h"
#include "hash_util.h"
using namespace std;

static int fallos = 0, aciertos = 0;
#define VERIFICAR(cond, msg) do { if (cond) { aciertos++; cout << "  [OK]    " << msg << "\n"; } \
                                   else { fallos++; cout << "  [FALLA] " << msg << "\n"; } } while (0)

static void titulo(const string& t) { cout << "\n=== " << t << " ===\n"; }

static bool menorIgual(const Reserva* a, const Reserva* b) {   // orden (fecha, hora, id)
    if (a->fecha != b->fecha) return a->fecha < b->fecha;
    if (a->hora != b->hora) return a->hora < b->hora;
    return a->idReserva <= b->idReserva;
}

// primera, intermedia y ultima reserva del archivo (en orden del archivo)
static void idsDelArchivo(const string& archivo, string& primera, string& media, string& ultima) {
    int total = 0;
    string linea;
    { ifstream in(archivo.c_str()); while (getline(in, linea)) if (!linea.empty() && linea.compare(0, 9, "idReserva") != 0) total++; }
    int k = 0;
    ifstream in(archivo.c_str());
    while (getline(in, linea)) {
        if (linea.empty() || linea.compare(0, 9, "idReserva") == 0) continue;
        string id = linea.substr(0, linea.find(','));
        if (k == 0) primera = id;
        if (k == total / 2) media = id;
        ultima = id;
        k++;
    }
}

static int contarEnEspera(const string& archivo) {
    int n = 0;
    string linea;
    ifstream in(archivo.c_str());
    while (getline(in, linea)) {
        size_t p = linea.rfind(',');
        if (p != string::npos && linea.compare(p + 1, 8, "EnEspera") == 0) n++;
    }
    return n;
}

static string fechaDeDia(int d) {      // 0 -> 2026-12-01 ... 40 -> 2027-01-10
    int dia = d < 31 ? d + 1 : d - 30;
    string s = to_string(dia);
    if (s.size() < 2) s = "0" + s;
    return (d < 31 ? "2026-12-" : "2027-01-") + s;
}

int main() {
    SistemaReservas sis;
    int nv = 0;
    ResumenCarga rc;

    // ------------------------------------------------------------------ 1
    titulo("1. Carga masiva (vuelos.csv + reservas_masivo.csv)");
    VERIFICAR(sis.cargarVuelos("vuelos.csv", nv), "vuelos.csv abierto");
    VERIFICAR(sis.cargarReservas("reservas_masivo.csv", rc), "reservas_masivo.csv abierto");
    cout << "  Vuelos cargados: " << nv << "\n";
    cout << "  Reservas leidas: " << rc.leidas << " | rechazadas: " << rc.rechazadas << "\n";
    cout << "  Insertadas en la hash: " << sis.tamanioHash() << " | en el arbol: " << sis.tamanioArbol() << "\n";
    int enHeaps = 0;
    for (int i = 0; i < sis.numVuelos(); i++) {
        if (sis.esperaTam(i) > 0) {
            cout << "  Heap del vuelo " << sis.vuelo(i).codigoVuelo << ": " << sis.esperaTam(i) << " en espera\n";
            enHeaps += sis.esperaTam(i);
        }
    }
    VERIFICAR(rc.leidas == 10000 && rc.rechazadas == 0, "10 000 reservas leidas y ninguna rechazada");
    VERIFICAR(sis.tamanioHash() == 10000 && sis.tamanioArbol() == 10000, "hash y arbol con 10 000 reservas");
    VERIFICAR(enHeaps == contarEnEspera("reservas_masivo.csv") && enHeaps == rc.enHeaps, "heaps = reservas EnEspera del archivo");
    VERIFICAR(sis.arbolValido(), "invariante AVL valido");

    const int N = sis.tamanioArbol();
    Reserva** todas = new Reserva*[N + 500];
    int nt = sis.copiarTodas(todas, N + 500);
    VERIFICAR(nt == N, "recorrido en orden entrega las 10 000 reservas");

    // ------------------------------------------------------------------ 2
    titulo("2. Busqueda por ID (hash)");
    string p, m, u;
    idsDelArchivo("reservas_masivo.csv", p, m, u);
    const string ids[3] = {p, m, u};
    const char* nombres[3] = {"primera", "intermedia", "ultima"};
    for (int i = 0; i < 3; i++) {
        Reserva* r = sis.buscarPorId(ids[i]);
        cout << "  " << nombres[i] << " (" << ids[i] << "): "; if (r) SistemaReservas::imprimirReserva(r); else cout << "no encontrada\n";
        VERIFICAR(r != nullptr && r->idReserva == ids[i], string("encontrada la reserva ") + nombres[i]);
    }
    VERIFICAR(sis.buscarPorId("XXX-000000") == nullptr, "ID inexistente devuelve no encontrado");

    // ------------------------------------------------------------------ 3
    titulo("3. Ordenamiento cronologico (primeras 20)");
    Reserva* buf[200];
    int k = sis.listadoCronologico(buf, 20);
    for (int i = 0; i < k; i++) { cout << "  " << (i + 1) << ". "; SistemaReservas::imprimirReserva(buf[i]); }
    bool ordenado = true;
    for (int i = 1; i < nt; i++) if (!menorIgual(todas[i - 1], todas[i])) ordenado = false;
    VERIFICAR(k == 20, "se obtuvieron 20 reservas");
    VERIFICAR(ordenado, "las 10 000 reservas salen ordenadas por (fecha, hora, id)");

    cout << "  -- Archivo pequeno (para comprobar a mano) --\n";
    {
        SistemaReservas chico;
        int nvc; ResumenCarga rcc;
        chico.cargarVuelos("vuelos.csv", nvc);
        chico.cargarReservas("reservas_pequeno.csv", rcc);
        Reserva* b2[40];
        int kc = chico.listadoCronologico(b2, 20);
        for (int i = 0; i < kc; i++) { cout << "  " << (i + 1) << ". "; SistemaReservas::imprimirReserva(b2[i]); }
        VERIFICAR(rcc.enHash == 30 && kc == 20, "archivo pequeno: 30 cargadas, listado de 20");
        bool ok = true;
        for (int i = 1; i < kc; i++) if (!menorIgual(b2[i - 1], b2[i])) ok = false;
        VERIFICAR(ok, "archivo pequeno: orden cronologico correcto");
    }

    // ------------------------------------------------------------------ 4
    titulo("4. Consulta por rango de fechas");
    const char* rangos[3][2] = {{"2026-12-01", "2026-12-07"}, {"2027-01-01", "2027-01-31"}, {"2030-01-01", "2030-01-31"}};
    const char* desc[3] = {"una semana (1-7 dic 2026)", "un mes (enero 2027)", "sin resultados (enero 2030)"};
    for (int i = 0; i < 3; i++) {
        int enM = 0;
        int total = sis.consultaRango(rangos[i][0], rangos[i][1], buf, 5, enM);
        int bruta = 0;
        for (int j = 0; j < nt; j++) if (todas[j]->fecha >= rangos[i][0] && todas[j]->fecha <= rangos[i][1]) bruta++;
        cout << "  Rango " << desc[i] << ": " << total << " reservas; muestra:\n";
        for (int j = 0; j < enM; j++) { cout << "     "; SistemaReservas::imprimirReserva(buf[j]); }
        VERIFICAR(total == bruta, string("conteo del arbol = conteo por fuerza bruta (") + to_string(bruta) + ") en " + desc[i]);
    }

    // ------------------------------------------------------------------ 5
    titulo("5. Colisiones: al menos 50 IDs en la misma posicion de la hash");
    int tam = sis.tamTabla();
    int* cont = new int[tam];
    for (int i = 0; i < tam; i++) cont[i] = 0;
    for (int i = 0; i < nt; i++) cont[hashId(todas[i]->idReserva, tam)]++;
    int mejor = 0;
    for (int i = 1; i < tam; i++) if (cont[i] > cont[mejor]) mejor = i;
    cout << "  Posicion con mas IDs: " << mejor << " (" << cont[mejor] << " IDs)\n";
    int hallados = 0;
    for (int i = 0; i < nt; i++)
        if (hashId(todas[i]->idReserva, tam) == mejor && sis.buscarPorId(todas[i]->idReserva) == todas[i]) hallados++;
    VERIFICAR(cont[mejor] >= 50, "al menos 50 IDs caen en la misma posicion");
    VERIFICAR(hallados == cont[mejor], "los " + to_string(cont[mejor]) + " IDs de esa cadena se encuentran");
    VERIFICAR(sis.cadenaMasLarga() == cont[mejor], "cadenaMasLarga() coincide con el conteo propio");
    cout << "  Factor de carga: " << sis.factorCarga() << " (tabla de " << tam << ")\n";
    delete[] cont;

    // ------------------------------------------------------------------ 6
    titulo("6. Insercion individual");
    int vl = -1, vf = -1;      // un vuelo con asientos libres y uno lleno
    for (int i = 0; i < sis.numVuelos(); i++) {
        if (vl < 0 && sis.vuelo(i).asientosOcupados < sis.vuelo(i).capacidad - 2 && sis.esperaTam(i) == 0) vl = i;
        if (vf < 0 && sis.vuelo(i).asientosOcupados == sis.vuelo(i).capacidad && sis.esperaTam(i) > 0) vf = i;
    }
    const Vuelo vuelL = sis.vuelo(vl);
    int enM = 0;
    int antes = sis.consultaRango(vuelL.fecha, vuelL.fecha, buf, 0, enM);
    Reserva* nueva = nullptr;
    auto res1 = sis.registrar("NUE-000001", "70000001", "Prueba Uno", vuelL.codigoVuelo, ECONOMICA, REGULAR, 199.9, CONFIRMADA, &nueva);
    VERIFICAR(res1 == SistemaReservas::REG_ASIENTO && nueva != nullptr && nueva->estado == CONFIRMADA, "vuelo con asientos libres: queda Confirmada");
    VERIFICAR(sis.buscarPorId("NUE-000001") == nueva, "aparece en la busqueda por ID");
    VERIFICAR(sis.buscarEnArbol(nueva->fecha, nueva->hora, nueva->idReserva) == nueva, "aparece en el arbol (busqueda por clave)");
    VERIFICAR(sis.consultaRango(vuelL.fecha, vuelL.fecha, buf, 0, enM) == antes + 1, "la consulta por rango de ese dia suma 1");
    int espAntes = sis.esperaTam(vf);
    Reserva* enEsp = nullptr;
    auto res2 = sis.registrar("NUE-000002", "70000002", "Prueba Dos", sis.vuelo(vf).codigoVuelo, EJECUTIVA, PREMIUM, 500.0, CONFIRMADA, &enEsp);
    VERIFICAR(res2 == SistemaReservas::REG_ESPERA && enEsp != nullptr && enEsp->estado == EN_ESPERA, "vuelo lleno: queda EnEspera");
    VERIFICAR(sis.esperaTam(vf) == espAntes + 1 && sis.enHeap(vf, enEsp) && sis.heapValido(vf), "entra al heap del vuelo y el heap sigue valido");
    VERIFICAR(sis.registrar("NUE-000001", "1", "Dup", vuelL.codigoVuelo, ECONOMICA, REGULAR, 1, CONFIRMADA) == SistemaReservas::REG_ID_EXISTE, "ID repetido rechazado");
    VERIFICAR(sis.registrar("NUE-000003", "1", "SinVuelo", "ZZ999", ECONOMICA, REGULAR, 1, CONFIRMADA) == SistemaReservas::REG_VUELO_NO_EXISTE, "vuelo inexistente rechazado");
    VERIFICAR(sis.tamanioHash() == 10002 && sis.tamanioArbol() == 10002, "hash y arbol con 10 002 reservas");

    // ------------------------------------------------------------------ 7
    titulo("7. Actualizacion de estado, cancelacion y eliminacion fisica");
    // refrescar la lista de reservas
    nt = sis.copiarTodas(todas, N + 500);
    Reserva* pend[10]; int np = 0;
    for (int i = 0; i < nt && np < 10; i++) if (todas[i]->estado == PENDIENTE) pend[np++] = todas[i];
    int okAct = 0;
    for (int i = 0; i < 10; i++) if (sis.actualizarEstado(pend[i]->idReserva, CONFIRMADA) == SistemaReservas::ACT_OK && pend[i]->estado == CONFIRMADA) okAct++;
    VERIFICAR(okAct == 10, "10 reservas Pendiente -> Confirmada");
    okAct = 0;
    for (int i = 0; i < 10; i++) if (sis.actualizarEstado(pend[i]->idReserva, CHECKIN) == SistemaReservas::ACT_OK && pend[i]->estado == CHECKIN) okAct++;
    VERIFICAR(okAct == 10, "las mismas 10 pasan de Confirmada -> CheckIn");
    VERIFICAR(sis.actualizarEstado(pend[0]->idReserva, CONFIRMADA) == SistemaReservas::ACT_TRANSICION_INVALIDA, "CheckIn -> Confirmada rechazada");
    VERIFICAR(sis.actualizarEstado("XXX-000000", CONFIRMADA) == SistemaReservas::ACT_NO_EXISTE, "actualizar un ID inexistente da error");
    {   // el cambio se ve tambien por el arbol (comparten el puntero)
        Reserva* viaArbol = sis.buscarEnArbol(pend[0]->fecha, pend[0]->hora, pend[0]->idReserva);
        VERIFICAR(viaArbol == pend[0] && viaArbol->estado == CHECKIN, "el cambio de estado se refleja en el arbol");
    }

    // cancelar 10 reservas con asiento en vuelos NO llenos, y luego eliminarlas fisicamente
    string idsCanc[10]; int nc = 0;
    for (int i = 0; i < nt && nc < 10; i++) {
        Reserva* r = todas[i];
        int iv = sis.vueloIndice(r->codigoVuelo);
        if (ocupaAsiento(r->estado) && sis.vuelo(iv).asientosOcupados < sis.vuelo(iv).capacidad && sis.esperaTam(iv) == 0) idsCanc[nc++] = r->idReserva;
    }
    int canceladas = 0;
    for (int i = 0; i < 10; i++) {
        Reserva* r = sis.buscarPorId(idsCanc[i]);
        int iv = sis.vueloIndice(r->codigoVuelo);
        int ocupAntes = sis.vuelo(iv).asientosOcupados;
        if (sis.cancelar(idsCanc[i]) == SistemaReservas::CAN_OK_ASIENTO && r->estado == CANCELADA &&
            sis.vuelo(iv).asientosOcupados == ocupAntes - 1) canceladas++;
    }
    VERIFICAR(canceladas == 10, "10 reservas canceladas; cada una libera su asiento");
    VERIFICAR(sis.cancelar(idsCanc[0]) == SistemaReservas::CAN_YA_CANCELADA, "cancelar dos veces la misma da error");
    VERIFICAR(sis.actualizarEstado(idsCanc[0], CONFIRMADA) == SistemaReservas::ACT_TRANSICION_INVALIDA, "una Cancelada no se reactiva");
    {   // una reserva que NO esta Cancelada (y que no es una de las canceladas arriba)
        Reserva* noCanc = nullptr;
        for (int i = 0; i < nt && !noCanc; i++) {
            bool esCancelada = false;
            for (int j = 0; j < 10; j++) if (todas[i]->idReserva == idsCanc[j]) esCancelada = true;
            if (!esCancelada && todas[i]->estado != CANCELADA) noCanc = todas[i];
        }
        VERIFICAR(sis.eliminarFisico(noCanc->idReserva) == SistemaReservas::ELI_NO_CANCELADA && sis.buscarPorId(noCanc->idReserva) == noCanc,
                  "no se elimina fisicamente una reserva que no esta Cancelada");
    }
    int eliminadas = 0;
    for (int i = 0; i < 10; i++) if (sis.eliminarFisico(idsCanc[i]) == SistemaReservas::ELI_OK) eliminadas++;
    VERIFICAR(eliminadas == 10, "10 reservas eliminadas fisicamente");
    bool ausentes = true;
    for (int i = 0; i < 10; i++) if (sis.buscarPorId(idsCanc[i]) != nullptr) ausentes = false;
    VERIFICAR(ausentes, "ya no aparecen en la busqueda por ID");
    int restantes = sis.copiarTodas(todas, N + 500);
    bool fueraArbol = true;
    for (int i = 0; i < restantes; i++) for (int j = 0; j < 10; j++) if (todas[i]->idReserva == idsCanc[j]) fueraArbol = false;
    VERIFICAR(fueraArbol && restantes == 9992 && sis.tamanioHash() == 9992 && sis.tamanioArbol() == 9992, "ya no aparecen en el arbol; hash y arbol con 9 992");
    VERIFICAR(sis.arbolValido(), "invariante AVL valido tras las eliminaciones");

    // ------------------------------------------------------------------ 8
    titulo("8. Lista de espera");
    int vw = -1;      // un vuelo lleno con espera >= 5 distinto del usado antes
    for (int i = 0; i < sis.numVuelos(); i++) if (i != vf && sis.esperaTam(i) >= 5 && sis.vuelo(i).asientosOcupados == sis.vuelo(i).capacidad) { vw = i; break; }
    Reserva* lista[64];
    int ln = sis.listaEspera(vw, lista, 64);
    cout << "  Vuelo " << sis.vuelo(vw).codigoVuelo << " - siguiente a atender: "; SistemaReservas::imprimirReserva(sis.siguienteEnEspera(vw));
    cout << "  Lista completa en orden de prioridad (" << ln << "):\n";
    for (int i = 0; i < ln; i++) cout << "     " << (i + 1) << ". " << lista[i]->idReserva << " | " << textoCategoria(lista[i]->categoria) << " | llegada #" << lista[i]->ordenLlegada << "\n";
    bool prioridadOk = (sis.siguienteEnEspera(vw) == lista[0]);
    for (int i = 1; i < ln; i++) {
        if (lista[i - 1]->categoria < lista[i]->categoria) prioridadOk = false;
        if (lista[i - 1]->categoria == lista[i]->categoria && lista[i - 1]->ordenLlegada > lista[i]->ordenLlegada) prioridadOk = false;
    }
    VERIFICAR(prioridadOk, "orden: Premium > Frecuente > Regular y, en empate, el que llego antes");
    VERIFICAR(sis.esperaTam(vw) == ln && sis.heapValido(vw), "mostrar la lista no altera el heap");

    // caso A: cancelar una reserva con asiento -> se promueve al primero de la espera
    Reserva* conAsiento = nullptr;
    int nt2 = sis.copiarTodas(todas, N + 500);
    for (int i = 0; i < nt2 && !conAsiento; i++)
        if (ocupaAsiento(todas[i]->estado) && todas[i]->codigoVuelo == sis.vuelo(vw).codigoVuelo) conAsiento = todas[i];
    Reserva* esperado = lista[0];
    Reserva* promovida = nullptr;
    int ocupAntes = sis.vuelo(vw).asientosOcupados;
    auto rc2 = sis.cancelar(conAsiento->idReserva, &promovida);
    VERIFICAR(rc2 == SistemaReservas::CAN_OK_ASIENTO && promovida == esperado, "caso A: se promueve al pasajero de mayor prioridad (" + esperado->idReserva + ")");
    VERIFICAR(promovida->estado == CONFIRMADA && !sis.enHeap(vw, promovida), "el promovido queda Confirmada y sale del heap");
    VERIFICAR(sis.vuelo(vw).asientosOcupados == ocupAntes && sis.esperaTam(vw) == ln - 1 && sis.heapValido(vw), "el vuelo sigue lleno y el heap es valido");

    // caso B: cancelar una EnEspera que NO esta en la raiz
    Reserva* l2[64];
    int ln2 = sis.listaEspera(vw, l2, 64);
    Reserva* medio = l2[ln2 / 2];
    VERIFICAR(medio != sis.siguienteEnEspera(vw), "la reserva elegida no esta en la raiz");
    auto rc3 = sis.cancelar(medio->idReserva);
    VERIFICAR(rc3 == SistemaReservas::CAN_OK_ESPERA && medio->estado == CANCELADA && !sis.enHeap(vw, medio), "caso B: se retira del heap y queda Cancelada");
    Reserva* l3[64];
    int ln3 = sis.listaEspera(vw, l3, 64);
    bool mismoOrden = (ln3 == ln2 - 1);
    for (int i = 0, j = 0; i < ln2 && mismoOrden; i++) { if (l2[i] == medio) continue; if (l3[j++] != l2[i]) mismoOrden = false; }
    VERIFICAR(sis.heapValido(vw) && mismoOrden, "el heap sigue siendo valido y el resto conserva su orden");

    // esfuerzo: vaciar las demas listas retirando en orden inverso, comprobando el heap en cada paso
    bool todoValido = true;
    int retirados = 0;
    for (int i = 0; i < sis.numVuelos(); i++) {
        if (i == vw || sis.esperaTam(i) == 0) continue;
        Reserva* l[64];
        int n2 = sis.listaEspera(i, l, 64);
        for (int j = n2 - 1; j >= 0; j--) {
            if (sis.cancelar(l[j]->idReserva) != SistemaReservas::CAN_OK_ESPERA) todoValido = false;
            if (!sis.heapValido(i)) todoValido = false;
            retirados++;
        }
        if (sis.esperaTam(i) != 0) todoValido = false;
    }
    VERIFICAR(todoValido, "retirar " + to_string(retirados) + " reservas EnEspera en orden inverso mantiene todos los heaps validos");

    // ------------------------------------------------------------------ 9
    titulo("9. Orden desfavorable: 1 000 reservas con fechas estrictamente crecientes");
    {
        Reserva* rs = new Reserva[1000];
        ArbolAVL a;
        for (int i = 0; i < 1000; i++) {
            string id = to_string(i + 1);
            while (id.size() < 6) id = "0" + id;
            rs[i].idReserva = "CRE-" + id;
            rs[i].fecha = fechaDeDia(i / 24);
            string h = to_string(i % 24);
            rs[i].hora = (h.size() < 2 ? "0" + h : h) + ":00";
            a.insertar(&rs[i]);
        }
        cout << "  Altura del AVL: " << a.altura() << " (un arbol sin balancear tendria 1000)\n";
        VERIFICAR(a.tamanio() == 1000 && a.verificar() && a.altura() <= 14, "altura logaritmica (<= 14) y arbol valido");
        delete[] rs;
    }

    // ------------------------------------------------------------------ 10
    titulo("10. Estadisticas generales");
    Estadisticas e = sis.estadisticas();
    SistemaReservas::imprimirEstadisticas(sis, e);
    VERIFICAR(e.total == sis.tamanioArbol(), "el total de las estadisticas coincide con el arbol");
    int sumaEstados = 0;
    for (int i = 0; i < 5; i++) sumaEstados += e.porEstado[i];
    VERIFICAR(sumaEstados == e.total, "la suma por estado es igual al total");

    // ------------------------------------------------------------------ consistencia global
    titulo("Consistencia global hash - arbol - heaps");
    int n3 = sis.copiarTodas(todas, N + 500);
    bool cons = (n3 == sis.tamanioHash());
    int enEsperaEnArbol = 0;
    for (int i = 0; i < n3; i++) {
        if (sis.buscarPorId(todas[i]->idReserva) != todas[i]) cons = false;
        if (todas[i]->estado == EN_ESPERA) {
            enEsperaEnArbol++;
            if (!sis.enHeap(sis.vueloIndice(todas[i]->codigoVuelo), todas[i])) cons = false;
        }
    }
    int enHeapsAhora = 0;
    for (int i = 0; i < sis.numVuelos(); i++) { enHeapsAhora += sis.esperaTam(i); if (!sis.heapValido(i)) cons = false; }
    VERIFICAR(cons && enEsperaEnArbol == enHeapsAhora, "toda reserva del arbol esta en la hash; toda EnEspera esta en su heap (y viceversa)");
    bool asientosOk = true;
    for (int i = 0; i < sis.numVuelos(); i++) {
        int c = 0;
        for (int j = 0; j < n3; j++) if (todas[j]->codigoVuelo == sis.vuelo(i).codigoVuelo && ocupaAsiento(todas[j]->estado)) c++;
        if (c != sis.vuelo(i).asientosOcupados || c > sis.vuelo(i).capacidad) asientosOk = false;
    }
    VERIFICAR(asientosOk, "asientosOcupados de cada vuelo = reservas con asiento, sin superar la capacidad");

    delete[] todas;
    cout << "\nRESUMEN: " << aciertos << " verificaciones correctas, " << fallos << " fallidas\n";
    return fallos == 0 ? 0 : 1;
}
