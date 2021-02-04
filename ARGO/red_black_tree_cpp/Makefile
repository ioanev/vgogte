CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=rb.cc rb_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=rb_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
