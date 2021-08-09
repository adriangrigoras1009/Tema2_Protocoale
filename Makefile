# Protocoale de comunicatii:
# Makefile

# Portul pe care asculta serverul (de completat)
PORT = 

# Adresa IP a serverului (de completat)
IP_SERVER = 

ID_SERVER =

all: server subscriber

subscriber: client.c
	gcc -Wall -g client.c -o subscriber
server: server.c
	gcc -Wall -g server.c -o server

.PHONY: clean run_server run_subscriber

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_subscriber:
	./subscriber ${ID_SERVER} ${IP_SERVER} ${PORT}

clean:
	rm -f server subscriber
