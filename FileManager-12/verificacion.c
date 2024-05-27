#inculde "directorios.h"

#define NUMPROCESOS 100

struct INFORMACION {
    int pid;
    unsigned int nEscrituras; //validadas 
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};

void print_date(time_t date) {
    char buffer[26];
    struct tm* tm_info;

    tm_info = localtime(&date);
    strftime(buffer, 26, "%a %d-%m-%Y %H:%M:%S", tm_info);
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {
    // Comprobar la sintaxis de los argumentos de entrada
    if (argc != 3) {
        fprintf(stderr, "Uso: verificacion <nombre_dispositivo> <directorio_simulacion>\n");
        return FALLO;
    }

    // Montar el dispositivo virtual
    // Implementar la lógica de montaje del dispositivo virtual

    // Calcular el número de entradas del directorio de simulación a partir del stat de su inodo
    struct stat st;
    if (stat(argv[2], &st) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    int num_entries = st.st_size / sizeof(struct dirent);

    // Si el número de entradas no es igual a NUMPROCESOS, entonces lanzar un error
    if (num_entries != NUMPROCESOS) {
        fprintf(stderr, "ERROR: El número de entradas del directorio no coincide con NUMPROCESOS\n");
        exit(EXIT_FAILURE);
    }

    // Crear el fichero "informe.txt" dentro del directorio de simulación
    char report_file_path[256];
    snprintf(report_file_path, sizeof(report_file_path), "%s/informe.txt", argv[2]);
    int fd = open(report_file_path, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Leer los directorios correspondientes a los procesos
    // Implementar la lógica de procesamiento de las entradas del directorio

    // Desmontar el dispositivo virtual
    // Implementar la lógica de desmontaje del dispositivo virtual

    return 0;
}