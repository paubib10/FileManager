#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0;
 static sem_t *mutex;

int bmount(const char* camino) {
    // If file is already open, close it
    if (descriptor > 0) bumount();

    // Open the file
    if ((descriptor = open(camino, O_RDWR | O_CREAT, 0666)) == -1) {
        perror("Error");
        return FALLO;
    }

    // Assign descriptor and return
    return descriptor;
}

int bumount() {
    // Close the file
    int res = close(descriptor);

     if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return -1;
       }
    }


    if (res == -1) {
        perror("Error");
        return FALLO;
    }
    deleteSem();
    return res;
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
       waitSem(mutex);
}


void mi_signalSem() {
       signalSem(mutex);
}


