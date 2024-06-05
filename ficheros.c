//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "ficheros.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    if(leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2) {
       fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
       return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];
    int escritos = 0;
    int temp;

    mi_waitSem();
    int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if(nbfisico == FALLO){
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    if(bread(nbfisico, buf_bloque) == FALLO){
        return FALLO;
    }

    if(primerBL == ultimoBL){
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if(bwrite(nbfisico, buf_bloque) == FALLO){
            return FALLO;
        }
        escritos = nbytes;
    } else if(primerBL < ultimoBL) {
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        temp = bwrite(nbfisico, buf_bloque);
        if(temp == FALLO){
            return FALLO;
        }
        escritos = BLOCKSIZE - desp1;

        for(int i = primerBL + 1; i < ultimoBL; i++){
            mi_waitSem();
            nbfisico = traducir_bloque_inodo(ninodo, i, 1);
            if(nbfisico == FALLO){
                mi_signalSem();
                return FALLO;
            }
            mi_signalSem();

            temp = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if(temp == FALLO){
                return FALLO;
            }
            escritos += BLOCKSIZE;
        }

        mi_waitSem();
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if(nbfisico == FALLO){
            mi_signalSem();
            return FALLO;
        }
        mi_signalSem();

        if(bread(nbfisico, buf_bloque) == FALLO){
            return FALLO;
        }

        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        if(bwrite(nbfisico, buf_bloque) == FALLO){
            return FALLO;
        }
        escritos += desp2 + 1;
    }

    mi_waitSem();
    if(leer_inodo(ninodo, &inodo) == FALLO){
        mi_signalSem();
        return FALLO;
    }

    if(inodo.tamEnBytesLog < (nbytes + offset)){
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    if(escribir_inodo(ninodo, &inodo) == FALLO){
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    return (nbytes == escritos) ? nbytes : FALLO;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }
    
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    int leidos = 0;
    if (offset >= inodo.tamEnBytesLog) {
        return leidos;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned char buf_bloque[BLOCKSIZE];

    int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
    if (primerBL == ultimoBL) {
        if (nbfisico != FALLO) {   
            if (bread(nbfisico, buf_bloque) == FALLO) {
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos = nbytes;
    } else if (primerBL < ultimoBL) {
        if (nbfisico != FALLO) { 
            if (bread(nbfisico, buf_bloque) == FALLO) {
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = BLOCKSIZE - desp1;

        for (int i = primerBL + 1; i < ultimoBL; i++) {
            nbfisico = traducir_bloque_inodo(ninodo, i, 0);
            if (nbfisico != FALLO) {
                if (bread(nbfisico, buf_bloque) == FALLO) {
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO) {
            if (bread(nbfisico, buf_bloque) == FALLO) {
                return FALLO;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }
        leidos += desp2 + 1;
    }

    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();

    return (nbytes == leidos) ? leidos : FALLO;
}


int mi_stat_f(unsigned int ninodo, STAT_t *p_stat) {
    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO ) return FALLO;

    // Copiar structura inodo a stat_t
    p_stat->tipo=inodo.tipo;
    p_stat->permisos=inodo.permisos;
    p_stat->atime=inodo.atime;
    p_stat->mtime=inodo.mtime;
    p_stat->ctime=inodo.ctime;
    p_stat->nlinks=inodo.nlinks;
    p_stat->tamEnBytesLog=inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados=inodo.numBloquesOcupados;

    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
    inodo_t inodo;
    if(leer_inodo(ninodo, &inodo) == FALLO) return FALLO;

    // Modificar los permisos del inodo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == FALLO) return FALLO;

    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    inodo_t inodo;
    if(leer_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }

    int primerBL;
    if(nbytes%BLOCKSIZE==0){
        primerBL=nbytes/BLOCKSIZE;
    }else{
        primerBL=nbytes/BLOCKSIZE+1;
    }
    int liberados=liberar_bloques_inodo(primerBL,&inodo);
    if(liberados==FALLO){
        return FALLO;
    }
    inodo.tamEnBytesLog=nbytes;
    inodo.numBloquesOcupados-=liberados;
    inodo.mtime=time(NULL);
    inodo.ctime=time(NULL);
    if(escribir_inodo(ninodo,&inodo)==FALLO){
        return FALLO;
    }
    return liberados;
}