//By: Jacob Martin and Timmy Lam

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include "Practical.h"

#define realTime CLOCK_REALTIME

sig_atomic_t stopFlag = 0;

void sig_handler(int signo){
	stopFlag = 1;
}

int main(int argc, char *argv[]) {
    //Args
    int count, size, nFlag;
    double interval;
    char *servPort = argv[3];
    char *server = argv[6]; // Server address/name

    //Parse
    count = atoi(argv[1]);
    interval = atof(argv[2]);
    size = atoi(argv[4]);
    nFlag = atoi(argv[5]);

    // If a parameter changed from default, display it
    fprintf(stderr, "\n");
    if(count != 0x7fffffff)
    {
    	fprintf(stderr, "%-10s  %10d\n", "Count", count);
    }
    if(size != 12)
    {
    	fprintf(stderr, "%-10s  %10d\n", "Size", size);
    }
    if(interval != 1.0)
    {
    	fprintf(stderr, "%-10s  %10.3lf\n", "Interval", interval);
    }
    if(strcmp(servPort, "33333") != 0)
    {
    	fprintf(stderr, "%-10s  %10s\n", "Port", servPort);
    }
    // Always display IP
    fprintf(stderr, "%-10s  %10s\n\n", "Server IP", server);

  // Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  // For the following fields, a zero value means "don't care"
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Get address(es)
  struct addrinfo *servAddr; // List of server addresses
  int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
  if (rtnVal != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rtnVal));

  // Create a datagram/UDP socket
  int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
      servAddr->ai_protocol); // Socket descriptor for client
  if (sock < 0)
    DieWithSystemMessage("socket() failed");

  struct timespec start, finish;
  double rtt, min, max, avg, sum;
  min = 100000000.0;
  max = 0.0;
  avg = 0.0;
  int sent = 0;
  int rec = 0;
  int tempFlag = 1;
  int j;
  signal(SIGINT,sig_handler);
  double prev, prevMin, prevMax;
for(j = 1; j <= count; j++)
{
  if(stopFlag == 1)
  {
	  // Signal triggered, set everything back one
	  sent--;
	  rec--;
	  j--;
	  sum = prev;
	  min = prevMin;
	  max = prevMax;
	  break;
  }
  else
  {
  // Create packet
  char *packet = malloc(size);
  if(!packet)
  {
	  perror("malloc failed");
	  exit(EXIT_FAILURE);
  }

  clock_gettime(realTime, &start); // Get start time
  sleep(interval);

  // Send packet
  int bytesSent = sendto(sock, packet, size, 0, 
      servAddr->ai_addr, servAddr->ai_addrlen);
  if(bytesSent < 0)
  {
	  perror("sendto() failed");
  }
  else
  {
	  sent++; // Packet sent successfully
  }

  struct sockaddr_storage fromAddr; // Source address of server
  // Set length of from address structure (in-out parameter)
  socklen_t fromAddrLen = sizeof(fromAddr);
  
  char *recvPacket = malloc(size);

  // Receive a response
  ssize_t numBytes = recvfrom(sock, recvPacket, size, 0, 
	   (struct sockaddr *) &fromAddr, &fromAddrLen);
  clock_gettime(realTime, &finish); // Get end time of packet
  if(numBytes < 0)
  {
	  perror("recvfrom() failed");
  }
  else
  {
	  rec++; // Packet received successfully
  }

  // Verify reception from expected source
  if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr *) &fromAddr))
    DieWithUserMessage("recvfrom()", "received a packet from unknown source");

  // Calculate rtt
  rtt = (finish.tv_sec - start.tv_sec) * 1000.0;
  rtt += (finish.tv_nsec - start.tv_nsec) / 1000000.0;
  if(interval >= 1.0)
  {
  	rtt = rtt - (interval * 1000.0) - 0.3;
  }
  
  // Determine print mode
  if(nFlag == 1 && tempFlag == 1)
  {
	  printf("**********\n");
	  tempFlag = 0;
  }
  else if(nFlag == 0 && tempFlag == 1)
  {
    if(stopFlag == 0){
  	printf("%3d    %3d    %3.3f\n", j, size, rtt);
    }
    else
    {
	    // Signal triggered, print nothing
	    printf("\r");
    }
  }
  
  // Calculate sum of rtt's
  prev = sum;
  sum += rtt;
  prevMin = min;
  if(rtt < min)
  {
	  min = rtt;
  }
  prevMax = max;
  if(rtt > max)
  {
	  max = rtt;
  }

  }
}
  // Calculate packet loss, average rtt, and format sum
  int packetLoss = ((sent - rec) / sent) * 100;

  if(j != count)
  {
	j--;
  }
  if(stopFlag == 1 && j == count)
  {
	j--;
  }	  
  avg = sum / (double)j;
  sum = sum * 1000.0;

  // Print results
  printf("\n%d packets transmitted, %d received, %d%% packet loss, time %d ms\n", 
	                                        sent, rec, packetLoss, (int)sum);
  printf("rtt min/avg/max = %.3f/%.3f/%.3f msec\n\n", min, avg, max);
  
  // Clean up
  freeaddrinfo(servAddr);
  close(sock);
  
  exit(0);
}
