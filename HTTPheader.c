/*-------------------------------------------------------------------------------------
 name: sendHTTPheader
 parameters:
     descriptor - where to write data
     type - type of file requested
 return value: none

 description:
   This function is used to send the HTTP header to the specified stream. The 
   header code sent depends on the type of the file requested or if an error
   message should be returned. This function will write to the specified
   stream the initial line, necessary headers and the blank line.  

   An HTTP response consists of the following:
        Initial Line
        Headers
        <<blank line>>
        Body of message (contents of file requested) 

   The type parameter determines the contents of the initial line and headers.
   This function uses an integer to determine the type, but it can be modified
   to accept a file extension or other parameter.

   Type parameter:
      1 - html file requested and was found
      2 - jpg file requested and was found
      3 - file type unknown and was found
      4 - file not found

---------------------------------------------------------------------------------*/
void sendHTTPheader(int desc, int type) {
  switch (type) {
  case 1:
    writen(desc, "HTTP/1.0 200 OK\r\n", 17);
    writen(desc, "Content-Type: text/html\r\n", 25);
    break;
  case 2:
    writen(desc, "HTTP/1.0 200 OK\r\n", 17);
    writen(desc, "Content-Type: image/jpeg\r\n", 26);
    break;
  case 3:
    writen(desc, "HTTP/1.0 200 OK\r\n", 17);
    writen(desc, "Content-Type: application/octet-stream\r\n", 40);
    break; 
  case 4:
    writen(desc, "HTTP/1.0 404 Not Found\r\n", 24);
    writen(desc, "Content-Type: text/html\r\n", 31);
    break;
  default:
    writen(desc, "HTTP/1.0 400 Bad Request\r\n", 23);
    writen(desc, "Content-Type: text/html\r\n", 25);
  }  // end switch

  // send the blank line separating header from body
  writen(desc, "\r\n", 2);

  return;    
}  // end function sendHTTPheader

