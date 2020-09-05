
CC=g++

all: daytime-server use-dlopen tls

daytime-server: daytime-server.cc
	$(CC) -o daytime-server daytime-server.cc

use-dlopen: use-dlopen.cc
	$(CC) -o use-dlopen use-dlopen.cc -ldl

tls: tls_server.c
	$(CC) `pkg-config --cflags openssl` -o $@ $^ `pkg-config --libs openssl`
	@echo "Don't forget to create cert.pem and key.pem"

clean:
	rm -f daytime-server use-dlopen tls

