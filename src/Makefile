SRCEXTS = .c .cpp .c++ .cxx 
HDREXTS = .h .hpp .h++ .hxx
CC = gcc
CXX = g++
CTAGS = ctags

INCLUDES = -I/usr/local/include -I/usr/include -I ../proto
LIBS = -L/usr/local/lib -L/usr/lib -L../lib -L/usr/local/lib -lpthread -lrt -lprotobuf -lmsg
CFLAGS = -g -Wall -Wno-unused-result -O2 -c $(INCLUDES) -std=c++0x 
OFLAGS = -g -Wall -O2 $(LIBS)

SRCDIR = .
SRCS = $(wildcard $(SRCDIR)/*.cc $(SRCDIR)/*.cpp)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
HEADS = $(addsuffix .h, $(basename $(SRCS)))
TARGET = spider-server

INSTALL_PATH = ../bin

APP = $(TARGET)

all : $(APP) install

show :
	@echo $(CFLAGS)

$(APP) : $(OBJS)
	$(CXX) $^ -o $@ $(OFLAGS)

%.o:%.cc
	$(CXX) $(CFLAGS) -o $@ $<

%.o:%.cpp
	$(CXX) $(CFLAGS) -o $@ $<

clean :
	rm -Rf *.o core.*
install:
	mkdir -p $(INSTALL_PATH); install $(TARGET) $(INSTALL_PATH)
