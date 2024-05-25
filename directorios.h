#ifndef DIRECTORIOS
#define DIRECTORIOS

#include "ficheros.h"

#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256

#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000)

#define PROFUNDIDAD 32 //profunidad máxima del arbol de directorios

#define CACHE 3

typedef struct {
  char nombre[TAMNOMBRE];
  unsigned int ninodo;
} entrada_t;

typedef struct {
    char camino[TAMNOMBRE * PROFUNDIDAD];
    int p_inodo;
} ultimaEntrada_t;
<<<<<<< HEAD
=======


>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51

// Error symbols
#define ERROR_CAMINO_INCORRECTO                         (-1)
#define ERROR_PERMISO_LECTURA                           (-2)
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA                (-3)
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO           (-4)
#define ERROR_PERMISO_ESCRITURA                         (-5)
#define ERROR_ENTRADA_YA_EXISTENTE                      (-6)
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO   (-7)

// Funciones
/**
 * Get the path from the given directory.
 *
 * @param camino path to extract
 * @param inicial start
 * @param final end
 * @param tipo type
 * @return if success or the error
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);

/**
 * Search for an entry in the given path.
 * @param camino_parcial path for the entry
 * @param p_inodo_dir path to the inode directory
 * @param p_inodo path to the inode
 * @param p_entrada path to the entry
 * @param reservar if the entry should be reserved
 * @param permisos the permissions
 * @return if success or the error
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo,
                   unsigned int *p_entrada, char reservar, unsigned char permisos);

/**
 * Show the error message for the given error.
 */
void mostrar_error_buscar_entrada(int error);
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer, char tipo);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, stat_t *p_stat);
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);

#endif // DIRECTORIOS