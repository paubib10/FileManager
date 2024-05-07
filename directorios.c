#include "directorios.h"
#include "debug.h"

#define DEBUG7 1

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    if (camino[0] != '/') {
        return FALLO;
    }
    // Localizamos el segundo '/'
    char *segundo_slash = strchr((camino + 1), '/');
    strcpy(tipo, "f");

    if(segundo_slash) { // Si se ha encontrado el segundo '/'
        // Inicial = camino - segundo_slah
        strncpy(inicial, (camino + 1), (strlen(camino)- strlen(segundo_slash)-1));
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
