HEADERS="-I../coil/"
LIBS="-lzmq"

all: bc_evaluator

bc_evaluator: bc_evaluator.o ptreal.o paramcoil.o blockcoil.o cblock.o plane.o evalcoil.o errhandler.o latticegrid.o
	g++ *.o -o bc_evaluator ${LIBS}

bc_evaluator.o: bc_evaluator.cpp
	g++ bc_evaluator.cpp -c ${HEADERS}

ptreal.o:
	g++ ../coil/ptreal.cpp -c 

paramcoil.o:
	g++ ../coil/paramcoil.cpp -c 

blockcoil.o:
	g++ ../coil/blockcoil.cpp -c 

cblock.o:
	g++ ../coil/cblock.cpp -c 

plane.o:
	g++ ../coil/plane.cpp -c 

evalcoil.o:
	g++ ../coil/evalcoil.cpp -c 

errhandler.o:
	g++ ../coil/errhandler.cpp -c 

latticegrid.o:
	g++ ../coil/latticegrid.cpp -c 

clean:
	rm -f bc_evaluator
	rm *.o
