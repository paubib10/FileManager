// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex; // Declaración de un puntero a un semáforo
static unsigned int inside_sc = 0; // Contador para controlar la entrada en la sección crítica

static int descriptor = 0; // Descriptor de archivo

int bmount(const char* camino) {
    
    if(descriptor > 0) {
        close(descriptor); // Cierra el descriptor si ya está abierto
    }

    umask(000); // Establece los permisos del archivo

    // Abre el archivo
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if(!mutex) { 
        mutex = initSem(); // Inicializa el semáforo
        if (mutex == SEM_FAILED) {
            return FALLO; // Error en la inicialización del semáforo
        }
    }

    // Asigna el descriptor y lo retorna
    return descriptor;
}

int bumount() {

    descriptor = close(descriptor); // Cierra el descriptor

    // Comprobamos si se ha cerrado correctamente
    if (close(descriptor) == -1) {
        fprintf(stderr, "Error al cerrar el fichero.\n");
        return FALLO; // Error al cerrar el archivo
    }
    
    deleteSem(); // Elimina el semáforo
    
    // Se ha cerrado correctamente
    return EXITO;
}

int bwrite(unsigned int nbloque, const void *buf) {
    // Mueve el puntero del archivo al bloque
    off_t res = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    if (res == -1) {
        perror("Error"); // Error en el movimiento del puntero
        return FALLO;
    }

    // Escribe el bloque
    res = write(descriptor, buf, BLOCKSIZE);

    if (res == -1) {
        perror("Error"); // Error en la escritura del bloque
        return FALLO;
    }

    return (int) res; // Retorna el número de bytes escritos
}

int bread(unsigned int nbloque, void *buf) {
    // Mueve el puntero del archivo al bloque
    off_t res = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);

    if (res == -1) {
        perror("Error"); // Error en el movimiento del puntero
        return FALLO;
    }

    // Lee el bloque
    res = read(descriptor, buf, BLOCKSIZE);

    if (res == -1) {
        perror("Error"); // Error en la lectura del bloque
        return FALLO;
    }

    return (int) res; // Retorna el número de bytes leídos
}

void mi_waitSem() {
    if (!inside_sc) { // inside_sc == 0, no se ha hecho ya un wait
        waitSem(mutex); // Espera el semáforo
    }
    inside_sc++; // Incrementa el contador de la sección crítica
}

void mi_signalSem() {
    inside_sc--; // Decrementa el contador de la sección crítica
    if (!inside_sc) {
        signalSem(mutex); // Señaliza el semáforo
    }
}
