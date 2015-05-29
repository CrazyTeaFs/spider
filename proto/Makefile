SRC = *.cc
CC = g++
CFLAG = -g -ggdb -Wall -c 
EXE = libmsg.a
LIB = ../lib/
all:
	protoc -I=. --cpp_out=. *.proto
	$(CC) $(SRC) $(CFLAG)
	ar -crv $(EXE) *.o
	cp $(EXE) $(LIB)
clean:
	rm -rf *.o 
	rm -rf $(EXE)

