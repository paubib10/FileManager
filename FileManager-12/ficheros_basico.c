#include <limits.h>
#include "ficheros_basico.h"

int tamMB(unsigned int nbloques) {
    return (nbloques / 8) % BLOCKSIZE != 0 ?
    ((int)nbloques / 8) / BLOCKSIZE + 1 :
    ((int)nbloques / 8) / BLOCKSIZE;
}

int tamAI(unsigned int ninodos) {
    return (ninodos * INODOSIZE) % BLOCKSIZE != 0 ?
    ((int)ninodos * INODOSIZE) / BLOCKSIZE + 1 :
    ((int)ninodos * INODOSIZE) / BLOCKSIZE;
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    superbloque_t sb = {{
        posSB + tamSB,
        sb.posPrimerBloqueMB + tamMB(nbloques) - 1,
        sb.posUltimoBloqueMB + 1,
        sb.posPrimerBloqueAI + tamAI(ninodos) - 1,
        sb.posUltimoBloqueAI + 1,
        nbloques - 1,
        0,
        0,
        nbloques,
        ninodos,
        nbloques,
        ninodos,
    }};

    if (bwrite(posSB, &sb) == FALLO) return FALLO;

    return EXITO;
}

// TODO: Se podria usar un memset de 1Kb para la pagina y luego comprobar si se necesitan mas bloques
int initMB() {
    // Leemos el superbloque
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    unsigned int metadatos = tamSB + tamMB(sb.totBloques) + tamAI(sb.totInodos);
    sb.cantBloquesLibres -= metadatos;

    unsigned int  bloques = metadatos / 8 / BLOCKSIZE;

    int posMB = 0;
    do {
        unsigned char bufferMB[BLOCKSIZE];
        if (memset(bufferMB, 0, BLOCKSIZE) == NULL) return FALLO;

        unsigned int bytes_1 = metadatos / 8;
        unsigned int bits_1  = metadatos % 8;

        int max = bytes_1 < BLOCKSIZE ? (int) bytes_1 : BLOCKSIZE;

        // Escribir el bloque en el buffer
        for (int i = 0; i < max ; ++i) bufferMB[i] = 255;

        if (max != BLOCKSIZE)
            for (int i = 0; i < bits_1; ++i) bufferMB[bytes_1] |= 1 << (7 - i);

        if (bwrite(sb.posPrimerBloqueMB + posMB, &bufferMB) == FALLO) return FALLO;

        posMB++;
        metadatos -= BLOCKSIZE * 8;
    } while (posMB <= bloques);

    // Actualizar el superbloque
    if (bwrite(posSB, &sb) == FALLO) return FALLO;

    return EXITO;
}

int initAI() {
    // Inicializamos la estructura de inodos
    inodo_t inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el superbloque
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    unsigned contInodos = sb.posPrimerInodoLibre + 1;

    // Recorremos los bloques de inodos
    for (unsigned i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        inodo_t inodo;

        if (bwrite(i, &inodo) == FALLO) return FALLO;

        // Recorremos los inodos de cada bloque
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            inodos[j].tipo = 'l';

            // Si quedan inodos por inicializar
            if (contInodos < sb.totInodos) {
                // Enlazamos el inodo con el siguiente
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        // Escribimos el bloque de inodos en el dispositivo virtual
        if (bwrite(i, &inodos) == FALLO) return FALLO;
    }

    return 1;
}

int escribir_bit(unsigned int nBloque, unsigned int bit) {
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    // Calculos para saber en que bloque del mapa de bits se encuentra el bloque nBloque
    unsigned int posbyte = nBloque / 8;
    unsigned int posbit = nBloque % 8;
    unsigned int nbloque = posbyte / BLOCKSIZE;
    unsigned int nBloqueAbs = sb.posPrimerBloqueMB + nbloque;

    //
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nBloqueAbs, &bufferMB) == FALLO) return FALLO;

    // Cambiamos el bit del bloque
    unsigned char mascara = 0b10000000;
    mascara >>= posbit;
    if (bit == 0) bufferMB[posbyte] &= ~mascara;
    else bufferMB[posbyte] |= mascara;

    if (bwrite(nBloqueAbs, &bufferMB) == FALLO) return FALLO;

    return EXITO;
}

char leer_bit(unsigned int nBloque) {
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    // Calculos para saber en que bloque del mapa de bits se encuentra el bloque nBloque
    unsigned int posbyte = nBloque / 8;
    unsigned int posbit = nBloque % 8;
    unsigned int nbloque = posbyte / BLOCKSIZE;
    unsigned int nBloqueAbs = sb.posPrimerBloqueMB + nbloque;

    //
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nBloqueAbs, &bufferMB) == FALLO) return FALLO;

    posbyte %= BLOCKSIZE;

    // Cambiamos el bit del bloque
    unsigned char mascara = 0b10000000;
    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7 - posbit);

    #if DEBUG3
    printf("[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n\n", nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
    #endif

    return mascara;
}

int reservar_bloque() {
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    // Comprobar si hay bloques libres
    if (sb.cantBloquesLibres == 0) return FALLO;

    unsigned char bufferMB[BLOCKSIZE];

    unsigned char bufferAux[BLOCKSIZE];
    if (memset(bufferAux, 255, BLOCKSIZE) == NULL) return FALLO;

    // Buscar el primer bloque libre
    int nBloqueMB;
    for (nBloqueMB = 0; nBloqueMB < sb.posUltimoBloqueMB - sb.posPrimerBloqueMB; nBloqueMB++) {
        // Leer el bloque de mapa de bits
        if (bread(sb.posPrimerBloqueMB + nBloqueMB, &bufferMB) == FALLO) return FALLO;

        // Si el bloque no esta lleno (hay bloques libres) parar la busqueda
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) break;
    }

    // Buscar el primer byte que no esta lleno
    int posByte = 0;
    for (int i = 0; i < BLOCKSIZE; ++i) {
        if (bufferMB[i] == 255) continue;

        posByte = i;
        break;
    }

    // Obtener el primer bit libre
    unsigned int mascara = 0b10000000; // 128
    int posBit = 0;
    while (bufferMB[posByte] & mascara) {
        bufferMB[posByte] <<= 1;
        posBit++;
    }

    int nBloque = (nBloqueMB * BLOCKSIZE + posByte) * 8 + posBit;

    // Escribimos el bit para reservarlo
    escribir_bit(nBloque, 1);

    // Actualizar el bloque SB
    sb.cantBloquesLibres--;
    if (bwrite(posSB, &sb) == FALLO) return FALLO;

    // Limpiar el bloque reservado
    if (memset(bufferAux, 0, BLOCKSIZE) == NULL) return FALLO;
    if (bwrite(sb.posPrimerBloqueDatos + nBloque, &bufferAux) == FALLO) return FALLO;

    return nBloque;
}

int liberar_bloque(unsigned int nBloque) {
    if (escribir_bit(nBloque, 0) == FALLO) return FALLO;

    // Leer el superbloque
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    // Incrementar la cantidad de bloques libres en el superbloque
    sb.cantBloquesLibres++;

    // Escribir el superbloque
    if (bwrite(posSB, &sb) == FALLO) return FALLO;

    return nBloque;
}

int escribir_inodo(unsigned int ninodo, inodo_t* inodo) {
    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) return FALLO;

    // Calcular el número de bloque del array de inodos que contiene el inodo solicitado
    unsigned int nBloque = sb.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    // Leer el bloque del array de inodos
    inodo_t inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nBloque, inodos) == FALLO) return FALLO;

    // Escribir el contenido del inodo en el lugar correspondiente del array
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;

    // Escribir el bloque modificado en el dispositivo virtual
    if (bwrite(nBloque, inodos) == FALLO) return FALLO;

    return EXITO;
}

int leer_inodo(unsigned int ninodo, inodo_t *inodo) {
    superbloque_t sb;
    if(bread(posSB, &sb) == FALLO) return FALLO;

    // Calcular el número de bloque del array de inodos que contiene el inodo solicitado
    unsigned int nBloque = sb.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    // Leer el bloque del array de inodos
    inodo_t inodos[BLOCKSIZE / INODOSIZE];
    if(bread(nBloque, inodos) == FALLO) return FALLO;

    // Leer el contenido del inodo en el lugar correspondiente del array
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    superbloque_t sb;
    if (bread(0,&sb) == FALLO) {
        ERR("reservar_inodo", "Error al leer el superbloque\n")
        return FALLO;
    }

    // Si no hay inodos libres, error
    if (sb.cantInodosLibres == 0) {
        ERR("reservar_inodo", "No hay inodos libres\n")
        return FALLO;
    }

    unsigned int inodoReservado = sb.posPrimerInodoLibre;
    sb.posPrimerInodoLibre += 1;

    inodo_t inodo = {{
        tipo,
        permisos,
        {0, 0, 0, 0, 0, 0},
        time(NULL),
        time(NULL),
        time(NULL),
        1,
        0,
        0
    }};

    if (escribir_inodo(inodoReservado, &inodo) == FALLO) {
        ERR("reservar_inodo", "Error al escribir el inodo\n")
        return FALLO;
    }

    // Actualización superbloque
    sb.cantInodosLibres -= 1;
    if (bwrite(posSB, &sb) == -1) {
        ERR("reservar_inodo", "Error al escribir el superbloque\n")
        return FALLO;
    }

    return (int) inodoReservado;
}

int obtener_nrangoBL(inodo_t inodo, unsigned int nblogico, unsigned int* ptr) {
    if(nblogico < DIRECTOS) { // < 12
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    } else if(nblogico < INDIRECTOS0) { // < 268
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    } else if(nblogico < INDIRECTOS1) { // < 65804 
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    } else if(nblogico < INDIRECTOS2) { // < 16843020
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        ERR("obtener_nrangoBL", "Bloque lógico fuera de rango\n")
        return FALLO;
    }
}

// TODO: Can remove else if and use if
int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if(nblogico < DIRECTOS) return (int) nblogico;
    else if (nblogico < INDIRECTOS0) return (int) nblogico - DIRECTOS;
    else if (nblogico < INDIRECTOS1) {
        int indirect1 = (int) (nblogico - INDIRECTOS0);

        if(nivel_punteros == 2) return indirect1 / (int) NPUNTEROS;
        else if(nivel_punteros == 1) return indirect1 % (int) NPUNTEROS;

    } else if (nblogico < INDIRECTOS2) {
        int indirect2 = (int) (nblogico - INDIRECTOS1);
        int pointers = (NPUNTEROS * NPUNTEROS);

        if(nivel_punteros == 3) return (int) (indirect2 / pointers);
        else if(nivel_punteros == 2) return (int) ((indirect2 % pointers) / NPUNTEROS);
        else if(nivel_punteros == 1) return (indirect2 % pointers) % (int) NPUNTEROS;
    }

    return FALLO;
}

int traducir_bloque_inodo(inodo_t* inodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr = 0, ptr_ant = 0;
    int indice;
    unsigned int buffer[NPUNTEROS];

    // Obtener el tipo de puntero más alto
    int nRangoBL = obtener_nrangoBL(*inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    int nivel_punteros = nRangoBL;

    while(nivel_punteros > 0) {
        // Si el tipo es lectura y el puntero es nulo, error
        if(ptr == 0 && reservar == 0) return FALLO;

        // Si el tipo es escritura y el puntero es nulo, reservar bloque
        if(ptr == 0 && reservar != 0) {
            ptr = reservar_bloque();
            inodo -> numBloquesOcupados++;
            inodo -> ctime = time(NULL);

            // printf("Reservado BF %d %d\n", ptr, nivel_punteros);

            // Si el bloque está en otra rama, guardamos el bloque
            if(nivel_punteros != nRangoBL) {
                buffer[indice] = ptr;
                bwrite(ptr_ant, buffer);
                #if DEBUG4
                    DEBUG("traducir_bloque_inodo",
                        "inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)\n",
                        nivel_punteros, indice, ptr, ptr, nivel_punteros)
                #endif
            } else {
                inodo->punterosIndirectos[nRangoBL - 1] = ptr;
                #if DEBUG4
                    DEBUG("traducir_bloque_inodo",
                        "inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)\n",
                        nRangoBL - 1, ptr, ptr, nivel_punteros)
                #endif
            }

            memset(buffer, 0, BLOCKSIZE);
        } else bread(ptr, buffer);

        // Guardamos el puntero y nos movemos al nivel inferior
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    if(ptr == 0 && reservar == 0) return FALLO;

    if(ptr == 0 && reservar != 0) {
        ptr = reservar_bloque();
        inodo -> numBloquesOcupados++;
        inodo -> ctime = time(NULL);

        // Actualizamos la direccion del bloque de datos
        if(nRangoBL != 0) {
            buffer[indice] = ptr;
            bwrite(ptr_ant, buffer);
            #if DEBUG4
                DEBUG("traducir_bloque_inodo",
                    "inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)\n",
                    indice, ptr, ptr, nblogico)
            #endif
        } else {
            inodo->punterosDirectos[nblogico] = ptr;
            #if DEBUG4
                DEBUG("traducir_bloque_inodo",
                    "inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)",
                    nblogico, ptr, ptr, nblogico)
            #endif
        }
    }

    // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
    return (int) ptr;
}

int liberar_inodo(unsigned int ninodo) {
    inodo_t inodo;
    leer_inodo(ninodo, &inodo);

    // Free the block of the inode
    int inodosLiberados = liberar_bloques_inodo(0, &inodo);

    // Update inode data
    inodo.numBloquesOcupados -= inodosLiberados;
    inodo.tipo= 'l';
    inodo.tamEnBytesLog = 0;

    // Should this return err?
    if (inodo.numBloquesOcupados != 0) ERR("liberar_inodo", "Number of blocks occupied is not 0\n")

    superbloque_t SB;
    if(bread(posSB, &SB) == FALLO) {
        ERR("liberar_inodo", "Error al leer el superbloque\n")
        return FALLO;
    }

    // Move the inode to the head of free inodes list
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres += 1;

    // Update metadata of the inode
    inodo.ctime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == FALLO) {
        ERR("liberar_inodo", "Error al escribir el inodo\n")
        return FALLO;
    }

    if(bwrite(posSB, &SB) == FALLO) {
        ERR("liberar_inodo", "Error al escribir el superbloque\n")
        return FALLO;
    }

    return (int) ninodo;
}

int liberar_bloques_inodo(unsigned int primerBL, inodo_t *inodo) {
    unsigned int nRangoBL, nivel_punteros, indice, ultimoBL;

    int breads = 0, bwrites = 0;

    unsigned int bloque_punteros[3][NPUNTEROS]; //1024 bytes
    int ptr_nivel[3];
    int indices[3];

    unsigned char buf_punteros[BLOCKSIZE];
    memset(buf_punteros, 0, BLOCKSIZE);

    int liberados = 0;

    // Saltar exploración bloques innecesarios: El bucle comienza desde el primerBL, no desde el principio.
    if (inodo->tamEnBytesLog == 0) {
        return 0;
    }

    if ((inodo->tamEnBytesLog % BLOCKSIZE) == 0)
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    else
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;

    #if DEBUG6
        DEBUG("liberar_bloques_inodo", "primerBL: %d, ultimoBL: %d\n", primerBL, ultimoBL)
    #endif
    unsigned ptr = 0;
    for (unsigned nBL = primerBL; nBL <= ultimoBL; nBL++) {
        nRangoBL = obtener_nrangoBL(*inodo, nBL, &ptr);
        if (nRangoBL == FALLO) return FALLO;

        nivel_punteros = nRangoBL;

        while (ptr > 0 && nivel_punteros > 0) {
            indice = obtener_indice(nBL, (int) nivel_punteros);

            // Minimizar L/E dispositivo(bread, bwrite): Solo se lee el bloque si es necesario.
            if(indice == 0 || nBL == primerBL) {
                bread(ptr, bloque_punteros[nivel_punteros - 1]);
                breads++;
            }

            ptr_nivel[nivel_punteros - 1] = (int) ptr;
            indices[nivel_punteros - 1] = (int) indice;
            ptr = bloque_punteros[nivel_punteros - 1][indice];

            /*
            #if DEBUG6
                DEBUG("liberar_bloques_inodo", "Estamos en el bloque %d y saltamos al bloque %d\n", ptr_nivel, ptr)
            #endif
            */

            nivel_punteros--;
        }

        if (ptr > 0) {
            liberar_bloque(ptr);
            #if DEBUG6
                DEBUG("liberar_bloques_inodo", "liberado BF %d de datos para BL %d\n", ptr, nBL)
            #endif
            liberados++;

            // Recursividad completa: La función se llama a sí misma de forma recursiva.
            if (nRangoBL == 0) {
                inodo->punterosDirectos[nBL] = 0;
            } else {
                nivel_punteros = 1;

                while (nivel_punteros <= nRangoBL) {
                    indice = indices[nivel_punteros - 1];
                    bloque_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    
                    // Compactación algoritmo inodo->hojas (anexo), aunando tratamiento indirectos:
                    // Los bloques de punteros se liberan si ya no son necesarios
                    if (memcmp(bloque_punteros[nivel_punteros - 1], buf_punteros, BLOCKSIZE) == 0) {
                        liberar_bloque(ptr);
                        #if DEBUG6
                        DEBUG("liberar_bloques_inodo", "liberado BF %i de punteros_nivel%i correspondiente al BL %i\n",
                                  ptr, nivel_punteros, nBL)
                        #endif
                        liberados++;

                        if(nivel_punteros == nRangoBL) inodo->punterosIndirectos[nRangoBL - 1] = 0;

                        nivel_punteros++;
                    } else {
                        bwrite(ptr, bloque_punteros[nivel_punteros - 1]);
                        bwrites++;
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }

    #if DEBUG6
        DEBUG("liberar_bloques_inodo", "total bloques liberados: %d, total_breads: %d, total_bwrites:%d\n",
            liberados, breads, bwrites)
    #endif

    return liberados;
}