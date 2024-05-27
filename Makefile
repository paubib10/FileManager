CC=gcc
CFLAGS=-c -g -Wall -std=gnu99
LDFLAGS=-pthread

INCLUDES=bloques.h ficheros_basico.h ficheros.h directorios.h semaforo_mutex_posix.h #simulacion.h
PROGRAMS=mi_mkfs leer_sf escribir leer permitir truncar mi_mkdir mi_chmod mi_ls mi_stat mi_touch mi_link mi_rm mi_escribir mi_cat mi_escribir_varios simulacion mi_rn #verificacion

# Map the programs and libraries to the sources
LIBRARIES=$(INCLUDES:.h=.o)
SOURCES=$(addsuffix .c, $(PROGRAMS)) $(INCLUDES:.h=.c)
OBJS=$(SOURCES:.c=.o)

all: install $(OBJS) $(PROGRAMS)

$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(addprefix bin/,$(LIBRARIES)) bin/$@.o -o out/$@

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o bin/$@ -c $<

install:
	mkdir -p bin/ out/

.PHONY: clean
clean:
	rm -rf bin/* out/* disco* ext*
