// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#ifndef BLOQUES
#define BLOQUES

#include <stdio.h>    // printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h>    // O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> // S_IRUSR, S_IWUSR
#include <stdlib.h>   // exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h>   // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>    // errno
#include <string.h>   // strerror()
#include "debug.h"

#define BLOCKSIZE 1024 // Número de bytes que tiene una página

#define EXITO        0 // Para gestión correcta
#define FALLO     (-1) // Para gestión incorrecta

/**
 * Monta el dispositivo, debe ser llamado una vez
 * antes de cualquier operación en el sistema de archivos
 *
 * @return si el dispositivo se monta correctamente
 */
int bmount(const char* camino);

/**
 * Desmonta el dispositivo dado del sistema de archivos.
 *
 * @return si el dispositivo se desmonta correctamente
 */
int bumount();

/**
 * Escribe una página de bloque completa en el dispositivo montado previamente.
 *
 * @param nbloque el número de bloque a escribir
 * @param buf el buffer a escribir (BLOCKSIZE bytes)
 * @return el número de bytes escritos
 */
int bwrite(unsigned int nbloque, const void *buf);

/**
 * Lee una página de bloque completa desde el dispositivo montado previamente.
 *
 * @param nbloque el número de bloque a leer
 * @param buf el buffer para leer (BLOCKSIZE bytes)
 * @return el número de bytes leídos
 */
int bread(unsigned int nbloque, void *buf);

void mi_waitSem();

void mi_signalSem();

#endif // BLOQUES
