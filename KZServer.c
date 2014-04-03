/*
 * Author: Ken Zyma
 * Date: Spring 2014
 * course: csc552
 * Assignment: Sockets Programming Project: A Web Server
 */

/*
 * File KZServer.c contains the source for a simple web server, 
 * capable of processing multiple simultaneous service requests in
 * parellel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "sockutils.c"  //shared lib.
#include "HTTPheader.c"

#define MAX_BUFFER 255

typedef enum { false, true } bool;

void exampleWrite(int connectionFd);
void handleRequest(int connectionFd);
void getFilename(const char* inputBuffer, char* outputBuffer);
void respondToClient(const int connectionFd,char* filename, FILE* openFile);
void getFileExtension(const char* filename,char* extension);
static void signal_handler(const int sig);

bool endProgram = false;

int main(const int argc, char * const argv[]){
    int portNumber = 80;
    //check usage
    if(argc == 2) {
        //get port number from argv[1].
        sscanf(argv[1],"%d",&portNumber);
    }else{
        printf("usage: %s [port number]\n",argv[0]);
	exit(1);
    }
    
    /*Signal Handler */
    struct sigaction act, act_old;
    act.sa_handler = signal_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if((sigaction(SIGINT,  &act, &act_old) == -1)   ||
        (sigaction(SIGTERM, &act, &act_old) == -1) ||
        (sigaction(SIGHUP,  &act, &act_old) == -1)  ||
        (sigaction(SIGQUIT, &act, &act_old) == -1)) {
        perror("Error Handling Signal.");
        return 1;
    }
    
    int listenFd = 0,connectionFd = 0;
    int procID = 0;
    
    //socket()
    if((listenFd = socket(AF_INET,SOCK_STREAM,0))==-1){
        //error from 'socket'
        perror("Failure to create socket.");
        exit(1);
    };
    
    //bind()
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(portNumber);
    if (bind(listenFd, (struct sockaddr*)&serverAddress,
             sizeof(serverAddress))==-1){
        //error from 'bind'
        perror("Failure to bind socket.");
        exit(1);
    };
    
    //listen()- w/ max connect queue set to 5.
    if(listen(listenFd,5)==-1){
        //error from 'listen'
        fprintf (stderr,"Failure to mark file descriptor %d as passive socket. %s\n",
                 listenFd, strerror(errno));
        exit(1);
    };
    
    while(endProgram==false) {
        
        //accept()- accept new connection on socket.
        /*
         *note: param socket must be created with socket(), bound to an 
         *  address with bind(), and issued a successful call to listen().
         */
        connectionFd = accept(listenFd,(struct sockaddr*)NULL,NULL);
        
        //fork()
        procID = fork();
        switch (procID) {
            case -1:
                //error
                perror("Failure to handle request.[Fork Error].");
                close(connectionFd);
                break;
            case 0:
                //child process - handle request
                close(listenFd);
                //exampleWrite(connectionFd);
                handleRequest(connectionFd);
                close(connectionFd);
                exit(0);
                break;
            default:
                //parent prcoess - close Fd's
                close(connectionFd);
                //KEN->PROBOBLY SHOULD NOT WAIT HERE, DEFEATS PURPOSE OF FORKING (I BELIEVE).
                wait(NULL);
                break;
        }
    }
    
    close(listenFd);
    
    return 0;
}

/*
 * Used to test server, writes current date and time on server.
 */
void exampleWrite(int connectionFd){
    char sendBuff[1025];
    time_t ticks;
    
    ticks = time(NULL);
    snprintf(sendBuff,sizeof(sendBuff),"%.24s\r\n", ctime(&ticks));
    write(connectionFd,sendBuff,strlen(sendBuff));
}

/*
 * Read, Process and Respond to client's request.
 */
void handleRequest(int connectionFd){
    int i=0;
    int j=0;
    char requestLine[MAX_BUFFER];
    int headerCount = -1;
    char headerLine[MAX_BUFFER][MAX_BUFFER];
    
    char filename[MAX_BUFFER];
    
    //get requestLine of input stream.
    readline(connectionFd, requestLine, MAX_BUFFER);
    //get header lines of input stream.
    do{
        headerCount++;
        readline(connectionFd,headerLine[headerCount],MAX_BUFFER);
    }while(strcmp("\r\n",headerLine[headerCount])!=0);
    //subract 1 for the ending "\n"
    headerCount--;
    
    //analyze request- assuming GET
    //get filename, read from GET until HTTP/...
    getFilename(requestLine,filename);
    
    //open file, if it exists. Else send error mssg to browser
    FILE *file = fopen(filename,"r");
    respondToClient(connectionFd,filename,file);

        
    //close file
    fclose(file);

    //test purpose, print request and header lines
    /*printf("%s\n",requestLine);
    for(i=0;i<headerCount;i++){
        printf("%s\n",headerLine[i]);
    }*/
    
    //test purpose, print filename
    /*printf("filename (w/path): %s\n",filename);*/
}

/*
 * Respond To Client
 */
void respondToClient(const int connectionFd,char* filename, FILE* openFile){
    const int HTML_REQ = 1;
    const int JPEG_REQ = 2;
    const int APP_REQ = 3;
    const int FILE_NOT_FOUND = 4;
    
    bool fileExists = true;
    int type = -1;
    
    int i=0;
    int j=0;
    
    char statusLine[MAX_BUFFER];
    char headerLine[MAX_BUFFER];
    char messageBuffer[1024];
    int bytesRead = 0;
    
    //setup status line
    if (openFile == NULL){
        fileExists = false;
        openFile = fopen("./error.html","r");
    }
    
    //get MIME type
    char extension[MAX_BUFFER];
    getFileExtension(filename,extension);
    for (i=0; i<strlen(extension); i++) {
        extension[i] = tolower(extension[i]);
    }
    
    //find type of http header to use
    if (fileExists) {
        if((strcmp(extension,".html")==0) || (strcmp(extension,".htm")==0)){
            type = HTML_REQ;
        }else if(strcmp(extension,".jpg")){
            type = JPEG_REQ;
        }else{
            type = APP_REQ;
        }
        
    }else{
        type = FILE_NOT_FOUND;
    }
    sendHTTPheader(connectionFd,type);
    //send body
    while((bytesRead=read(fileno(openFile),messageBuffer,1024))>0){
         writen(connectionFd,messageBuffer,bytesRead);
    }
    writen(connectionFd,"\r\n",3);
    
}

/*
 * Get filename from HTTP request line. Assumes this will only 
 * receive http GET and is not garenteed to work otherwise.
 */
void getFilename(const char* inputBuffer, char* outputBuffer){
    int i=0;
    int j=0;
    
    //use current file dir.
    outputBuffer[0] = '.';
    
    i=3;
    j=0;
    do{
        i++;
        j++;
        outputBuffer[j] = inputBuffer[i];
    }while(outputBuffer[j]!=' ');
    outputBuffer[j] = '\0';     //end of string and remove ' '.
}

/*
 * return file extension (any characters after the (.) character
 *      in a string.
 */
void getFileExtension(const char* filename,char* extension){
    int i=0;
    int j=0;
    int startIndex = 0;
    
    //find period (.) character
    for (i=1; i<strlen(filename); i++) {
        if (filename[i]=='.') {
            startIndex = i;
        }
    }
    
    j=0;
    //manual copy--can use a strncopy as well
    for (i=startIndex; i<strlen(filename); i++) {
        extension[j] = filename[i];
        j++;
    }
    extension[strlen(filename)] = '\0';
}

/*
 * Sig handler
 */
static void signal_handler(const int sig){
    switch(sig){
        case SIGINT:
        case SIGHUP:
            //do nothing
        break;
        case SIGTERM:
        case SIGQUIT:
            endProgram = true;
            //gracefully terminate program
    }
}





