// By: Jacob Martin and Timmy Lam

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "Practical.h"

static const int MAXPENDING = 5; // Maximum outstanding connection requests

int SetupTCPServerSocket(const char *service) {
  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  int servSock = -1;
  for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    if (servSock < 0)
      continue;       // Socket creation failed; try next address

    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);
      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");
      fputs("Binding to ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);
      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  return servSock;
}

int AcceptTCPConnection(int servSock) {
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  // clntSock is connected to a client!

  fputs("Handling client ", stdout);
  PrintSocketAddress((struct sockaddr *) &clntAddr, stdout);
  fputc('\n', stdout);

  return clntSock;
}

void HandleTCPClient(int clntSocket) {
 char lines[1000][1000];
 int numLines = 0;
 char buffer[BUFSIZE]; // Buffer for receiving client data

 struct sockaddr_storage clientAddress;
 socklen_t addressLength = sizeof(clientAddress); // Get length of address

 // Get address info
 if(getpeername(clntSocket, (struct sockaddr *) &clientAddress, &addressLength) < 0)
		 DieWithSystemMessage("getpeername() failed");

 // Print address
  char name[INET_ADDRSTRLEN];
  if(inet_ntop(AF_INET, &((struct sockaddr_in *) &clientAddress)->sin_addr, name, sizeof(name)) == NULL)
  	DieWithSystemMessage("inet_ntop() failed");
  printf("Received data from %s:\n", name);

  ssize_t numBytesRcvd;
  // Receive the file contents line by line
  while ((numBytesRcvd = recv(clntSocket, buffer, sizeof(buffer), 0)) > 0) {
	  // Check for complete lines of text
	  char *endOfLine = buffer;
	  char *nextNewLine;
	  while((nextNewLine = strchr(endOfLine, '\n')) != NULL){
		  int lineLen = nextNewLine - endOfLine;
		  if(lineLen > 0){
			  // Copy to new buffer to be reversed
			  memcpy(lines[numLines], endOfLine, lineLen);
			  lines[numLines][lineLen] = '\0';
			  numLines++;
		  }
		  endOfLine = nextNewLine + 1;
	  }
  }
  // Remove newline from end of line
  for(int i = 0; i < numLines; i++){
	  int lineLength = strlen(lines[i]);
	  if(lineLength > 0 && lines[i][lineLength - 1] == '\n'){
		  lines[i][lineLength - 1] = '\0';
	  }
  }
  for(int i = 0, j = numLines - 1; i < j; i++, j--){
	  // Reverse line order
	  char temp[1000];
	  strcpy(temp, lines[i]);
	  strcpy(lines[i], lines[j]);
	  strcpy(lines[j], temp);
  }
	  // reverse character order
	  for(int i = 0; i < numLines; i++){
		  char *start = strchr(lines[i], ' ') + 1;
		  int len = strlen(start);
		  int k;
		  for(k = 0; k < len / 2; k++){
			  char temp2 = start[k];
			  start[k] = start[len-1-k];
			  start[len-1-k] = temp2;
		  }
	  }
  for(int i = 1; i < numLines; i++){
	  printf("%s\n", lines[i]);
  }
    // Send the reversed line back to the client
    if (send(clntSocket, buffer, numBytesRcvd, 0) != numBytesRcvd)
     DieWithSystemMessage("send() failed");
  printf("Goodbye!\n");
  // Close client socket
  close(clntSocket);
}
