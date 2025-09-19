// **************************************************************************************
// * webServer (webServer.cpp)
// * - Implements a very limited subset of HTTP/1.0, use -v to enable verbose debugging output.
// * - Port number 1701 is the default, if in use random number is selected.
// *
// * - GET requests are processed, all other metods result in 400.
// *     All header gracefully ignored
// *     Files will only be served from cwd and must have format file\d.html or image\d.jpg
// *
// * - Response to a valid get for a legal filename
// *     status line (i.e., response method)
// *     Cotent-Length:
// *     Content-Type:
// *     \r\n
// *     requested file.
// *
// * - Response to a GET that contains a filename that does not exist or is not allowed
// *     statu line w/code 404 (not found)
// *
// * - CSCI 471 - All other requests return 400
// * - CSCI 598 - HEAD and POST must also be processed.
// *
// * - Program is terminated with SIGINT (ctrl-C)
// **************************************************************************************
#include "webServer.h"


// **************************************************************************************
// * Signal Handler.
// * - Display the signal and exit (returning 0 to OS indicating normal shutdown)
// * - Optional for 471, required for 598
// **************************************************************************************
// void sig_handler(int signo) {}


// **************************************************************************************
// * processRequest,
//   - Return HTTP code to be sent back
//   - Set filename if appropriate. Filename syntax is valided but existance is not verified.
// **************************************************************************************
int readHeader(int sockFd,std::string &filename) {
  return 0;
}


// **************************************************************************
// * Send one line (including the line terminator <LF><CR>)
// * - Assumes the terminator is not included, so it is appended.
// **************************************************************************
void sendLine(int socketFd, std::string &stringToSend) {
  return;
}

// **************************************************************************
// * Send the entire 404 response, header and body.
// **************************************************************************
void send404(int sockFd) {
  return;
}

// **************************************************************************
// * Send the entire 400 response, header and body.
// **************************************************************************
void send400(int sockFd) {
  return;
}


// **************************************************************************************
// * sendFile
// * -- Send a file back to the browser.
// **************************************************************************************
void sesendFile(int sockFd,std::string filename) {
  return;
}


// **************************************************************************************
// * processConnection
// * -- process one connection/request.
// **************************************************************************************
int processConnection(int sockFd) {
 
  // Call readHeader()

  // If read header returned 400, send 400

  // If read header returned 404, call send404

  // 471: If read header returned 200, call sendFile
  
  // 598 students
  // - If the header was valid and the method was GET, call sendFile()
  // - If the header was valid and the method was HEAD, call a function to send back the header.
  // - If the header was valid and the method was POST, call a function to save the file to dis.

  #define BUFFER_SIZE 1024
  int bytesread;

  char buffer[BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);

  bytesread = read(sockFd, buffer, BUFFER_SIZE);

  if (bytesread < 1) {
    std::cout << "Read Failed for sockFd in processConnection()" << strerror(errno) << std::endl;
  }



  return 0;
}
    

int main (int argc, char *argv[]) {


  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
 
  int opt = 0;
  while ((opt = getopt(argc,argv,"d:")) != -1) {
    
    switch (opt) {  
    case 'd':
      LOG_LEVEL = std::stoi(optarg);
      break;
    case ':':
    case '?':
    default:
      std::cout << "useage: " << argv[0] << " -d LOG_LEVEL" << std::endl;
      exit(-1);
    }
  }


  // *******************************************************************
  // * Catch all possible signals
  // ********************************************************************
  DEBUG << "Setting up signal handlers" << ENDL;
  

  
  // *******************************************************************
  // * Creating the inital socket using the socket() call.
  // ********************************************************************

  int listenFd;
  listenFd = socket(AF_INET, SOCK_STREAM, 0);
  DEBUG << "Calling Socket() assigned file descriptor " << listenFd << ENDL;

  


  if ((listenFd < 0)){
    std::cout<<"Error creating socket" << strerror(errno) << std::endl;
    exit(-1);
  }

    
    uint16_t port = 1701;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

  
  // ********************************************************************
  // * The bind() call takes a structure used to spefiy the details of the connection. 
  // *
  // * struct sockaddr_in servaddr;
  // *
  // On a cient it contains the address of the server to connect to. 
  // On the server it specifies which IP address and port to lisen for connections.
  // If you want to listen for connections on any IP address you use the
  // address INADDR_ANY
  // ********************************************************************

  if (bind(listenFd, (sockaddr*) &servaddr, sizeof(servaddr)) < 0){
    std::cout << "bind() failed " << strerror(errno) << std::endl;
    exit(-1);
  }


  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // *
  // * Don't forget to check to see if bind() fails because the port
  // * you picked is in use, and if the port is in use, pick a different one.
  // ********************************************************************
  
  DEBUG << "Calling bind()" << ENDL;
  
  std::cout << "Using port: " << port << std::endl;


  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a que for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  DEBUG << "Calling listen()" << ENDL;
  int listeng = 1;
  if (listen(listenFd, listeng) < 0){
    std::cout << "Listen() failed" << strerror(errno) << std::endl;
    exit(-1);
  }


  // ********************************************************************
  // * The accept call will sleep, waiting for a connection.  When 
  // * a connection request comes in the accept() call creates a NEW
  // * socket with a new fd that will be used for the communication.
  // ********************************************************************
  int quitProgram = 0;
  while (!quitProgram) {
    int connFd = 0;
    DEBUG << "Calling connFd = accept(fd,NULL,NULL)." << ENDL;
    if ((connFd = accept(listenFd, (sockaddr *) NULL, NULL)) < 0){
        std::cout << "accept() failed" << strerror(errno) << std::endl;
        exit(-1);
    }
    

    DEBUG << "We have recieved a connection on " << connFd << ". Calling processConnection(" << connFd << ")" << ENDL;
    quitProgram = processConnection(connFd);
    DEBUG << "processConnection returned " << quitProgram << " (should always be 0)" << ENDL;
    DEBUG << "Closing file descriptor " << connFd << ENDL;
    close(connFd);
  }
  

  ERROR << "Program fell through to the end of main. A listening socket may have closed unexpectadly." << ENDL;
  closefrom(3);

}
