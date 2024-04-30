#include "ficheros.h"
#include "debug.h"

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    if (nbytes <= 0) return 0;

    inodo_t inodo;
    // Read and check inode permissions
    if (leer_inodo(ninodo, &inodo) < 0) {
        ERR("mi_write_f", "No se ha podido leer el inodo");
        return FALLO;
    }

    if((inodo.permisos & 2) != 2) {
        ERR("mi_write_f", "Error: No hay permisos de escritura");
        return FALLO;
    }

    int primerBL = (int) offset / BLOCKSIZE;
    int ultimoBL = (int) (offset + nbytes - 1) / BLOCKSIZE;

    int desp1 = (int) offset % BLOCKSIZE;
    int desp2 = (int) (offset + nbytes - 1) % BLOCKSIZE;

    unsigned bytes = 0;
    unsigned char buf_bloque[BLOCKSIZE];

    // Translate the logical block to a physical block and check if it exists
    int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if(nbfisico < 0) return FALLO;

    // Read the inode content from the disk
    if(bread(nbfisico, buf_bloque) == FALLO) return FALLO;

    if(primerBL == ultimoBL) {
        // Update the content of the block and save it
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        if(bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes += nbytes;
    } else {
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        int bytesWritten;
        if((bytesWritten = bwrite(nbfisico, buf_bloque)) == FALLO) return FALLO;

        bytes += bytesWritten - desp1;

        // Read the middle blocks
        for(int i = primerBL + 1; i < ultimoBL; i++) {
            nbfisico = traducir_bloque_inodo(&inodo, i, 1);
            if(nbfisico == FALLO) return FALLO;

            // Update the content of the block and save it
            int displacement = (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE;
            bytes += bwrite (nbfisico, buf_original + displacement);
        }

        // Read the last block
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        if (nbfisico == FALLO) return FALLO;

        if(bread(nbfisico, buf_bloque) == FALLO) return FALLO;

        memcpy(
                buf_bloque,
                buf_original + (nbytes - desp2 - 1),
                desp2 + 1
        );

        if (bwrite(nbfisico, buf_bloque) == FALLO) return FALLO;

        bytes += desp2 + 1;
    }

    if (inodo.tamEnBytesLog < offset + bytes) {
        inodo.tamEnBytesLog = offset + bytes;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        ERR("mi_write_f", "No se ha podido escribir el inodo");
        return FALLO;
    }

    return (int) bytes;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    if (nbytes <= 0) return 0;

    int bfisico;
    int bytesRead = 0;
    unsigned char auxBuff[BLOCKSIZE];

    inodo_t inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        ERR("mi_read_f", "No se ha podido leer el inodo")
        return FALLO;
    }

    // Check R permissions
    if ((inodo.permisos & 4) != 4) {
        ERR("mi_read_f", "Error: no hay permisos de lectura")
        return FALLO;
    }

    // Check the edges of the file
    if (offset>= inodo.tamEnBytesLog) return bytesRead;
    if (offset + nbytes >= inodo.tamEnBytesLog) nbytes = inodo.tamEnBytesLog - offset;

    // Calculate displacements
    int desp1 = (int) offset % BLOCKSIZE;
    int desp2 = (int) (offset + nbytes - 1) % BLOCKSIZE;

    // Calculate the first and last block
    int primerBloqueL = (int) offset / BLOCKSIZE;
    int ultimoBloqueL = (int) (offset + nbytes - 1) / BLOCKSIZE;

    bfisico = traducir_bloque_inodo(&inodo, primerBloqueL, 0);

    if(primerBloqueL == ultimoBloqueL) {
        if(bfisico != FALLO) {
            if(bread(bfisico, auxBuff) == FALLO) return FALLO;

            memcpy(buf_original, auxBuff + desp1, nbytes);
        }

        bytesRead += nbytes;
    } else {
        // Read the first block
        if(bfisico != FALLO) {
            if (bread(bfisico, auxBuff) == FALLO) return FALLO;
            memcpy(buf_original, auxBuff + desp1, BLOCKSIZE - desp1);
        }

        bytesRead += BLOCKSIZE - desp1;

        // Read the middle blocks
        for(int i = primerBloqueL + 1; i < ultimoBloqueL; i++) {
            bfisico = traducir_bloque_inodo(&inodo, i, 0);

            if (bfisico != FALLO) {
                if (bread(bfisico, auxBuff)==-1) return FALLO;

                memcpy(buf_original+bytesRead,auxBuff, BLOCKSIZE);
            }

            bytesRead = bytesRead + BLOCKSIZE;
        }

        // Read the last block
        bfisico = traducir_bloque_inodo(&inodo, ultimoBloqueL, 0);
        if (bfisico != -1) {
            if (bread(bfisico, auxBuff)==-1) return FALLO;

            memcpy((buf_original+nbytes)-desp2-1, auxBuff, desp2+1);
        }

        bytesRead = bytesRead + desp2 + 1;
    }

    // Update the access time
    inodo.atime = time(NULL);
    if(escribir_inodo(ninodo, &inodo) == FALLO) {
        ERR("mi_read_f", "No se ha podido escribir el inodo")
        return FALLO;
    }

    return bytesRead;
}

int mi_stat_f(unsigned int ninodo, stat_t *p_stat) {
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
    if(leer_inodo(ninodo, &inodo) == -1) return FALLO;

    // Modificar los permisos del inodo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == -1) return FALLO;

    return EXITO;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    int liberados;
    inodo_t inodo;

    // Leer el inodo
    if(leer_inodo(ninodo, &inodo) == FALLO) {
        ERR("mi_truncar_f", "Error al leer el inodo")
        return FALLO;
    }

    // Comprobar permisos
    if((inodo.permisos & 2) != 2) {
        ERR("mi_truncar_f", "Error: permisos de escritura")
        return FALLO;
    }

    // Comprobar que no se trunca más de lo que hay
    if(nbytes > inodo.tamEnBytesLog) {
        ERR("mi_truncar_f", "Error: nbytes > tamEnBytesLog")
        return FALLO;
    }

    // Obtener bloque lógico
    int primerBL;
    if(nbytes % BLOCKSIZE == 0) primerBL = (int) nbytes / BLOCKSIZE;
    else primerBL = (int) (nbytes / BLOCKSIZE) + 1;

    // Liberar bloques 
    liberados = liberar_bloques_inodo(primerBL, &inodo);

    // Actualizar inodo
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    // Escribir inodo
    if(escribir_inodo(ninodo, &inodo) == FALLO) {
        ERR("mi_truncar_f", "Error al escribir el inodo")
        return FALLO;
    }

    // Devolver el número de bloques liberados
    return liberados;
}