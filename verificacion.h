//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "simulacion.h"

typedef struct { 
    int pid;
    unsigned int nEscrituras; // validadas
    REGISTRO_t PrimeraEscritura;
    REGISTRO_t UltimaEscritura;
    REGISTRO_t MenorPosicion;
    REGISTRO_t MayorPosicion;

} INFORMACION_t;