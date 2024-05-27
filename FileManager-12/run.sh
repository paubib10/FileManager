make clean install all > /dev/null
echo "System build"

./out/mi_mkfs disco 100000
echo "Disk created"