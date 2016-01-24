make clean
make
#TODO: after layer2 moves from example/2, we wont need the "ln -s" lines
ln -fs src/libamqpcpp.so .
ln -fs src/libamqpcpp.a .