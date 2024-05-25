#include "bloques.h"
#include "semaforo_mutex_posix.h"
<<<<<<< HEAD

static sem_t *mutex;
static unsigned int inside_sc = 0;
=======
>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51

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
<<<<<<< HEAD
    
    deleteSem();

=======
    deleteSem();
>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51
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
<<<<<<< HEAD
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

=======
       waitSem(mutex);
}


void mi_signalSem() {
       signalSem(mutex);
}


>>>>>>> d5d1384dab3b242d5fd11a1059a2082355146f51
