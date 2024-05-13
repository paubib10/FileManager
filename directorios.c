#include "directorios.h"
#include "debug.h"

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

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, 
                   unsigned int *p_entrada, char reservar, unsigned char permisos) {
    entrada_t entrada;
    inodo_t inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if(!strcmp(camino_parcial, "/")) {
        superbloque_t SB;
        bread(posSB, &SB); // Control de errores

        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;

        return EXITO;
    }

    // Extraemos el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUG7
        DEBUG("buscar_entrada", "inicial: %s, final: %s, reservar: %d\n", inicial, final, reservar)
    #endif

    // Buscamosla entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if((inodo_dir.permisos & 4) != 4) {
        #if DEBUG7
            DEBUG("buscar_entrada", "El inodo %d no tiene permisos de lectura\n", *p_inodo_dir)
        #endif
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    entrada_t  buff_lectura [BLOCKSIZE / sizeof(entrada_t)];
    memset(buff_lectura, 0, BLOCKSIZE);

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada_t);
    num_entrada_inodo = 0;

    if(cant_entradas_inodo > 0) {
        
        if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) < 0) {
            return ERROR_PERMISO_LECTURA;
        }

        memset(buff_lectura, 0, (BLOCKSIZE / sizeof(entrada_t)) * sizeof(entrada_t));

        while(num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {

            num_entrada_inodo++;
            memset(entrada.nombre, 0, sizeof(entrada.nombre));

            if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) < 0) {
                return ERROR_PERMISO_LECTURA;
            }
        }
    }

    if((strcmp(entrada.nombre, inicial) != 0) && (num_entrada_inodo == cant_entradas_inodo)) {

        switch (reservar) {
        case 0: // modo consulta. Como no existe la entrada retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            break;
        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por el *p_inodo_dir
            // Si es fichero no permitir escritura
            if(inodo_dir.tipo == 'f') {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            // Si es directorio comprobar que tiene permiso de escritura
            if((inodo_dir.permisos & 2) != 2) {
                return ERROR_PERMISO_ESCRITURA;
            } else {
                strcpy(entrada.nombre, inicial);

                if(tipo == 'd') {
                    if(strcmp(final, "/") == 0) {
                        // Reservar un nuevo inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo('d', permisos);

                        #if DEBUG7
                            DEBUG("buscar_entrada", "reservado inodo %d tipo %c con permisos %d para '%s'\n", entrada.ninodo, tipo, permisos, entrada.nombre)
                        #endif

                    } else { // Cuelgan más directorios o ficheros
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else { // Es un fichero
                    // Reservar un nuevo inodo como fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo('f', permisos);

                    #if DEBUG7
                        DEBUG("buscar_entrada", "reservado inodo %d tipo %c con permisos %d para '%s'\n", entrada.ninodo, tipo, permisos, entrada.nombre)
                    #endif
                
                }

                #if DEBUG7
                    DEBUG("buscar_entrada", "creada entrada: %s, %d\n", inicial, entrada.ninodo);
                #endif

                // Escribir la entrada en el directorio padre
                if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) < 0) {

                    if(entrada.ninodo != -1) {
                        liberar_inodo(entrada.ninodo);
                        /*#if DEBUG7
                            DEBUG("buscar_entrada", "liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial)
                        #endif*/
                    }
                    return FALLO;
                }
            }   
        }
    }

    // Si hemos llegado al final del camino
    if(!strcmp(final, "/") || !strcmp(final, "")) {
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
            // Modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        // Cortamos la recursividad
        *p_inodo = entrada.ninodo; // Asignar a *p_inodo el numero del inodo del directorio o fichero creado o leido
        *p_entrada = num_entrada_inodo; // Asignar a *p_entrada el numero de su entrada dentro del último directorio que lo contiene

        return EXITO; // 0

    } else {
        *p_inodo_dir = entrada.ninodo; // Asignar a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return EXITO;
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

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0) {
        return error;
    }

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

int mi_stat(const char *camino, stat_t *p_stat) {

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