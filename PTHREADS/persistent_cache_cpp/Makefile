CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=pc_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=pc_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
