#include "directorios.h"
#include "debug.h"

#define DEBUG7 1

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') {
        fprintf(stderr, "Error: el camino debe comenzar con '/'.\n");
        return FALLO;
    }

    // Copiamos el camino para no modificar el original
    char *camino_copia = strdup(camino);

    // Buscamos el segundo '/'
    char *segundo_slash = strchr(camino_copia + 1, '/');

    if (segundo_slash != NULL) { // Si hay un segundo '/'
        // Copiamos la parte inicial
        strncpy(inicial, camino_copia + 1, segundo_slash - camino_copia - 1);
        inicial[segundo_slash - camino_copia - 1] = '\0'; // Aseguramos que la cadena termine

        // Copiamos la parte final
        strcpy(final, segundo_slash);

        // Asignamos el tipo
        *tipo = 'd';
    } else { // Si no hay un segundo '/'
        // Copiamos todo el camino en inicial
        strcpy(inicial, camino_copia + 1);

        // La parte final es una cadena vacía
        strcpy(final, "");

        // Asignamos el tipo
        *tipo = 'f';
    }

    // Liberamos la memoria de la copia del camino
    free(camino_copia);

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

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, sizeof(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if(!strcmp(camino_parcial, "/")) {
        superbloque_t SB;
        bread(posSB, &SB); // Control de errores

        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    // Extraemos el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUG7
        DEBUG("buscar_entrada", "inicial: %s, final: %s, reservar: %d\n", inicial, final, reservar)
    #endif

    // Leemos el inodo del directorio
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) {
        return ERROR_PERMISO_LECTURA;
    }

    // Inicializamos el buffer de lectura 
    entrada_t buff_lectura[BLOCKSIZE / sizeof(entrada_t)];
    memset(buff_lectura, 0, (BLOCKSIZE / sizeof(entrada_t)) * sizeof(entrada_t));

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada_t);
    num_entrada_inodo = 0;

    // int offset = 0;
    if(cant_entradas_inodo > 0) {
        if((inodo_dir.permisos & 4) != 4) {
            return ERROR_PERMISO_LECTURA;
        }
        if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) < 0) {
            return ERROR_PERMISO_LECTURA;
        }

        while(num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {
            num_entrada_inodo++;
            if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada_t), sizeof(entrada_t)) < 0) 
                return ERROR_PERMISO_LECTURA;
        }
    }

    if(num_entrada_inodo == cant_entradas_inodo && (inicial != buff_lectura[num_entrada_inodo % (BLOCKSIZE / sizeof(entrada_t))].nombre)) {
        switch (reservar)
        {
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
                    if(strcmp(final, "/")) {
                        // Reservar un nuevo inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo(tipo, permisos);

                        #if DEBUG7
                            DEBUG("buscar_entrada", "reservado inodo %d tipo %c con permisos %d para '%s'\n", entrada.ninodo, tipo, permisos, entrada.nombre)
                        #endif
                    } else { // Cuelgan más directorios o ficheros
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else { // Es un fichero
                    // Reservar un nuevo inodo como fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo(tipo, permisos);

                    #if DEBUG7
                        DEBUG("buscar_entrada", "reservado inodo %d tipo %c con permisos %d para '%s'\n", entrada.ninodo, tipo, permisos, entrada.nombre)
                    #endif
                
                }

                #if DEBUG7
                    DEBUG("buscar_entrada", "creada entrada: %s, %d\n", inicial, entrada.ninodo);
                #endif

                // Escribir la entrada en el directorio padre
                if(mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(entrada_t)) == FALLO) {
                    if(entrada.ninodo != -1) {
                        liberar_inodo(entrada.ninodo);
                        #if DEBUG7
                            DEBUG("buscar_entrada", "liberado inodo %i, reservado a %s\n", num_entrada_inodo, inicial)
                        #endif
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
        *(p_inodo) = num_entrada_inodo; // Asignar a *p_inodo el numero del inodo del directorio o fichero creado o leido
        *(p_entrada) = entrada.ninodo; // Asignar a *p_entrada el numero de su entrada dentro del último directorio que lo contiene

        return EXITO; // 0
    } else {

        *(p_inodo_dir) = entrada.ninodo; // Asignar a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada
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