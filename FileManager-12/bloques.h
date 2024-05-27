#ifndef BLOQUES
#define BLOQUES

#include <stdio.h>    //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h>    //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>   //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h>   // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>    //errno
#include <string.h>   // strerror()
#include "debug.h"

#define BLOCKSIZE 1024 // Number of bytes that a page has

#define EXITO        0 // para gestión correcta
#define FALLO     (-1) // para gestión incorrecta

/**
 * Mount the device, it has to be called once's
 * before any in the file system
 *
 * @return if the device is mounted correctly
 */
int bmount(const char* camino);

/**
 * Unmount the given device from the file system.
 *
 * @return if the device is mounted correctly
 */
int bumount();

/**
 * Write a full block page on the previously mounted device.
 *
 * @param nbloque the block number to write
 * @param buf the buffer to write (BLOCKSIZE bytes)
 * @return the number of bytes written
 */
int bwrite(unsigned int nbloque, const void *buf);

/**
 * Read a full block page from the previously mounted device.
 *
 * @param nbloque the block number to read
 * @param buf the buffer to read (BLOCKSIZE bytes)
 * @return the number of bytes read
 */
int bread(unsigned int nbloque, void *buf);

void mi_waitSem();

void mi_signalSem();

#endif //BLOQUES