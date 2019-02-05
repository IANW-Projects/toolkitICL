#!/bin/bash
rm *.cl
rm log.txt
rm *.h5

cd ..
cmake ..
make clean
make -j 4

cd bin

