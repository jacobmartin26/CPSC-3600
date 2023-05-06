//By: Jacob Martin and Timmy Lam

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
  int iter;
  char *serverIP = NULL;

  //Default Values
  int count = 0x7fffffff;
  int port = 33333;
  int size = 12;
  int nFlag = 0, SFlag = 0;
  double interval = 1.0;

  while((iter = getopt(argc, argv, "c:i:p:s:nS")) != -1){
	  switch(iter){
		  case 'c':
			  count = (int) strtol(optarg, NULL, 0); // strtol to handle hex or decimal
			  break;
		  case 'i':
			  interval = atof(optarg);
			  break;
		  case 'p':
			  port = atoi(optarg);
			  break;
		  case 's':
			  size = atoi(optarg);
			  break;
		  case 'n':
			  nFlag = 1;
			  break;
		  case 'S':
			  SFlag = 1;
			  break;
		  case '?':
			  break;
	  }
  }
  if(SFlag == 1){
	  // Server mode, so pass proper arguments
	  char servPortStr[32];
	  snprintf(servPortStr, sizeof(servPortStr), "%d", port);
	  execlp("./server.out", "./server.out", servPortStr, NULL);
	  perror("\nFAILED TO EXEC CLIENT, REREAD ANY INSTRUCTIONS PROVIDED\n");
	  exit(EXIT_FAILURE);
  }

  if(optind < argc){
	  serverIP = argv[optind];
  }
  else{
	  fprintf(stderr, "IP required in client mode.\n");
	  exit(EXIT_FAILURE);
  }

  char countStr[32];
  snprintf(countStr, sizeof(countStr), "%d", count);
  char intervalStr[32];
  snprintf(intervalStr, sizeof(intervalStr), "%lf", interval);
  char portStr[32];
  snprintf(portStr, sizeof(portStr), "%d", port);
  char sizeStr[32];
  snprintf(sizeStr, sizeof(sizeStr), "%d", size);
  char nStr[32];
  snprintf(nStr, sizeof(nStr), "%d", nFlag);
  char ipStr[32];
  snprintf(ipStr, sizeof(ipStr), "%s", serverIP);
  execlp("./client.out", "./client.out", countStr, intervalStr, portStr, sizeStr, nStr, ipStr, NULL);
  perror("\nFAILED TO EXEC CLIENT, REREAD ANY INSTRUCTIONS PROVIDED\n");
  exit(EXIT_FAILURE);
}
