CC=gcc
CXX=g++
CXXFLAGS=-g -std=gnu++11 -pedantic
CFLAGS=-g -std=gnu11 -pedantic 

main : main.cpp spidev_test.o SparkFunLSM9DS1.o

spidev_test.o : spidev_test.c spidev_test.h

SparkFunLSM9DS1.o : SparkFunLSM9DS1.cpp SparkFunLSM9DS1.h

clean :
	rm -rf *.o main

remake : clean main

.PHONY : clean remake

