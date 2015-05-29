SRC = *.cpp
CC = g++
CFLAG =-g -Wall 
INCLUDE =-I../proto -I../base
LINK =-L../lib
LIB =../lib/libmsg.a ../lib/libase.a
LINKER =-lprotobuf -lmsg -lbase -lpthread
EXE =client
all:
	$(CC) $(INCLUDE) $(SRC) $(CFLAG) $(LINK) $(LINKER) -o $(EXE)
clean:
	rm -rf *.o 
	rm -rf $(EXE)
