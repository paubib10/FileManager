#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;
static unsigned int inside_sc = 0;

static int descriptor = 0;

int bmount(const char* camino) {
    
    if(descriptor > 0) {
        close(descriptor);
    }

    umask(000);

    // Open the file
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if(!mutex) { 
        mutex = initSem();
        if (mutex == SEM_FAILED) {
            return FALLO;
        }
        
    }

    // Assign descriptor and return
    return descriptor;
}

int bumount() {

    descriptor = close(descriptor);

    // Comprobamos si se ha cerrado correctamente
    if (close(descriptor) == -1) {
        fprintf(stderr, "Error al cerrar el fichero.\n");
        return FALLO;
    }
    
    deleteSem();
    
    // Se ha cerrado correctamente
    return EXITO;
}

int bwrite(unsigned int nbloque, const void *buf) {
    // Move the file pointer to the block
    off_t res = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    if (res == -1) {
        perror("Error");
        return FALLO;
    }

    // Write the block
    res = write(descriptor, buf, BLOCKSIZE);

    if (res == -1) {
        perror("Error");
        return FALLO;
    }

    return (int) res;
}

int bread(unsigned int nbloque, void *buf) {
    // Move the file pointer to the block
    off_t res = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    if (res == -1) {
        perror("Error");
        return FALLO;
    }

    // Read the block
    res = read(descriptor, buf, BLOCKSIZE);

    if (res == -1) {
        perror("Error");
        return FALLO;
    }

    return (int) res;
}

void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}

