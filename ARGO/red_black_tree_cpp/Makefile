export ARGO_INSTALL_DIRECTORY=${HOME}/argodsm

CC      = mpic++
CFLAGS  =-O3 -lpthread -std=c++11
CFLAGS +=-L${ARGO_INSTALL_DIRECTORY}/build/lib
CFLAGS +=-I${ARGO_INSTALL_DIRECTORY}/src
CFLAGS +=-largo -largobackend-mpi -lnuma -lrt
CFLAGS +=-Wl,-rpath=${ARGO_INSTALL_DIRECTORY}/build/lib

SOURCES=rb.cc rb_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
	EXECUTABLE=rb_nvm

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	rm -rf $(EXECUTABLE)
