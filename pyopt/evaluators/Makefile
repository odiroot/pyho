HEADERS="-I../blockcoil/"
LIBS="-lzmq"

all: bc_evaluator

bc_evaluator: bc_evaluator.o ptreal.o paramcoil.o blockcoil.o cblock.o plane.o evalcoil.o errhandler.o latticegrid.o
	g++ *.o -o bc_evaluator ${LIBS}

bc_evaluator.o: bc_evaluator.cpp
	g++ bc_evaluator.cpp -c ${HEADERS}

ptreal.o:
	g++ ../blockcoil/ptreal.cpp -c 

paramcoil.o:
	g++ ../blockcoil/paramcoil.cpp -c 

blockcoil.o:
	g++ ../blockcoil/blockcoil.cpp -c 

cblock.o:
	g++ ../blockcoil/cblock.cpp -c 

plane.o:
	g++ ../blockcoil/plane.cpp -c 

evalcoil.o:
	g++ ../blockcoil/evalcoil.cpp -c 

errhandler.o:
	g++ ../blockcoil/errhandler.cpp -c 

latticegrid.o:
	g++ ../blockcoil/latticegrid.cpp -c 

clean:
	rm -f bc_evaluator
	rm *.o
