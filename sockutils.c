// socket_utils.c
// November 9, 2000
// Lisa Frye
// from Stevens, UNIX Network Programming Vol 1 book, page 78-79
// readn, writen, and readline functions to use with sockets

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

ssize_t readn(int fd, void *vptr, size_t n)
{
  size_t   nleft;
  ssize_t  nread;
  char     *ptr;

  ptr = vptr;
  nleft = n;
  
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)   // if get interrupted by a signal 
	nread = 0;          // just call read again
      else
	return (-1);        // error
    }   // end if read
    
    else if (nread == 0)
      break;                // EOF  

    nleft -= nread;
    ptr += nread;
  }   // end while

  return (n - nleft);      // return >= 0
}   // end function readn



ssize_t writen(int fd, void *vptr, size_t n)
{
  size_t   nleft;
  ssize_t  nwritten;
  const char     *ptr;

  ptr = vptr;
  nleft = n;
  
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (errno == EINTR)   // if get interrupted by a signal 
	nwritten = 0;       // just call write again
      else
	return (-1);        // error
    }   // end if write

    nleft -= nwritten;
    ptr += nwritten;
  }  //   end while

  return (n);
}   // end function writen


ssize_t readline(int fd, void *vptr, size_t maxlen)
{
  ssize_t n, rc;
  char c, *ptr;

  ptr = vptr;
  
  for (n = 1; n < maxlen; n++) {
  again:
    if ((rc = read(fd, &c, 1)) == 1) {
      *ptr++ = c;
      if (c == '\n')
	break;       // newline is stored
    }   // end if read
    else if (rc == 0) {
      if (n == 1)
	return (0);    // EOF, no data read
      else
	break;         // EOF, some data was read
    }   // end else-if rc
    else {
      if (errno == EINTR)
	goto again;
      return 0;          // error, errno set by read()
    }   // end else
  }   // end for

  *ptr = 0;           // null terminate
  return (n);
}   /// end function readline

      
      
