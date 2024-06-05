//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "simulacion.h"

#define DEBUGON 0

int acabados = 0;

// Enterrador
void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char const *argv[]) {
    // Comprobamos sintaxis
    if (argc != 2) {
        fprintf(stderr,RED"Sintaxis: ./simulacion <disco>\n");
        return FALLO;
    }

    // Montamos el dispositivo
    if (bmount(argv[1]) == FALLO) {
        exit(0);
    }

    // Creamos el directorio
    char camino[21] = "/simul_";
    time_t time_now;
    time(&time_now);
    struct tm *tm = localtime(&time_now);
    sprintf(camino + strlen(camino), "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    // Creamos el directorio
    if(mi_creat(camino, 6) == FALLO) {
        fprintf(stderr,RED"Error al crear el directorio '%s'\n"RESET, camino);
        exit(0);
    }

    fprintf(stderr, "*** SIMULACION DE %i PROCESOS REALIZANDO CADA UNO %i ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);

    // Asociamos la señal SIGCHLD al manejador enterrador
    signal(SIGCHLD, reaper);

    pid_t pid;
    for(int proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        
        pid = fork();

        if(pid == 0) { // Proceso hijo
            if(bmount(argv[1]) < 0) {
                return FALLO;
            }

            // Creamos directorio del proceso hijo
            char nombreDirectorio[38];
            sprintf(nombreDirectorio, "%sproceso_%d/", camino, getpid());

            if(mi_creat(nombreDirectorio, 6) == -1) {
                fprintf(stderr,RED"Error al crear el directorio del proceso\n"RESET);
                exit(0);
            }

            // Creamos el fichero prueba.dat
            char nombreFichero[48];
            sprintf(nombreFichero, "%sprueba.dat", nombreDirectorio);

            if(mi_creat(nombreFichero, 6) == -1) {
                fprintf(stderr,RED"Error al crear el fichero prueba.dat del proceso\n"RESET);
                bumount();
                exit(0);
            }

            // Inicializamos numeros aleatorios
            srand(time(NULL) + getpid());

            for(int nescritura = 0; nescritura < NUMESCRITURAS; nescritura++) {
                // Inicializamos el registro
                REGISTRO_t registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura + 1;
                registro.nRegistro = rand() % REGMAX; // [0, 499.999]

                mi_write(nombreFichero, &registro, registro.nRegistro * sizeof(REGISTRO_t), sizeof(REGISTRO_t));

                #if DEBUGON
                    fprintf(stderr, "[simulacion.c -> Escritura %i en %s]\n", nescritura + 1, nombreFichero);
                #endif

                usleep(50000); // 0.05 segundos
            }

            #if DEBUG12
                fprintf(stderr,GRAY"[Proceso %d: Completadas %d escrituras en %s]\n"RESET, proceso, NUMESCRITURAS, nombreFichero);
            #endif

            // Desmontamos el dispositivo hijo
            if(bumount() < 0) {
                return FALLO;
            }
            exit(0);
        }
        
        usleep(200000); // 0.15 segundos
    }

    while(acabados < NUMPROCESOS) {
        pause();
    }
    
    fprintf(stderr, "Todos los procesos han terminado\n");
    
    // Desmontamos el dispositivo padre
    if(bumount() < 0) {
        return FALLO;
    }

    return FALLO;
}