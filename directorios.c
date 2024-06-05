//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "directorios.h"

// Variables globales
ultimaEntrada_t ultimaEntrada[CACHE];
int MAXCACHE = CACHE;
int cacheIndex=0;

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    if (camino[0] != '/') {
        return FALLO;
    }
    // Localizamos el segundo '/'
    char *segundo_slash = strchr((camino + 1), '/');
    strcpy(tipo, "f");

    if(segundo_slash) { // Si se ha encontrado el segundo '/'
        // Inicial = camino - segundo_slah
        int len = segundo_slash - (camino + 1);
        strncpy(inicial, (camino + 1), len);
        inicial[len] = '\0';
        // Final = segundo_slash
        strcpy(final, segundo_slash);

        // Comprobamos si es un directorio
        if(final[0] == '/') strcpy(tipo, "d");

    } else { // Si no se ha encontrado
        // Inicial = camino
        strcpy(inicial, (camino + 1));
        // Final = vacío
        strcpy(final, "");
    }
    return EXITO;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    superbloque_t SB;
    entrada_t entrada;
    inodo_t inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));

    if (strcmp(camino_parcial, "/") == 0) {
        if (bread(posSB, &SB) == FALLO) {
            return FALLO;
        }
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUGN7
        fprintf(stderr, GRAY "[buscar_entrada()->inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) {
        return FALLO;
    }
    if ((inodo_dir.permisos & 4) != 4) {
        #if DEBUGN7
            fprintf(stderr, "[buscar_entrada()->El inodo %d no tiene permisos de lectura]\n", *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada_t);
    num_entrada_inodo = 0;

    while (num_entrada_inodo < cant_entradas_inodo) {
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) == FALLO) {
            return FALLO;
        }
        if (strcmp(inicial, entrada.nombre) == 0) {
            break;
        }
        num_entrada_inodo++;
        memset(entrada.nombre, 0, sizeof(entrada.nombre));
    }

    if (num_entrada_inodo == cant_entradas_inodo) {
        if (reservar == 0) {
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        }
        if (inodo_dir.tipo == 'f') {
            return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
        }
        if ((inodo_dir.permisos & 2) != 2) {
            return ERROR_PERMISO_ESCRITURA;
        }

        strcpy(entrada.nombre, inicial);
        if (tipo == 'd' && strcmp(final, "/") == 0) {
            entrada.ninodo = reservar_inodo('d', permisos);
        } else if (tipo == 'f') {
            entrada.ninodo = reservar_inodo('f', permisos);
        } else {
            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
        }

        #if DEBUGN7
            fprintf(stderr, GRAY "[buscar_entrada()->reservado inodo: %d tipo %c con permisos %d para '%s']\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
        #endif

        if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) == FALLO) {
            if (entrada.ninodo != FALLO) {
                liberar_inodo(entrada.ninodo);
            }
            return FALLO;
        }
    }

    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) {
        if (reservar == 1 && num_entrada_inodo < cant_entradas_inodo) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}


void mostrar_error_buscar_entrada(int error) {
    char* msg;

    switch (error) {
        case -1: msg = "Camino incorrecto"; break;
        case -2: msg = "Permiso denegado de lectura"; break;
        case -3: msg = "No existe el archivo o el directorio"; break;
        case -4: msg = "No existe algún directorio intermedio"; break;
        case -5: msg = "Permiso denegado de escritura"; break;
        case -6: msg = "El archivo ya existe"; break;
        case -7: msg = "No es un directorio"; break;
        default: msg = "Error desconocido"; break;
    }

    fprintf(stderr, RED"Error: %s.\n"RESET, msg);
}

int mi_creat(const char *camino, unsigned  char permisos) {
    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0) {
        //mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return error;
    }
    mi_signalSem();
    return EXITO;
}

int mi_dir(const char *camino, char *buffer, char tipo) {
    struct tm *tm;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error, nEntradas = 0;
    char tmp[100], tamEnBytes[10];
    entrada_t entrada;
    inodo_t inodo;

    // Buscar entrada
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // Leer inodo
    if (leer_inodo(p_inodo, &inodo) < 0) return FALLO;

    // Comprobar permisos
    if ((inodo.permisos & 4) != 4) return FALLO;

    // Procesar directorio
    if (tipo == 'd') {
        entrada_t entradas[BLOCKSIZE / sizeof(entrada_t)];
        memset(&entradas, 0, sizeof(entrada_t));
        nEntradas = inodo.tamEnBytesLog / sizeof(entrada_t);
        int offset = 0;
        offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

        // Leer todas las entradas
        for (int i = 0; i < nEntradas; i++) {
            if (leer_inodo(entradas[i % (BLOCKSIZE / sizeof(entrada_t))].ninodo, &inodo) < 0) {
                return FALLO;
            }

            // Agregar entrada a buffer
            // Tipo
            strcat(buffer, (inodo.tipo == 'd') ? "d" : "f");
            strcat(buffer, "\t");

            // Permisos
            if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
            if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
            if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
            strcat(buffer, "\t");

            // mTime
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                    tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            // Tamaño
            sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
            strcat(buffer, tamEnBytes);
            strcat(buffer, "\t");

            // Nombre
            strcat(buffer, CYAN);
            strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(entrada_t))].nombre);
            while ((strlen(buffer) % TAMFILA) != 0) {
                strcat(buffer, " ");
            }
            strcat(buffer, RESET);

            // Siguiente
            strcat(buffer, "\n");

            if (offset % (BLOCKSIZE / sizeof(entrada_t)) == 0) {
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }
    } else {
        // Procesar archivo
        mi_read_f(p_inodo_dir, &entrada, sizeof(entrada_t) * p_entrada, sizeof(entrada_t));
        if (leer_inodo(entrada.ninodo, &inodo) < 0) return FALLO;

        // Agregar entrada a buffer
        // Tipo
        strcat(buffer, (inodo.tipo == 'd') ? "d" : "f");
        strcat(buffer, "\t");

        // Permisos
        if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
        strcat(buffer, "\t");

        // mTime
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour,
                tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // Tamaño
        sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytes);
        strcat(buffer, "\t");

        // Nombre
        strcat(buffer, CYAN);
        strcat(buffer, entrada.nombre);
        while ((strlen(buffer) % TAMFILA) != 0) {
            strcat(buffer, " ");
        }
        strcat(buffer, RESET);

        // Siguiente
        strcat(buffer, "\n");

        nEntradas++;
    }
    return nEntradas;
}

int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);

    if(error < 0) {
        return error;
    }

    mi_chmod_f(p_inodo, permisos);

    return EXITO;
}

int mi_stat(const char *camino, STAT_t *p_stat) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if(error < 0) {
        return error;
    }

    if(mi_stat_f(p_inodo, p_stat) < 0) {
        return FALLO;
    }

    return p_inodo;
}

// Función para escribir en un archivo
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int encontrado = 0;
    cacheIndex = cacheIndex % 3;

    // Buscar en la caché
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (strcmp(ultimaEntrada[i].camino, camino) == 0) {
            #if DEBUGN9
                fprintf(stderr, BLUE "[mi_write() -> Usamos caché de escritura en lugar de buscar_entrada()]\n" RESET);
            #endif
            p_inodo = ultimaEntrada[i].p_inodo;
            encontrado = 1;
            break;
        }
    }

    // Si no se encuentra en la caché
    if (!encontrado) {
        int resultado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (resultado < 0) {
            mostrar_error_buscar_entrada(resultado);
            return resultado;
        }
        strcpy(ultimaEntrada[cacheIndex].camino, camino);
        ultimaEntrada[cacheIndex].p_inodo = p_inodo;
        #if DEBUGN9
            fprintf(stderr, ORANGE "[mi_write() -> Actualizamos caché de escritura]\n" RESET);
        #endif
        cacheIndex++;
    }
    
    int bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
    if (bytes_escritos < 0) {
        return FALLO;
    }

    return bytes_escritos;
}

// Función para leer un archivo
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int encontrado = 0;
    int bytes_leidos = 0;

    // Verificar la caché
    for (int i = 0; i < MAXCACHE - 1; i++) {
        if (strcmp(camino, ultimaEntrada[i].camino) == 0) {
            p_inodo = ultimaEntrada[i].p_inodo;
            encontrado = 1;

            #if DEBUG9
                fprintf(stderr, CYAN "\nmi_read() -> Usamos caché de lectura en lugar de buscar_entrada()\n" RESET);
            #endif
            break;
        }
    }

    // Si no está en la caché
    if (!encontrado) {
        int resultado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (resultado < 0) {
            return resultado;
        }

        // Actualizar caché
        if (MAXCACHE > 0) {
            strcpy(ultimaEntrada[CACHE - MAXCACHE].camino, camino);
            ultimaEntrada[CACHE - MAXCACHE].p_inodo = p_inodo;
            MAXCACHE--;

            #if DEBUG9
                fprintf(stderr, ORANGE "mi_read() -> Actualizamos caché de lectura\n" RESET);
            #endif
        } else {  // Reemplazo FIFO
            for (int i = 0; i < CACHE - 1; i++) {
                strcpy(ultimaEntrada[i].camino, ultimaEntrada[i + 1].camino);
                ultimaEntrada[i].p_inodo = ultimaEntrada[i + 1].p_inodo;
            }
            strcpy(ultimaEntrada[CACHE - 1].camino, camino);
            ultimaEntrada[CACHE - 1].p_inodo = p_inodo;

            #if DEBUG9
                fprintf(stderr, ORANGE "mi_read() -> Actualizamos caché de lectura\n" RESET);
            #endif
        }
    }

    bytes_leidos = mi_read_f(p_inodo, buf, offset, nbytes);
    if (bytes_leidos < 0) {
        return ERROR_PERMISO_LECTURA;
    }

    return bytes_leidos;
}


int mi_link(const char *camino1, const char *camino2) {
    mi_waitSem();

    // Variables para almacenar los resultados de buscar_entrada
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0;
    unsigned int p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;
    inodo_t inodo;
    int error;

    // Buscar la entrada para el camino 1
    error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    if(leer_inodo(p_inodo1, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if(inodo.tipo != 'f') {
        mi_signalSem();
        return ERROR_CAMINO_INCORRECTO;
    }

    if((inodo.permisos & 4) != 4) {
        mi_signalSem();
        return ERROR_PERMISO_LECTURA;
    }

    // Buscar la entrada para el camino 2
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    // Leer la entrada del directorio para el camino 2
    entrada_t entrada2;
    if(mi_read_f(p_inodo_dir2, &entrada2, sizeof(entrada_t) * (p_entrada2), sizeof(entrada_t)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    // Crear el enlace y escribir la entrada
    entrada2.ninodo = p_inodo1;

    // Incrementar el número de enlaces
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    if(escribir_inodo(p_inodo1, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if(mi_write_f(p_inodo_dir2, &entrada2, sizeof(entrada_t) * (p_entrada2), sizeof(entrada_t)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    // Liberar el inodo y actualizar la metainformación
    if(liberar_inodo(p_inodo2) < 0 || escribir_inodo(p_inodo1, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    return EXITO;
}

int mi_unlink(const char *camino) {
    mi_waitSem();

    // Variables para almacenar los resultados de buscar_entrada y leer_inodo
    superbloque_t SB;
    bread(posSB, &SB);
    unsigned int p_inodo_dir = SB.posInodoRaiz, p_inodo = SB.posInodoRaiz;
    unsigned int p_entrada = 0;
    inodo_t inodo, inodo_dir;
    int error;

    // Buscar la entrada para el camino
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    // Leer el inodo y comprobar si es un directorio no vacío
    if(leer_inodo(p_inodo, &inodo) < 0 || (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)) {
        fprintf(stderr, RED"Error: El directorio %s no está vacío\n"RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    // Leer el inodo del directorio
    if(leer_inodo(p_inodo_dir, &inodo_dir) < 0) {
        mi_signalSem();
        return FALLO;
    }

    // Si la entrada no es la última, mover la última entrada a su lugar
    int num_entrada = inodo_dir.tamEnBytesLog / sizeof(entrada_t);
    if(p_entrada != num_entrada - 1) {
        entrada_t entrada;
        if(mi_read_f(p_inodo_dir, &entrada, sizeof(entrada_t) * (num_entrada - 1), sizeof(entrada_t)) < 0) {
            mi_signalSem();
            return FALLO;
        }
        if(mi_write_f(p_inodo_dir, &entrada, sizeof(entrada_t) * (p_entrada), sizeof(entrada_t)) < 0) {
            mi_signalSem();
            return FALLO;
        }
    }

    // Truncar el fichero del directorio y disminuir el número de enlaces
    if(mi_truncar_f(p_inodo_dir, sizeof(entrada_t) * (num_entrada - 1)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    inodo.nlinks--;

    // Si no quedan enlaces, liberar el inodo, de lo contrario, actualizar la metainformación
    if(!inodo.nlinks) {
        if (liberar_inodo(p_inodo) < 0) {
            mi_signalSem();
            return FALLO;
        }
    } else {
        inodo.ctime = time(NULL);
        if(escribir_inodo(p_inodo, &inodo) < 0) {
            mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();
    return EXITO;
}