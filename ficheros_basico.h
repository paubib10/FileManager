//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#ifndef FICHEROS_BASICO
#define FICHEROS_BASICO

#include <time.h>
#include <limits.h>
#include "bloques.h"

#define INODOSIZE    128

#define posSB        0
#define tamSB        1

#define NPUNTEROS    (BLOCKSIZE / sizeof(unsigned int))   // 256 punteros por bloque
#define DIRECTOS     12
#define INDIRECTOS0  (NPUNTEROS + DIRECTOS)    // 268
#define INDIRECTOS1  (NPUNTEROS * NPUNTEROS + INDIRECTOS0)    // 65.804
#define INDIRECTOS2  (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 16.843.020

typedef union inodo {
    struct {
        unsigned char tipo;                     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
        unsigned char permisos;                 // Permisos (lectura y/o escritura y/o ejecución)
        unsigned char reservado_alineacion1[6]; // Reservado
        time_t atime;                           // Last data access time
        time_t mtime;                           // Last modify time
        time_t ctime;                           // Last inode change time
        unsigned int nlinks;                    // Cantidad de enlaces de entradas en directorio
        unsigned int tamEnBytesLog;             // Tamaño en bytes lógicos (EOF)
        unsigned int numBloquesOcupados;        // Cantidad de bloques ocupados zona de datos
        unsigned int punterosDirectos[12];      // 12 punteros a bloques directos
        unsigned int punterosIndirectos[3];     // 1 simple, 1 doble, 1 triple
    };
    char padding[INODOSIZE];
} inodo_t;

typedef union superbloque {
    struct {
        unsigned int posPrimerBloqueMB;          // Posición absoluta del primer bloque del mapa de bits
        unsigned int posUltimoBloqueMB;          // Posición absoluta del último bloque del mapa de bits
        unsigned int posPrimerBloqueAI;          // Posición absoluta del primer bloque del array de inodos
        unsigned int posUltimoBloqueAI;          // Posición absoluta del último bloque del array de inodos
        unsigned int posPrimerBloqueDatos;       // Posición absoluta del primer bloque de datos
        unsigned int posUltimoBloqueDatos;       // Posición absoluta del último bloque de datos
        unsigned int posInodoRaiz;               // Posición del inodo del directorio raíz (relativa al AI)
        unsigned int posPrimerInodoLibre;        // Posición del primer inodo libre (relativa al AI)
        unsigned int cantBloquesLibres;          // Cantidad de bloques libres (en todo el disco)
        unsigned int cantInodosLibres;           // Cantidad de inodos libres (en el AI)
        unsigned int totBloques;                 // Cantidad total de bloques del disco
        unsigned int totInodos;                  // Cantidad total de inodos (heurística)
    };
    char padding[BLOCKSIZE];
} superbloque_t;

/**
 * Calculate the size of the bitmap.
 *
 * @param nbloques number of blocks
 * @return the size of the bitmap
 */
int tamMB(unsigned int nbloques);

/**
 * Calculate the size of the inode table.
 *
 * @param ninodos number of inodes
 * @return the size of the inode table
 */
int tamAI(unsigned int ninodos);

/**
 * Calculate the size of the data blocks.
 *
 * @param nbloques number of blocks
 * @param ninodos number of inodes
 * @return if success
 */
int initSB(unsigned int nbloques, unsigned int ninodos);

/**
 * Initialize the bitmap.
 *
 * @return if success
 */
int initMB();

/**
 * Initialize the inode table.
 *
 * @return if success
 */
int initAI();

/**
 * Write a bit in the bitmap.
 *
 * @param nbloque block number
 * @param bit bit to write
 * @return if success
 */
int escribir_bit(unsigned int nbloque, unsigned int bit);

/**
 * Read a bit from the bitmap.
 *
 * @param nbloque block number
 * @return the bit read
 */
char leer_bit(unsigned int nbloque);

/**
 * Look for the first available free block on the bitmap.
 *
 * @return the reserved block number
 */
int reservar_bloque();

/**
 * Free a given block that is on use from the bitmap.
 *
 * @param nbloque block number
 * @return the freed block number
 */
int liberar_bloque(unsigned int nbloque);

/**
 * Write an inode to the inode table.
 *
 * @param ninodo inode number
 * @param inodo inode to write
 * @return if success
 */
int escribir_inodo(unsigned int ninodo, inodo_t* inodo);

/**
 * Read an inode from the inode table.
 *
 * @param ninodo inode number
 * @param inodo inode to read
 * @return if success
 */
int leer_inodo(unsigned int ninodo, inodo_t *inodo);

/**
 * Look for the first available inode on the inode table.
 *
 * @param tipo inode type
 * @param permisos inode permissions
 * @return the reserved inode number or -1 if error
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos);

/**
 * Get in which range is the given node:
 * 0: direct block
 * 1: indirect block
 * 2: double indirect block
 * 3: triple indirect block
 *
 * @param inodo inode to check
 * @param nblogico logical block number
 * @param ptr pointer to the start of the range
 * @return the range
 */
int obtener_nrangoBL(inodo_t inodo, unsigned int nblogico, unsigned int* ptr);

/**
 * Get the index of the block in the inode.
 *
 * @param inodo inode to check
 * @param nblogico logical block number
 * @param reservar flag to reserve the block
 * @return the physical block number
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros);

/**
 * Translate a logical block to a physical block.
 *
 * @param inodo inode to check
 * @param nblogico logical block number
 * @param reservar flag to reserve the block
 * @return the physical block number
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar);


/**
 * Free an inode and append it on the head of free inodes list.
 *
 * @param ninodo inode to free
 * @return if success   
 */
int liberar_inodo(unsigned int ninodo);

/**
 * Free all the used blocks inside the given inode from primerBL.
 *
 * @param primerBL start block
 * @param inodo target inode
 * @return the amount of freed blocks
 */
int liberar_bloques_inodo(unsigned int primerBL, inodo_t *inodo);

#endif //FICHERO_BASICO