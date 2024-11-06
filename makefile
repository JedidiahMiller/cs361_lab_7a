all: daytimeUDPclient.c

	gcc daytimeUDPclient.c -o client
	gcc daytimeUDPserver.c -o server

clean:
	rm -f client server
