CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=sps_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=sps_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
