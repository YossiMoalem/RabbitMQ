rm -rf build
mkdir build && cd build
cmake ../
cmake --build .
cd librabbitmq
sudo make install
pwd
ln -sf ./build/librabbitmq/librabbitmq.so ../../
ln -sf ./build/librabbitmq/librabbitmq.so.1 ../../
