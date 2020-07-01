cd ../
if [ ! -d "bin" ]; then
  mkdir bin
fi
cd bin
if [ ! -d "test" ]; then
  mkdir test
fi
cd test
cmake -DAETHER_CONFIG='../../test/config.h' ../../test/
make
./obj_test
