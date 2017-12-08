CC = g++
CFLAGS = -std=c++11 -Iinclude -static
LDFLAGS = -Llib -lboost_system -pthread

all: ledger

ledger: ledger.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp %.hpp
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f ledger *.o

