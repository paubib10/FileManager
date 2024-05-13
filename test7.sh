# Adelaida
# test6d.sh  #truncados parciales texto varios bloques

clear
make clean
make
echo -e "\x1B[38;2;17;245;120m################################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkfs disco 100000\x1b[0m"
echo -e "\x1B[38;2;17;245;120m#inicializamos el sistema de ficheros con 100.000 bloques\x1b[0m"
./out/mi_mkfs disco 100000
echo
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./out/leer_sf disco
echo