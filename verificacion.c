#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "simulacion.h"

#define NUMPROCESOS 100
#define N 10

typedef struct INFORMACION {
    int pid;
    unsigned int nEscrituras; //validadas 
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
} Info;

void verifySyntax(char *deviceName, char *simulationDir) {
    // Montar el dispositivo virtual
    // mountDevice(deviceName);

    // Calcular el número de entradas en el directorio de simulación
    struct stat st;
    stat(simulationDir, &st);
    int numEntries = st.st_size; // Este es un marcador de posición, reemplazar con el cálculo real

    if (numEntries != NUMPROCESOS) {
        printf("ERROR: El número de entradas no coincide con NUMPROCESOS\n");
        return;
    }

    // Crear el archivo "informe.txt" dentro del directorio de simulación
    char filePath[256];
    sprintf(filePath, "%s/informe.txt", simulationDir);
    FILE *file = fopen(filePath, "w");
    if (file == NULL) {
        printf("ERROR: No se pudo crear informe.txt\n");
        return;
    }

    // Leer los directorios correspondientes a los procesos
    DIR *dir = opendir(simulationDir);
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        Info info;
        // Extraer el PID del nombre de la entrada y guardarlo en el registro info
        info.pid = atoi(entry->d_name);

        // Recorrer secuencialmente el archivo prueba.dat utilizando un buffer de N registros de escritura
        FILE *dataFile = fopen("prueba.dat", "r");
        struct REGISTRO buffer[N];
        int firstWriteValidated = 0;
        while (fread(buffer, sizeof(struct REGISTRO), N, dataFile)) {
            // Si la escritura es válida
            if (buffer->pid == info.pid) { // Verificación de validación real
                if (!firstWriteValidated) {
                    // Inicializar los registros significativos con los datos de esa escritura
                    firstWriteValidated = 1;
                    info.PrimeraEscritura = *buffer;
                    info.MenorPosicion = *buffer;
                } else {
                    // Comparar número de escritura (para obtener la primera y la última) y actualizarlas si es necesario
                    if (buffer->nEscritura < info.PrimeraEscritura.nEscritura) {
                        info.PrimeraEscritura = *buffer;
                    }
                    if (buffer->nEscritura > info.UltimaEscritura.nEscritura) {
                        info.UltimaEscritura = *buffer;
                    }
                    if (buffer->posicion < info.MenorPosicion.posicion) {
                        info.MenorPosicion = *buffer;
                    }
                    if (buffer->posicion > info.MayorPosicion.posicion) {
                        info.MayorPosicion = *buffer;
                    }
                }
                info.nEscrituras++;
            }
        }
        fclose(dataFile);

        // Añadir la información del struct info al archivo informe.txt al final
        fprintf(file, "PID: %d\n", info.pid);
        fprintf(file, "Numero de escrituras: %u\n", info.nEscrituras);
        fprintf(file, "Primera Escritura\t %u\t %u\t %s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.posicion, asctime(localtime(&info.PrimeraEscritura.fecha)));
        fprintf(file, "Ultima Escritura\t %u\t %u\t %s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.posicion, asctime(localtime(&info.UltimaEscritura.fecha)));
        fprintf(file, "Menor Posición\t %u\t %u\t %s", info.MenorPosicion.nEscritura, info.MenorPosicion.posicion, asctime(localtime(&info.MenorPosicion.fecha)));
        fprintf(file, "Mayor Posición\t %u\t %u\t %s", info.MayorPosicion.nEscritura, info.MayorPosicion.posicion, asctime(localtime(&info.MayorPosicion.fecha)));
    }
    closedir(dir);
    fclose(file);

    // Desmontar el dispositivo virtual
    // unmountDevice(deviceName);
}