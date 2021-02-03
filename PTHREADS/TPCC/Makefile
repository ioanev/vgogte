CC=g++
CFLAGS= -lpthread -std=c++11
LDFLAGS=
SOURCES=tpcc_db.cc tpcc_nvm.cc
EXECUTABLE=tpcc_nvm

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(SOURCES) 
	    $(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	    rm $(EXECUTABLE) 
