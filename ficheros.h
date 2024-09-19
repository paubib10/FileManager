#ifndef FICHEROS
#define FICHEROS

#include "ficheros_basico.h"

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
 * Write nbytes from the buf_original to the file on the given inode with the specified padding.
 *
 * @param ninodo inode number start
 * @param buf_original buffer with the data to write
 * @param offset offset in the file
 * @param nbytes number of bytes to write
 * @return number of bytes written
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 * Read nbytes from the file on the given inode to the buf_original with the specified padding.
 *
 * @param ninodo inode number start
 * @param buf_original buffer to store the data
 * @param offset offset in the file
 * @param nbytes number of bytes to read
 * @return number of bytes read
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);

/**
 * Get the status of the file on the given inode.
 *
 * @param ninodo inode number
 * @param p_stat pointer to the stat_t structure to store the status
 * @return if the status is retrieved correctly
 */
int mi_stat_f(unsigned int ninodo, STAT_t *p_stat);

/**
 * Change the permissions of the file on the given inode.
 *
 * @param ninodo inode number
 * @param permisos new permissions
 * @return if the permissions are changed correctly
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);

/**
 * Truncate the file on the given inode to the specified number of bytes.
 * It will free the needed blocks and update the inode.
 *
 * @param ninodo inode number
 * @param nbytes number of bytes
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);

#endif //FICHEROS