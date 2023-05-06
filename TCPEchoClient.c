// By: Jacob Martin and Timmy Lam

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
  if (argc != 4) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)",
        "<Server Address/Name> <Input File> <Server Port/Service>");

  char *server = argv[1];     // First arg: server address/name
  char *inputFile = argv[2];  // Second arg: file to send to server
  char *service = argv[3];    // Third arg: port or service name

  // Open input file for reading
  FILE *input = fopen(inputFile, "r");
  if (input == NULL)
    DieWithSystemMessage("fopen() failed");

  // Create a connected TCP socket
  int sock = SetupTCPClientSocket(server, service);
  if (sock < 0)
    DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");

  // Check for Byte Ordered Mark and skip it
  unsigned char bom[3];
  if(fread(bom, sizeof(unsigned char), 3, input) == 3) {
	  if(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
		  ;
	  } else {
		  rewind(input);
	  }
  }

  printf("Sending data to %s:\n", server);
  // Send the file to the server
  char buffer[BUFSIZE]; // I/O buffer
  int lineCount = 1; // Track the current line number
  char concat[1024];
  while (fgets(buffer, BUFSIZE, input)) {
    // Add the line number to the buffer
    char lineNumber[10];
    sprintf(lineNumber, "%d. ", lineCount++);
    strcpy(concat, lineNumber);
    strcat(concat, buffer);
    printf("%s", concat);

    // Send the buffer to the server
    ssize_t sentBytes = send(sock, concat, strlen(concat), 0);
    if (sentBytes < 0)
      DieWithSystemMessage("send() failed");
    else if (sentBytes != strlen(concat))
      DieWithUserMessage("send()", "sent unexpected number of bytes");
  }
  printf("Goodbye!!!!!\n");
  // Close the input file and the socket
  fclose(input);
  close(sock);
  exit(0);
}
