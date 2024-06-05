// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#ifndef FICHEROS
#define FICHEROS

#include "ficheros_basico.h"

// Definición de la estructura STAT
typedef union STAT {
    struct {
        unsigned char tipo;                // Tipo ('l':libre, 'd':directorio o 'f':fichero)
        unsigned char permisos;            // Permisos (lectura y/o escritura y/o ejecución)
        unsigned reservado_alineacion1[6]; // Alineación
        time_t atime;                      // Fecha y hora del último acceso a datos: atime
        time_t mtime;                      // Fecha y hora de la última modificación de datos: mtime
        time_t ctime;                      // Fecha y hora de la última modificación del inodo: ctime
        unsigned int nlinks;               // Cantidad de enlaces de entradas en directorio
        unsigned int tamEnBytesLog;        // Tamaño en bytes lógicos
        unsigned int numBloquesOcupados;   // Cantidad de bloques ocupados
    };
    char padding[INODOSIZE];
} STAT_t;

/**
 * Escribe nbytes desde buf_original en el fichero del inodo especificado con el desplazamiento indicado.
 *
 * @param ninodo Número del inodo
 * @param buf_original Buffer con los datos a escribir
 * @param offset Desplazamiento en el fichero
 * @param nbytes Número de bytes a escribir
 * @return Número de bytes escritos
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 * Lee nbytes del fichero del inodo especificado en buf_original con el desplazamiento indicado.
 *
 * @param ninodo Número del inodo
 * @param buf_original Buffer para almacenar los datos leídos
 * @param offset Desplazamiento en el fichero
 * @param nbytes Número de bytes a leer
 * @return Número de bytes leídos
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 * Obtiene el estado del fichero en el inodo especificado.
 *
 * @param ninodo Número del inodo
 * @param p_stat Puntero a la estructura STAT_t para almacenar el estado
 * @return Si el estado se obtiene correctamente
 */
int mi_stat_f(unsigned int ninodo, STAT_t *p_stat);

/**
 * Cambia los permisos del fichero en el inodo especificado.
 *
 * @param ninodo Número del inodo
 * @param permisos Nuevos permisos
 * @return Si los permisos se cambian correctamente
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

/**
 * Trunca el fichero en el inodo especificado al número de bytes indicado.
 * Liberará los bloques necesarios y actualizará el inodo.
 *
 * @param ninodo Número del inodo
 * @param nbytes Número de bytes
 * @return Si el truncado se realiza correctamente
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);

#endif // FICHEROS
