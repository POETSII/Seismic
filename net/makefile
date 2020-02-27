.PHONY: default all clean

default: net net3d
all: default

net: Net.cpp Net.h
	gcc Net.cpp -o net -lm

net3d: Net3d.cpp Net3d.h
	gcc Net3d.cpp -o net3d -lm

clean:
	rm net
	rm net3d
