#include "verificacion.h"

int main(int argc, char const *argv[])
{
    // Comprobamos sintaxis
    if (argc != 3)
    {
        fprintf(stderr, RED "Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulacion>\n" RESET);
        return FALLO;
    }

    // Montamos el dispositivo
    if (bmount(argv[1]) == -1)
        return FALLO;

    // Calculamos el nº de entradas del directorio de simulación
    STAT_t stat;
    mi_stat(argv[2], &stat);

    fprintf(stderr, "dir_sim: %s\n", argv[2]);

    int numEntradas = stat.tamEnBytesLog / sizeof(entrada_t);

    if (numEntradas != NUMPROCESOS)
    {
        fprintf(stderr, RED "Error en el nº de entradas (%i).\n" RESET, numEntradas);

        // Desmontamos el dispositivo
        if (bumount() == -1)
        {
            return FALLO;
        }
        return FALLO;
    }

    fprintf(stderr, "numentradas: %i NUMPROCESOS: %i\n", numEntradas, NUMPROCESOS);

    // Creamos el fichero "informe.txt" en el directorio de simulación
    char informe[100];
    sprintf(informe, "%s%s", argv[2], "informe.txt");

    if (mi_creat(informe, 7) < 0)
        return FALLO;

    // Leemos los directorios correspondientes a cada proceso
    // MEJORA REALIZADA: Leemos todas las entradas al principio del bucle
    entrada_t entradas[numEntradas];
    // memset(entradas, 0, sizeof(entradas));

    int error = mi_read(argv[2], &entradas, 0, sizeof(entradas));
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    int escritos = 0;
    for (int entrada = 0; entrada < numEntradas; entrada++)
    {
        // Extraemos el PID
        pid_t pid = atoi(strchr(entradas[entrada].nombre, '_') + 1);

        // Guardamos la información
        INFORMACION_t informacion;
        informacion.pid = pid;
        informacion.nEscrituras = 0;

        char buffer_fichero[256];
        sprintf(buffer_fichero, "%s%s/%s", argv[2], entradas[entrada].nombre, "prueba.dat");

        // Recorremos secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras
        int cant_registros_buffer_escrituras = 256;
        REGISTRO_t buffer_escrituras[cant_registros_buffer_escrituras];
        // memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        int offset = 0;
        while (mi_read(buffer_fichero, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {

            int registro_validadas = 0;

            while (registro_validadas < cant_registros_buffer_escrituras)
            {
                if (registro_validadas < sizeof(buffer_escrituras) / sizeof(buffer_escrituras[0]))
                { // Comprobación de límites del array
                    if (buffer_escrituras[registro_validadas].pid == pid)
                    { // Escritura es válida
                        if (informacion.nEscrituras == 0)
                        { // Primera escritura del proceso

                            // Inicializamos los registros significativos con los datos de esa escritura
                            informacion.PrimeraEscritura = buffer_escrituras[registro_validadas];
                            informacion.UltimaEscritura = buffer_escrituras[registro_validadas];
                            informacion.MenorPosicion = buffer_escrituras[registro_validadas];
                            informacion.MayorPosicion = buffer_escrituras[registro_validadas];
                        }
                        else
                        {

                            if (buffer_escrituras[registro_validadas].nEscritura < informacion.PrimeraEscritura.nEscritura)
                            {
                                informacion.PrimeraEscritura = buffer_escrituras[registro_validadas];
                            }
                            else if (buffer_escrituras[registro_validadas].nEscritura > informacion.UltimaEscritura.nEscritura)
                            {
                                informacion.UltimaEscritura = buffer_escrituras[registro_validadas];
                            }

                            if (buffer_escrituras[registro_validadas].nRegistro < informacion.MenorPosicion.nRegistro)
                            {
                                informacion.MenorPosicion = buffer_escrituras[registro_validadas];
                            }
                            else if (buffer_escrituras[registro_validadas].nRegistro > informacion.MayorPosicion.nRegistro)
                            {
                                informacion.MayorPosicion = buffer_escrituras[registro_validadas];
                            }
                        }
                        informacion.nEscrituras++;
                    }
                }
                registro_validadas++;
            }
            STAT_t stat_fichero;
            if (mi_stat(buffer_fichero, &stat_fichero) < 0){
                fprintf(stderr, "AAAAAHHHHHH");
                return FALLO;
            }

            if (offset < stat_fichero.tamEnBytesLog){ // Comprobación de límites del archivo
                memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
                offset += sizeof(buffer_escrituras);
            }
        }

#if DEBUG13
        fprintf(stderr, GRAY "[%i) %i escrituras validadas en %s]\n" RESET, entrada + 1, informacion.nEscrituras, buffer_fichero);
#endif

        // Añadimos la información al fichero "informe.txt"
        char tiempoPrimero[100];
        char tiempoUltimo[100];
        char tiempoMenor[100];
        char tiempoMayor[100];

        struct tm *tm;
        tm = localtime(&informacion.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %d-%m-%Y %H:%M:%S", tm);

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\n"
                "Primera escritura:\t%d\t%d\t%s\n"
                "Ultima escritura:\t%d\t%d\t%s\n"
                "Menor posicion:\t\t%d\t%d\t%s\n"
                "Mayor posicion:\t\t%d\t%d\t%s\n\n",
                pid, informacion.nEscrituras,
                informacion.PrimeraEscritura.nEscritura, informacion.PrimeraEscritura.nRegistro, tiempoPrimero,
                informacion.UltimaEscritura.nEscritura, informacion.UltimaEscritura.nRegistro, tiempoUltimo,
                informacion.MenorPosicion.nEscritura, informacion.MenorPosicion.nRegistro, tiempoMenor,
                informacion.MayorPosicion.nEscritura, informacion.MayorPosicion.nRegistro, tiempoMayor);

        if ((escritos += mi_write(informe, &buffer, escritos, strlen(buffer))) < 0)
        {
            fprintf(stderr, RED "Error al escribir en el fichero informe.txt\n" RESET);
            if (bumount() == -1)
            {
                return FALLO;
            }
            return FALLO;
        }
        // free(buffer_fichero);
    }

    // Desmontamos el dispositivo
    if (bumount() == -1)
    {
        return FALLO;
    }
}