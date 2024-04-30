#ifndef DIRECTORIOS
#define DIRECTORIOS

#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256

#include "ficheros.h"

typedef struct {
  char nombre[TAMNOMBRE];
  unsigned int ninodo;
} entrada_t;

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

#endif // DIRECTORIOS