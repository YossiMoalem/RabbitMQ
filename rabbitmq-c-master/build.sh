rm -rf build
mkdir build && cd build
cmake ../
cmake --build .
cd librabbitmq
sudo make install
pwd
ln -s ./build/librabbitmq/librabbitmq.so ../../
ln -s ./build/librabbitmq/librabbitmq.so.1 ../../
