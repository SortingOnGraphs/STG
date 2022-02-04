all: stg 

stg: main.o STG.o
	g++ main.o STG.o -g -I. -I.. -o stg -lm

main.o: main.cpp STG.h
	g++ -Wall -DHAVE_CONFIG_H -DLINUX -g -O2 -I. -I..  -c main.cpp

STG.o: STG.cpp STG.h
	g++ -Wall -DHAVE_CONFIG_H -DLINUX  -g -O2 -I. -I..  -c STG.cpp

clean:
	rm -rf *o stg 
