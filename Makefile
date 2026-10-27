serv:
	gcc serv.c list.c -lpthread -Wformat -Wall -o server

cli:
	gcc cli.c -lpthread -Wformat -Wall -o client

clean:
	rm -f server client
