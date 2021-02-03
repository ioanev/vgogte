CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=tatp_db.cc tatp_nvm.cc
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=tatp_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
