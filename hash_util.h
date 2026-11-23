// hash_util.h - funcion hash djb2 sobre idReserva.
// Se usa uint32_t para que el resultado sea identico en cualquier plataforma
// (con "unsigned long" cambiaria entre Windows y Linux) y asi el generador de
// datos pueda fabricar IDs que colisionen de forma reproducible.
#ifndef HASH_UTIL_H
#define HASH_UTIL_H

#include <string>
#include <stdint.h>

inline int hashId(const std::string& clave, int tam) {
    uint32_t h = 5381;
    for (size_t i = 0; i < clave.size(); i++) {
        h = h * 33u + (unsigned char)clave[i];
    }
    return (int)(h % (uint32_t)tam);
}

#endif
