/*
  Author: Chad Holst
  Course: CPSC441
  Instructor: Carey Williamson
  Date submitted: Feb 18, 2022

  The following sources assisted the development of this code and were supplied by the instructor:
  
  https://pages.cpsc.ucalgary.ca/~carey/CPSC441/examples/wordlen-client-UDP.c (UDP client setup)
  https://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass0/mypal-server.c (two if statements to check for /n and /r for accurate byte count)
  https://uofc-my.sharepoint.com/personal/william_black_ucalgary_ca/_layouts/15/onedrive.aspx?id=%2Fpersonal%2Fwilliam%5Fblack%5Fucalgary%5Fca%2FDocuments%2FCPSC441%2FW2022 (UDP timeout)

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // close
#include <netinet/in.h>
#include <signal.h>
#include <string.h> // memset
#include <sys/socket.h> // socket, bind, accept, listen, connect
#include <arpa/inet.h> // struct sockaddr_in, htons, inet_addr
#include <sys/types.h>
#include <errno.h>

#define SIZE 2000

// configure server IP
// #define SERVER_IP "136.159.5.25"  // csx1.cpsc.ucalgary.ca 
 #define SERVER_IP "136.159.5.27"  // csx3.cpsc.ucalgary.ca 
// #define SERVER_IP "127.0.0.1"   // loopback interface 

// global variable
int error_num; // used to define which error message was recieved
fd_set fds;
int server_fd; 
int choice = 0;
int TCP_UDP_port;
char sendMessage[SIZE];
char recMessage[SIZE];
char sendVowels[SIZE];
char sendOthers[SIZE];
char storedOthers[SIZE];
char storedVowels[SIZE];
char selection[1];

// This is a signal handler using SIGINT to close sockets
void closeSockets(int sig)
{
	close(server_fd);
	exit(0);
}

// menu for function options
void printMenu()
{
  printf("\nPlease select one of the following options:\n");
  printf(" (2) Devowel the text you have entered\n");
  printf(" (1) Envowel the text you have entered\n");
  printf(" (0) Exit program\n");
  printf("Your selection is: ");
}

int main(int argc, char *argv[])
{
    if(argc < 2) // check arg is 1
    {
        printf("Error: please provide a port number.\n");
        return 1;
    }

    // miscellaneous set up
    TCP_UDP_port = atoi(argv[1]); // command line arg: port number 
    static struct sigaction action; // for signal
    struct timeval tcp_tv; // set timeout for TCP with timeval structure for setsockopt()
    tcp_tv.tv_sec = 5; // 5 second timeout value
    struct timeval udp_tv; // set 5 second timeout for UDP with timeval structure for setsockopt()
    udp_tv.tv_sec = 5; // 5 second timeout value

    // set up a signal handler for closing sockets using CTRL + C
    action.sa_handler = closeSockets;
    action.sa_flags = 0;
    sigemptyset(&(action.sa_mask));
    if(sigaction(SIGINT, &action, NULL) == -1)
    {
      printf("sigaction() has failed.\n");
    }

    // TCP address initializtion 
    struct sockaddr_in socketAddress; // must connect socket to server which requires ip address and port number
    memset(&socketAddress, 0, sizeof(socketAddress)); // initialize the address to 0
    socketAddress.sin_family = AF_INET; // set IPv4 protocol as communication domain (internet protocol version 4)
    socketAddress.sin_port = htons(TCP_UDP_port); // convert unsigned integer from host byte order to network byte order
    socketAddress.sin_addr.s_addr = inet_addr(SERVER_IP); // sin_addr is in_addr variable and s_addr is a member that is long (convert IP to long)

    // TCP socket creation: create an endpoint for sending/recieving data across the network
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket with IPv4 on SOCK_STREAM which is a reliable connection-based protocol
    if(tcp_sock == -1)
    {
      printf("TCP socket() failed.\n"); 
      return 1;
    }
    
    // connection request: pass the fd which identifies the socket and the address of target server
    server_fd = connect(tcp_sock, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    if(server_fd == -1)
    {
      printf("connect() failed.\n"); 
      return 1;
    }

    if((setsockopt(tcp_sock, SOL_SOCKET, SO_RCVTIMEO, &tcp_tv, sizeof(tcp_tv)) == -1)) // set 5 second timeout for recv()
    {
      printf("setting timeout for TCP socket has failed.\n");
    }

    printf("Connected to TCP server.\n");
    // ---------------------------------------- TCP setup done ---------------------------------------------- 
    
    // UDP setup and address initialization
    struct sockaddr_in udp_server;
    struct sockaddr *server;
    int len;
    memset((char *) &udp_server, 0, sizeof(udp_server)); // set values to 0
    udp_server.sin_family = AF_INET; // set IPv4 protocol as communication domain (internet protocol version 4)
    udp_server.sin_port = htons(TCP_UDP_port); // convert unsigned integer from host byte order to network byte order
    server = (struct sockaddr *) &udp_server;
    len = sizeof(udp_server);

    // Creating UDP socket file descriptor
    int udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // create UDP socket with IPv4 on SOCK_DGRAM which is a unreliable and connectionless datagram-based protocol
    if(udp_sock == -1) 
    {
      printf("UDP socket() failed"); 
      return 1;
    }
    
    // convert character string into address family structure
    if(inet_pton(AF_INET, SERVER_IP, &udp_server.sin_addr) == 0)
    {
      printf("inet_pton() failed\n");
      return -1;
    }

    if((setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &udp_tv, sizeof(udp_tv)) == -1)) // set 5 second timeout for recvfrom()
    {
      printf("setting recvfrom() timeout for UDP socket has failed.\n");
    }
    
    if((setsockopt(udp_sock, SOL_SOCKET, SO_SNDTIMEO, &udp_tv, sizeof(udp_tv)) == -1)) // set 5 second timeout for sendto()
    {
      printf("setting sendto() timeout for UDP socket has failed.\n");
    }

    for( ; ; ) // infinite loop while client communicates with server
    {
      choice = 0;
      printMenu();
      bzero(sendMessage, SIZE); // clear arrays each iteration
      bzero(recMessage, SIZE);
      bzero(storedVowels, SIZE);
      bzero(storedOthers, SIZE);
      bzero(selection, 1);
      scanf("%d%*c", &choice); // selection of function to be sent to server
      selection[0] = choice;
      send(tcp_sock, selection, 1, 0);
      switch(choice) // loop through each client choice
      {
        case 2: // devowel the single input message
        {
          int other_bytes, vowel_bytes;
          printf("Please enter the message to devowel:\n"); // enter original message
          scanf("%[^\n]s", &sendMessage);
          int sentTCP = send(tcp_sock, sendMessage, SIZE, 0);
          if(sentTCP == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK)
            {
              printf("TCP timeout has been reached and no data has been recieved.\n");
            }
            else
            {
              printf("send() has failed due to the following error number: %d\n", error_num);
            }
            udp_tv.tv_sec = 5; // reset timer
          } 
          bzero(sendMessage, SIZE);

          other_bytes = recv(tcp_sock, storedOthers, SIZE, 0); // recieve other characters from server
          if(other_bytes == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK) // either error message can occurs when timeout is exceeded
            {
              printf("TCP timeout has been reached and no data has been recieved.\n");
            }
            else
            {
              printf("recv() has failed due to the following error number: %d\n", error_num);
            }
            tcp_tv.tv_sec = 5; // reset timer
          }
          else if(other_bytes >= 0)
          {
            printf("\nClient recieved %d bytes of non-vowels through TCP with the following message:\n%s\n", other_bytes, storedOthers);
          }
          
          int sentUDP;
          const char* message = "";
          sentUDP = sendto(udp_sock, message, strlen(message), 0, server, len); // "wake up" UDP communication
          if(sentUDP == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK) // error messages for when timeout is reached
            {
              printf("UDP sendto() timeout has been reached and packets have been lost. Please try again.\n");
            }
            else
            {
              printf("UDP sendto() has failed and the error number is: %i\n", error_num);
            }
            udp_tv.tv_sec = 5; // reset timer
          }

          vowel_bytes = recvfrom(udp_sock, storedVowels, SIZE, 0, server, (socklen_t*)&len); // recieve vowels from server
          if(vowel_bytes == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK) // error messages for when timeout is reached
            {
              printf("UDP recvfrom() timeout has been reached and packets have been lost. Please try again.\n");
            }
            else
            {
              printf("UDP recvfrom() has failed and the error number is: %i\n", error_num);
            }
            udp_tv.tv_sec = 5; // reset timer
          }
          else if(vowel_bytes >= 0)
          {
            printf("\nClient recieved %d bytes of vowels through UDP with the following message:\n%s\n", vowel_bytes, storedVowels);
          }
          break;
        }
        case 1: // envowel two input text messages
        {
          int other_bytes, vowel_bytes;
          printf("Please enter the non-vowel part of the message to envowel:\n");
          scanf("%[^\n]%*c", &sendOthers);

          printf("Please enter the vowel part of the message to envowel:\n");
          scanf("%[^\n]%*c", &sendVowels);

          other_bytes = send(tcp_sock, sendOthers, strlen(sendOthers), 0);
          if(other_bytes == -1)
          {
            error_num = errno;
            printf("send() has failed and the error number is: %i\n", error_num);
          }
          else
          {
            printf("Client sent %d bytes of non-vowels through TCP with the following message:\n%s\n", other_bytes, sendOthers);
          }
          bzero(sendOthers, SIZE);
          
          vowel_bytes = sendto(udp_sock, sendVowels, strlen(sendVowels), 0, server, len);
          if(vowel_bytes == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK)
            {
              printf("UDP sendto() timeout has been reached and packets have been lost. Please try again.\n");
            }
            else
            {
              printf("UDP sendto() has failed and the error number is: %i\n", error_num);
            }
            udp_tv.tv_sec = 5; // reset timer
          }
          else
          {
            printf("\nClient sent %d bytes of vowels through UDP with the following message:\n%s\n", vowel_bytes, sendVowels);
          }
          bzero(sendVowels, SIZE);

          int tcp_bytes;
          while(tcp_bytes = recv(tcp_sock, recMessage, SIZE, 0) > 0)
          {
            // check newline and carriage return twice for accurate byte count (mypal-server.c)
            if((tcp_bytes > 0) && ((recMessage[tcp_bytes-1] == '\n') || (recMessage[tcp_bytes-1] == '\r')) )
            {
              recMessage[tcp_bytes-1] = '\0';
              tcp_bytes--;
            }
            if((tcp_bytes > 0) && ((recMessage[tcp_bytes-1] == '\n') || (recMessage[tcp_bytes-1] == '\r')) )
            {
              recMessage[tcp_bytes-1] = '\0';
              tcp_bytes--;
            }
            int rec_len = strlen(recMessage);
            fprintf(stderr, "\nClient recieved %i bytes through TCP and the merged message is:\n%s\n", rec_len, recMessage);
          }
          if(tcp_bytes == -1)
          {
            error_num = errno;
            if(error_num == EAGAIN || error_num == EWOULDBLOCK)
            {
              printf("TCP timeout has been reached and no data has been recieved.\n");
            }
            else
            {
              printf("recv() has failed due to the following error number: %d\n", error_num);
            }
            tcp_tv.tv_sec = 5; // reset timer
          }
          break;
        }
        case 0: // exit program
        {
          printf("The client has exited. Goodbye server.\n");
          close(udp_sock); // close UDP socket
          close(tcp_sock); // close TCP socket
          exit(0);
          break;
        }
      }
    }
}