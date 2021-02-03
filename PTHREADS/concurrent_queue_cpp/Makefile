CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=cq.cc cq_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=cq_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
