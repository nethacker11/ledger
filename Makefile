CC = g++
CFLAGS = -std=c++11 
LDFLAGS = 

all: ledger

ledger: ledger.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp %.hpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f ledger *.o

