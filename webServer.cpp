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
void sig_handler(int signo) {
  DEBUG << "Caught signal #" << signo << ENDL;
  DEBUG << "Closing file descriptors 3-31." << ENDL;
  closefrom(3);
  exit(1);
}


// **************************************************************************************
// * processRequest,
//   - Return HTTP code to be sent back
//   - Set filename if appropriate. Filename syntax is valided but existance is not verified.
// **************************************************************************************
int readHeader(int sockFd,std::string &filename) {

  //default return code
  

  
  int bytesread;
  std::string container; //Container for the buffer

  char buffer[BUFFER_SIZE];
  bzero(buffer, BUFFER_SIZE);

  DEBUG << "ProcessConnection bytes being read" << ENDL;

  while ((bytesread = read(sockFd, buffer, BUFFER_SIZE)) > 0){
    DEBUG << "This is container: " << container << ENDL;
    container.append(buffer, bytesread);
    if (container.find("\r\n\r\n") != std::string::npos){
      break;
    }
  }

  
  if (bytesread < 0) {
    std::cout << "Read Failed for sockFd in processConnection()" << strerror(errno) << std::endl;
  }

  //Getting First Line to parse the Get Request using Regex
  size_t pos = container.find("\r\n");
  std::string firstLine = container.substr(0, pos);



  std::smatch match; // smatch is from regex library to store regex_match results
  std::regex getLine("^GET\\s+/(file[0-9]\\.html|image[0-9]\\.jpg)\\s+HTTP/1\\.[01]$");
  
  if (std::regex_match(firstLine, match, getLine)){ //That means that there the request has valid request
    std::istringstream iss(firstLine);
    std::string requestMethod, fileName, httpVersion;
    iss >> requestMethod >> fileName >> httpVersion;
  
    DEBUG << "Testing Regex" << ENDL;
    DEBUG << "Method: " << requestMethod << "| File Name: " << fileName << "| HTTP Version: " << httpVersion << ENDL;

    
    
    //Request is Valid but file might now exist so check file in directory
    if(std::filesystem::exists("data"+fileName)){
      //Set the Filename to the filename from the requestheader
      DEBUG << "File Found in directory" << ENDL;
      filename = "data" + fileName;
      return 200;
    }
    
    
    DEBUG << "File not found in directory" << ENDL;
    return 404;
  }
  //Non valid files
  if (firstLine.rfind("GET", 0) == 0){
    WARNING << "GET request for non valid files" << ENDL;
    return 404;
  }

  //Other unspported methods 
  WARNING << "Invalid or unsupported request method" << ENDL;


  

  
  
  return 400; //Bad Request
}


// **************************************************************************
// * Send one line (including the line terminator <LF><CR>)
// * - Assumes the terminator is not included, so it is appended.
// **************************************************************************
void sendLine(int socketFd, const std::string &stringToSend) {

  size_t length = stringToSend.size();
  char buffer[length + 2];
  memcpy(buffer, stringToSend.c_str(), length); //Exactly copy "length" bytes to buffer
  buffer[length] = '\r';
  buffer[length + 1] = '\n';


  DEBUG << "Writing to Scoket" << ENDL;
  if (write(socketFd, buffer, length+2) == -1){
    ERROR << "Failed to write to socket" << ENDL;
  }
}

// **************************************************************************
// * Send the entire 404 response, header and body.
// **************************************************************************
void send404(int sockFd) {
  DEBUG << "Running send404" << ENDL;
  INFO << "Sending 404 Not Found" <<ENDL;

  
  sendLine(sockFd, "404 Not Found");
  sendLine(sockFd, "Content-Type: text/html");
  sendLine(sockFd, "");
  sendLine(sockFd, "404 Not Found");
  sendLine(sockFd, "The requested file could not be found or is not permitted");

  
}

// **************************************************************************
// * Send the entire 400 response, header and body.
// **************************************************************************
void send400(int sockFd) {
  INFO << "Sending 400 Bad Request" << ENDL;
  sendLine(sockFd, "400 Bad Request");
  sendLine(sockFd, "Content-type: text/html");
  sendLine(sockFd, "");
  sendLine(sockFd, "Bad Request");
  sendLine(sockFd, "The server could not understand the request");
  
}


// **************************************************************************************
// * sendFile
// * -- Send a file back to the browser.
// **************************************************************************************
void sendFile(int sockFd,std::string filename) {
  struct stat fileStat;

  if (stat(filename.c_str(), &fileStat) == -1){
    WARNING << "File not found or no read permission: " << filename << ENDL;
    send404(sockFd);
  }

  size_t fileSize = fileStat.st_size;

  //Find the content-type
  std::string contentType;
  if (filename.find("html") != std::string::npos){
    contentType = "Content-Type: text/html";
  }
  else if (filename.find("jpg") != std::string::npos){
    contentType = "Content-Type: image/jpg";
  }

  INFO << "Sending 200 OK for " << filename << " | file size: " << fileSize << " bytes" << ENDL;

  sendLine(sockFd, "HTTP/1.0 200 OK");
  sendLine(sockFd, "Content-Length: " + std::to_string(fileSize));
  sendLine(sockFd, contentType);
  sendLine(sockFd, ""); //End header


  int fileFd = open(filename.c_str(), O_RDONLY);
  if (fileFd == -1){
    ERROR << "Failed to open file " << filename << ENDL;
    return;
  }

  //Create Buffer to send back;
  char buffer[10];
  ssize_t bytesRead;

  while ((bytesRead = read(fileFd, buffer, sizeof(buffer))) > 0){
    if (write(sockFd, buffer, sizeof(buffer)) < 0){
      ERROR << "Failed to write to socket" << ENDL;
      break;
    }
  }

  close(fileFd);
  
}


// **************************************************************************************
// * processConnection
// * -- process one connection/request.
// **************************************************************************************
int processConnection(int sockFd) {
 
  // Call readHeader()
  std::string filename;
  int responseCode = readHeader(sockFd, filename);

  // If read header returned 400, send 400

  // If read header returned 404, call send404

  // 471: If read header returned 200, call sendFile
  
  // 598 students
  // - If the header was valid and the method was GET, call sendFile()
  // - If the header was valid and the method was HEAD, call a function to send back the header.
  // - If the header was valid and the method was POST, call a function to save the file to dis.

  if (responseCode == 400){
    send400(sockFd);
  }
  else if (responseCode == 404){
    send404(sockFd);
  }
  else if (responseCode == 200){
    DEBUG << "Sending File: " << filename << "| Reponse Code: 200" << ENDL;
    sendFile(sockFd, filename);
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
  
  signal(SIGINT,sig_handler);

  
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

  while(1){
    servaddr.sin_port = htons(port);

    DEBUG << "Calling Bind on port: " << port << ENDL;
    if (bind(listenFd, (sockaddr*) &servaddr, sizeof(servaddr)) < 0){
      std::cout << "bind() failed " << strerror(errno) << std::endl;
      if (errno == EADDRINUSE){
        port++;
      }else{
        exit(-1);
      }
    }else{
      break;
    }
  }
  
  


  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // *
  // * Don't forget to check to see if bind() fails because the port
  // * you picked is in use, and if the port is in use, pick a different one.
  // ********************************************************************
  
 


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
