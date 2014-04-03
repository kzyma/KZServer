########################################################
#
#     Author: Ken Zyma
#       Date: Spring 2014
#     Course: CSC552
# Assignment: Sockets Programming Project: A Web Server
#
##########################################################

KZServer: KZServer.c sockutils.o HTTPheader.o
	gcc -o KZServer KZServer.c sockutils.o HTTPheader.o -lnsl -lsocket

HTTPheader.o: sockutils.o
	gcc -c HTTPheader.c sockutils.o

sockutils.o:
	gcc -c sockutils.c

clean:
	rm -rf *o
