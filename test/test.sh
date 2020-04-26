cd ../
mkdir bin
cd bin
mkdir obj
cd obj
cmake -DAETHER_CONFIG='../../test/config.h' ../../aether/obj/
make

cd ../
mkdir test
cd test
cmake -DAETHER_CONFIG='../../test/config.h' ../../test/
make
./obj_test
