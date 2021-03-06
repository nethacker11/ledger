CC = g++-4.9
CFLAGS = -std=c++11
LDFLAGS = -L /usr/lib/x86_64-linux-gnu -lboost_system -pthread
DEPS = $(wildcard *.hpp)

all: ledger wallet

ledger: ledger.o
	$(CC) -o $@ $^ $(LDFLAGS)

wallet: wallet.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f ledger wallet *.o

