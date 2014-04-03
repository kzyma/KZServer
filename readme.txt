############################# INTRO ################################
  Author: Ken Zyma
  Date: Spring 2014
  Course: csc552
  Assignment: Sockets Programming Project: A Web Server

  usage:
  KZServer [port number]

KZServer is a web server that is capable of processing multiple 
simultaneous service requests in parallel. This implements version 
1.0 of HTTP, were separate HTTP requests are sent for each component 
of the Web page.

######################### COMPILATION ###############################

Running GNU’s make utility will compile and link to an executable 
named KZServer.

########################## HOW TO RUN ###############################

Run the executable KZServer, passing in one argument representing the port number
to ‘listen on’. 

	ex: ./KZServer 8080

######################## DESIGN OVERVIEW ############################

The design was relatively straightforward for this project. The program 
set’s up a socket on the designated port and then listen’s for requests. 
When a request is make, it ‘forks’
a child process to execute and continues listening for other requests.
The only part of my project I could see differing greatly from other projects
is how I handled using the HTTPheader.c file. While others might have changed 
this file to take a file descriptor instead of stream, I simply converted the 
connection file descriptor to a stream before passing it to the function
sendHTTPheader().

######################## OTHER (BUGS/ECT) ############################

Ambiguities in the specification:
None

Known Bugs:
The file sockutils.c was included in the make for this program, when it should 
instead link to the shared library for use. I was having problems doing so, and 
thus will have to work out the use of shared libraries for the next iteration of 
this project, or next project.
