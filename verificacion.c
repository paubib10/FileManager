#include "verificacion.h"

<<<<<<< HEAD
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
=======
#define DEBUG 1

int main(int argc, char const *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, RED "Error de sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        return FALLO;
    }

    STAT_t st;
    mi_stat(argv[2], &st);

#if DEBUG
    fprintf(stderr, "Directorio de simulación: %s\n", argv[2]);
#endif

    //int numentradas = (st.tamEnBytesLog / sizeof(entrada_t));
    int numentradas = 100;
    
    if (numentradas != NUMPROCESOS)
    {
        printf(RED "verificacion.c: Error en el número de entradas.\n" RESET);
        bumount();
        return -1;
    }

#if DEBUG
    fprintf(stderr, "numentradas: %i, NUMPROCESOS: %i\n", numentradas, NUMPROCESOS);
#endif

    //directorio/informe.txt
    char nfichero[100];
    sprintf(nfichero, "%s%s", argv[2], "informe.txt");
    if (mi_creat(nfichero, 7) < 0)
    {
        bumount(argv[1]);
        exit(0);
    }

    //Cargamos las entradas
    entrada_t entrs[numentradas];
    int error = mi_read(argv[2], entrs, 0, sizeof(entrs));
>>>>>>> 0f41a49a4db81b553f14d3363437c0c85599b158
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

<<<<<<< HEAD
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
=======
    int nbytes_info_f = 0;
    for (int nentr = 0; nentr < numentradas; nentr++)
    {

        //Leemos la entrada de directorio y extraemos el pid a partir del nombre
        // de la entrada
        pid_t pid = atoi(strchr(entrs[nentr].nombre, '_') + 1); //sacamos el pid a traves del nombre
        INFORMACION_t info;
        info.pid = pid;
        info.nEscrituras = 0;

        char f_prueba[128]; //camino fichero
        sprintf(f_prueba, "%s%s/%s", argv[2], entrs[nentr].nombre, "prueba.dat");

        //Buffer de N registros de escrituras
        int cant_registros_buffer_escrituras = 256 * 24; //Un multiple de BLOCKSIZE, en plataforma de 64bits
        REGISTRO_t buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        int offset = 0;
        //Mientras haya escrituras en prueba.dat
        while (mi_read(f_prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {

            //iterador escrituras buffer
            int nregistro = 0;
            while (nregistro < cant_registros_buffer_escrituras)
            {
                //Si es valida
                if (buffer_escrituras[nregistro].pid == info.pid)
                {
                    //Si es la Primera escritura validada
                    if (!info.nEscrituras)
                    {
                        info.MenorPosicion = buffer_escrituras[nregistro];
                        info.MayorPosicion = buffer_escrituras[nregistro];
                        info.PrimeraEscritura = buffer_escrituras[nregistro];
                        info.UltimaEscritura = buffer_escrituras[nregistro];
                        info.nEscrituras++;
                    }
                    else
                    {
                        //Actualizamos los datos de las fechas la primera y la última escritura si se necesita
                        if ((difftime(buffer_escrituras[nregistro].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[nregistro].nEscritura < info.PrimeraEscritura.nEscritura)
                        {
                            info.PrimeraEscritura = buffer_escrituras[nregistro];
                        }
                        if ((difftime(buffer_escrituras[nregistro].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[nregistro].nEscritura > info.UltimaEscritura.nEscritura)
                        {
                            info.UltimaEscritura = buffer_escrituras[nregistro];
                        }
                        if (buffer_escrituras[nregistro].nRegistro < info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion = buffer_escrituras[nregistro];
                        }
                        if (buffer_escrituras[nregistro].nRegistro > info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion = buffer_escrituras[nregistro];
                        }
                        info.nEscrituras++;
                    }
                }
                nregistro++;
            }
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }

#if DEBUG
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", nentr + 1, info.nEscrituras, f_prueba);
#endif
        //Añadimos la informacion del struct info en el fichero
>>>>>>> 0f41a49a4db81b553f14d3363437c0c85599b158
        char tiempoPrimero[100];
        char tiempoUltimo[100];
        char tiempoMenor[100];
        char tiempoMayor[100];
<<<<<<< HEAD

        struct tm *tm;
        tm = localtime(&informacion.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&informacion.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %d-%m-%Y %H:%M:%S", tm);
=======
        struct tm *tm;

        tm = localtime(&info.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %Y-%m-%d %H:%M:%S", tm);
>>>>>>> 0f41a49a4db81b553f14d3363437c0c85599b158

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

<<<<<<< HEAD
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
=======
        sprintf(buffer, "PID: %i\nNumero de escrituras: %i\n", pid, info.nEscrituras);
        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Primera escritura",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                asctime(localtime(&info.PrimeraEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Ultima escritura",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                asctime(localtime(&info.UltimaEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Menor posicion",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Mayor posicion",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:"
                "\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMayor po"
                "sición:\t\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                tiempoPrimero,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                tiempoUltimo,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                tiempoMenor,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                tiempoMayor);
        //Escribimos en prueba.dat y actualizamos offset
        if ((nbytes_info_f += mi_write(nfichero, &buffer, nbytes_info_f, strlen(buffer))) < 0)
        {
            printf("verifiacion.c: Error al escribir el fichero: '%s'\n", nfichero);
            bumount();
            return FALLO;
        }
    }
    bumount();
}
>>>>>>> 0f41a49a4db81b553f14d3363437c0c85599b158
