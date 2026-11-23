Sistema de reservas aereas - EDA 2
==================================

Modulos
  modelo.h            Reserva, Vuelo y enumeraciones
  config.h            TAM_TABLA_HASH (10007) y MAX_VUELOS
  hash_util.h         funcion hash djb2 sobre idReserva (uint32_t, igual en cualquier plataforma)
  TablaHash.h         hash con encadenamiento; es dueña de las Reserva
  ArbolAVL.h/.cpp     AVL por (fecha, hora, idReserva)
  HeapEspera.h        max-heap de la lista de espera de un vuelo
  SistemaReservas.*   coordina las tres estructuras (funcionalidades 1 a 10)
  generador.cpp       genera vuelos.csv, reservas_pequeno.csv y reservas_masivo.csv
  pruebas.cpp         escenarios del enunciado con verificacion automatica
  mediciones.cpp      tiempos (chrono) y metricas para el informe
  main.cpp            menu con las 10 funcionalidades (+ opcion 11: metricas de las estructuras)

Compilar y ejecutar (desde esta carpeta)
  g++ -std=c++11 -O2 generador.cpp -o generador
  cd datos && ../generador && cd ..            (opcional: ya vienen generados en datos/)
  g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp main.cpp -o reservas        (el programa con menu)
  g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp pruebas.cpp -o pruebas
  g++ -std=c++11 -O2 ArbolAVL.cpp SistemaReservas.cpp mediciones.cpp -o mediciones
  cd datos && ../reservas        (o ../pruebas, ../mediciones)

Nota: generador, reservas, pruebas y mediciones tienen cada uno su propio main(); en un IDE
cada uno debe ser un proyecto (o target) distinto. Los CSV deben estar en la carpeta de trabajo
del programa.

Memoria (opcional)
  g++ -std=c++11 -g -fsanitize=address,undefined ArbolAVL.cpp SistemaReservas.cpp pruebas.cpp -o pruebas_asan

IMPORTANTE: si cambian TAM_TABLA_HASH en config.h, vuelvan a ejecutar el generador;
los IDs que colisionan se fabrican para ese tamano de tabla.
