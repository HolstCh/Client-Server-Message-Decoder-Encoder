/*
  Author: Chad Holst
  Course: CPSC441
  Instructor: Carey Williamson
  Date submitted: Feb 18, 2022

  The following sources assisted the development of this code and were supplied by the instructor:
  
  https://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass0/mypal-server.c (two if statements to check for /n and /r for accurate byte count)
  https://pages.cpsc.ucalgary.ca/~carey/CPSC441/examples/wordlen-server-UDP.c (UDP server setup)

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <string.h> // memset
#include <sys/socket.h> // socket, bind, accept, listen, connect
#include <arpa/inet.h> // struct sockaddr_in, htons, inet_addr
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>

// size of buffers and string technique directives
#define SIZE 2000
#define SIMPLE 0 // 1 to use simple string functionality, 0 to not use
#define ADVANCED 1 // 1 to use advanced string functionality, 0 to not use

// global variables
int error_num; // used to define which error message was recieved
int child_fd; 
int parent_fd;
int TCP_UDP_port;
char storedVowels[SIZE];
char storedOthers[SIZE];
char sendMessage[SIZE];
char recMessage[SIZE];
char choice[1];

// signal handler set up using SIGINT for closing server sockets after client is done
void closeSockets(int sig)
{
  fprintf(stderr, "\nThe server has exited. Goodbye world.\n");
  close(parent_fd); // close parent socket fd
  close(child_fd); // close child socket fd
  exit(0);
}

// check if char in array is a vowel or not
bool isVowel(char* vowel) 
{
  if(*vowel == 'a' || *vowel == 'A' || *vowel == 'e' || *vowel == 'E' || *vowel == 'i' ||
      *vowel == 'I' || *vowel =='o' || *vowel =='O' || *vowel == 'u' || *vowel == 'U')
  {
      return true;
  }
  else
  {
      return false;
  }
}

#if ADVANCED
// takes each vowel from rightmost array and then places into vowels array to encode into integer/vowel format
// vowel in rightmost array is replaced with a space as array is for non-vowels where new vowel spaces will be removed by removeSpaces()
void devowelAdvanced(char* vowels, char* vowel) 
{
  int i = 0;
  int j = 0;
  while(*vowel != '\0') // iterate until null terminator
  {
    if(isVowel(vowel))
    {
      vowels[i++] = j + '0'; // place integer into integer/vowel format array
      vowels[i++] = *vowel; // place vowel into integer/vowel format array
      *vowel = ' '; // replace vowel as space in non-vowel array
      j = 0; // reset counter
    }
    else
    {
      j++; // count character that has been skipped
    }
    vowel++; // move to next position in array
  }
  *vowel = '\0'; // ensure null terminator is last (insurance)
}

// mark spaces in char array as $ so those specific spaces are not removed with removeSpaces()
void markSpaces(char* others) 
{
  while(*others != '\0') // iterate until null terminator
  {
    if(isspace(*others))
    {
      *others = '$'; // replace single space with dollar sign as a placeholder
    }
    others++; // move to next position in array
  }
  *others = '\0'; // ensure null terminator is last (insurance)
}

// removes unmarked spaces in order to compress others array through skipping spaces where vowels used to be with another iterator
// original spaces are preserved with $ as a placeholder to assist in decoding/merging
void removeSpaces(char* others)
{
  char* iterator = others; // point to same array and skip ahead if spaces present
  while(*iterator != '\0') // iterate until null terminator
  {
    if(isspace(*iterator) == false) // character in same array is not a space
    {
      *others = *iterator;  // place non-space in array 
      others++; // move non-space pointer to next position
    }
    iterator++; // move iterator to next position
  }
  *others = '\0'; // ensure null terminator is last (required)
}

// removes markers that are placeholders for wanted spaces (spaces before removing vowels)
void demarkSpaces(char* others)
{
  while(*others != '\0') // iterate until null terminator
  {
    if(*others == '$')
    {
        *others = ' '; // replace $ with single space
    }
    others++; // move to next position in array
  }
  *others = '\0'; // ensure null terminator is last (insurance)
}
// take integer/vowel format in vowels array and compressed others format in others and decode into merged array
void envowelAdvanced(char* merged, char* vowels, char* others)
{
  size_t numbVowels = (strlen(vowels)/2); // number of vowels and not integers in int/vowel format
  size_t numbOthers = strlen(others); // number of characters of non-vowels
  size_t total = numbVowels + numbOthers; // total size of new merged array

  int i = 0; // count characters in both arrays
  int k = 0; // index for new merged array where both arrays place characters into
  while(i < total) // iterate until expected merged array size
  {
    int j = 0;
    int nextVowel = *vowels - '0'; // convert char number of skipped characters to integer
    while(j < nextVowel) // iterate until integer number of skipped characters
    {
      merged[k++] = *others; // place other characters into merged array and move to next position
      others++; // move to next position in others array
      i++; // count total characters so far
      j++; // count until vowel position to place in merged
    }
    if(*vowels == '\0') // if no more vowels 
    {
      while(*others != '\0') // place the rest of the other characters into new merged array
      {
        merged[k++] = *others;
        others++;
      }
    }
    vowels++; // skip integer to next vowel position
    merged[k++] = *vowels; // place vowel into new merged array and move to next position
    vowels++; // move to next integer position
    i++; // count total characters so far
  }
  merged[k] = '\0'; // ensure null terminator is last (required)
}
#endif

#if SIMPLE
// takes each vowel from rightmost array and then places into vowels array to encode into space/vowel format
// vowel in rightmost array is replaced with a space
void devowelSimple(char* vowels, char* vowel)
{
  int i = 0; // index for vowels array
  while(*vowel != '\0') // iterate until null terminator
  {
    if(isVowel(vowel))
    {
      vowels[i] = *vowel; // place vowel in vowels array
      *vowel = ' '; // replace vowel with space in other array
    }
    else
    {
      vowels[i] = ' '; // ensure there is a single space for non-vowels in other array
    }
    i++;
    vowel++; // iterate other array
  }
  *vowel == '\0'; // ensure null terminator is last (insurance)
}
// take space/vowel format in vowels array and space/others format in others and decode into merged array
void envowelSimple(char* merged, char* vowels, char* others)
{
  int i = 0; // merged index for vowels
  int j = 0; // merged index for others
  while(vowels != "\0" && others != "\0") // iterate until null terminators are reached in both arrays
  {
    if(isspace(*vowels) && isspace(*others)) // if both arrays contain a space
    {
      merged[i] = *vowels; // include space that isn't vowel or consonant
      vowels++; // move vowels position
      i++; // move merged vowels index
      others++; // move others position
      j++; // move merged others index
    }
    else
    {
      if(isalpha(*vowels) && (isspace(*others) || *others == '\0')) // if correct vowels position
      {
        merged[i] = *vowels; // place vowel in merged
      }
      else if((isalnum(*others) || ispunct(*others)) && (isspace(*vowels) || *vowels == '\0')) // if correct others position
      {
        merged[j] = *others; // place other characters in merged
      }

      if(*vowels != '\0') // move vowels position and merged vowels index
      {
        vowels++;
        i++;
      }
      if(*others != '\0') // move others position and merged others index
      { 
        others++;
        j++;
      }  
      if(*vowels == '\0' && *others == '\0') // all characters have been placed in merged
      {
        break;
      } 
    } 
  }
  *vowels = '\0'; // ensure null terminators are last
  *others = '\0';
}
#endif

int main(int argc, char *argv[])  
{
  if(argc < 2) // check arg is 1
  {
    printf("Error: please provide a port number.\n");
    return 1;
  }
  
  TCP_UDP_port = atoi(argv[1]); // command line arg: port number 
  static struct sigaction action; // signal action
  int struct_size; 
  struct sockaddr_in address; // internet socket address
  struct timeval tv; // set timeout with struct for TCP and UDP
  tv.tv_sec = 5;
  
  // set up a signal handler for closing sockets using CTRL + C
  action.sa_handler = closeSockets;
  action.sa_flags = 0;
  sigemptyset(&(action.sa_mask));
  if(sigaction(SIGINT, &action, NULL) == -1)
  {
    printf("sigaction() has failed.\n");
  }
  
  // initialize TCP address
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(TCP_UDP_port); // takes port from command line
  address.sin_addr.s_addr = htonl(INADDR_ANY); // server will connect to any IP address

  // create parent socket fd
  parent_fd = socket(PF_INET, SOCK_STREAM, 0);
  if(parent_fd == -1)
  {
    printf("socket() call failed!\n");
    exit(1);
  }

  // bind address to socket
  if(bind(parent_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) == -1)
  {
    printf("bind() call failed!\n");
    exit(1);
  }
  
  // start listening for incoming connections from web browser
  if(listen(parent_fd, 10) == -1 )
  {
    printf("listen() call failed!\n");
    exit(1);
  }

  // UDP setup and address initialization
  struct sockaddr_in udp_server;
  struct sockaddr_in udp_client;
  struct sockaddr *server;
  struct sockaddr *client;
  int len;
  memset((char *) &udp_server, 0, sizeof(udp_server));
  udp_server.sin_family = AF_INET;
  udp_server.sin_port = htons(TCP_UDP_port);
  udp_server.sin_addr.s_addr = htonl(INADDR_ANY);
  server = (struct sockaddr *) &udp_server;
  client = (struct sockaddr *) &udp_client;
  len = sizeof(udp_server);

  // create UDP socket
  int udp_socket_fd;
  udp_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(udp_socket_fd == -1)
  {
    printf("udp socket() call failed!\n");
    return -1;
  }

  // bind UDP socket to address
  if(bind(udp_socket_fd, server, sizeof(udp_server)) < 0)
  {
    printf("UDP bind() to port %d failed!\n", TCP_UDP_port);
    return -1;
  }
  printf("Server is now running on TCP port %d and UDP port %d...\n", TCP_UDP_port, TCP_UDP_port);
  
  printf("Waiting for incoming connections...\n");
  struct_size = sizeof(struct sockaddr_in);
  while(1) // keep accepting clients serially
  { 
    // accept an incoming TCP connection with client
    child_fd = accept(parent_fd, (struct sockaddr*)&address, (socklen_t*)& struct_size);
    if(child_fd == -1)
    {
      printf("accept() failed\n");
      return 1;
    }
    printf("Connection to TCP server has been accepted.\n");

    int byte;
    int selection = -1;
    byte = recv(child_fd, choice, 1, 0); // recieve client function choice
    if(byte == -1)
    {
      error_num = errno;
      fprintf(stderr, "recv() has failed due to the following error number: %d\n", error_num);
    }
    selection = choice[0];
    bzero(choice, 1);
    while(selection) // iterates through selections until client exits
    {
      if(selection == 2) // devowel the recieved message
      {
        fprintf(stderr, "\nThe client has selected %d as their choice which is devowel.\n\n", selection);
        int tcp_bytes; // bytes involved in TCP function calls
        int udp_bytes; // bytes involved in UDP function calls
        while(tcp_bytes = recv(child_fd, recMessage, SIZE, 0) > 0) // loop while recieving TCP input message
        {
          // check newline and carriage return twice (two if statements: mypal-server.c)
          if((tcp_bytes > 0) && ((recMessage[tcp_bytes-1] == '\n') || (recMessage[tcp_bytes-1] == '\r')) )
          {
            recMessage[tcp_bytes-1] = '\0'; // place null terminator instead
            tcp_bytes--; // subtract total TCP bytes recieved
          }
          if((tcp_bytes > 0) && ((recMessage[tcp_bytes-1] == '\n') || (recMessage[tcp_bytes-1] == '\r')) )
          {
            recMessage[tcp_bytes-1] = '\0';
            tcp_bytes--;
          }
          int tcp_len = strlen(recMessage); // length of message recieved
          fprintf(stderr, "Server recieved %i bytes through TCP and the entire message is:\n%s\n", tcp_len, recMessage);
          
          #if SIMPLE // use simple devowel technique
          devowelSimple(storedVowels, recMessage);
          strcpy(storedOthers, recMessage);
          #endif

          #if ADVANCED // use advanced devowel technique
          markSpaces(recMessage);
          devowelAdvanced(storedVowels, recMessage);
          removeSpaces(recMessage);
          demarkSpaces(recMessage);
          strcpy(storedOthers, recMessage);
          #endif

          int others_len = strlen(storedOthers);
          tcp_bytes = send(child_fd, storedOthers, others_len, 0); // send other letters/punctuation through TCP
          if(tcp_bytes == -1)
          {
            error_num = errno;
            fprintf(stderr, "send() failed sendng others in devowel with the following error number: %d\n", error_num);
          }
          else
          {
            fprintf(stderr, "\nSent %d bytes to the client through TCP with the following message:\n%s\n", tcp_bytes, storedOthers);
          }

          bzero(recMessage, SIZE); // clear buffer
          int wake_up;
          wake_up = recvfrom(udp_socket_fd, recMessage, SIZE, 0, client, (socklen_t*)&len); // recieve wake up message
          if(wake_up == -1)
          {
            error_num = errno;
            fprintf(stderr, "recvfrom() has failed and the error number is: %i\n", error_num);
          }

          int vowels_len = strlen(storedVowels);
          udp_bytes = sendto(udp_socket_fd, storedVowels, vowels_len, 0, client, len); // send vowels to client through UDP
          if(udp_bytes == -1)
          {
            error_num = errno;
            printf("UDP sendto() has failed and the error number is: %i\n", error_num);
          }
          else
          {
            fprintf(stderr, "\nSent %d bytes with to the client through UDP with the following message:\n%s\n", udp_bytes, storedVowels);
          }

          // clear out message strings again to be safe 
          bzero(recMessage, SIZE);
          bzero(storedVowels, SIZE);
          bzero(storedOthers, SIZE);
          bzero(choice, 1);

          fprintf(stderr, "\nAwaiting the client's next selection ...\n");
          byte = recv(child_fd, choice, 1, 0); // recv() clients function selection
          if(byte == -1)
          {
            error_num = errno;
            fprintf(stderr, "recv() has failed due to the following error number: %d\n", error_num);
          }
          selection = choice[0]; // clients int selection
          bzero(choice, 1);
          break;
        }
        if(tcp_bytes == -1)
        {
          error_num = errno;
          fprintf(stderr, "recv() has failed due to the following error number: %d\n", error_num);
        }
      }
      if(selection == 1) // envowel the two recieved messages
      {
        fprintf(stderr, "\nThe client has selected %d as their choice which is envowel.\n", selection);
        int tcp_bytes; // bytes involved with TCP function calls
        int udp_bytes; // bytes involved with UDP function calls
        while((tcp_bytes = recv(child_fd, storedOthers, SIZE, 0) > 0) &&
        (udp_bytes = recvfrom(udp_socket_fd, storedVowels, SIZE, 0, client, (socklen_t*)&len) > 0)) // loop while recieving through both sockets
        {
          // check newline and carriage return twice for accurate byte count (mypal-server.c)
          if((tcp_bytes > 0) && ((storedOthers[tcp_bytes-1] == '\n') || (storedOthers[tcp_bytes-1] == '\r')) )
          {
            storedOthers[tcp_bytes-1] = '\0'; // place null terminator instead
            tcp_bytes--; // subtract total TCP bytes recieved
          }
          if((tcp_bytes > 0) && ((storedOthers[tcp_bytes-1] == '\n') || (storedOthers[tcp_bytes-1] == '\r')) )
          {
            storedOthers[tcp_bytes-1] = '\0';
            tcp_bytes--;
          }
          int tcp_len = strlen(storedOthers); // total TCP bytes
          fprintf(stderr, "\nServer recieved %i bytes through TCP and the non-vowels are the following:\n%s\n", tcp_len, storedOthers);
        
          if((udp_bytes > 0) && ((storedVowels[udp_bytes-1] == '\n') || (storedVowels[udp_bytes-1] == '\r')) )
          {
            storedVowels[udp_bytes-1] = '\0'; // place null terminator instead
            udp_bytes--; // subtract total UDP bytes recieved
          }
          if((udp_bytes > 0) && ((storedVowels[udp_bytes-1] == '\n') || (storedVowels[udp_bytes-1] == '\r')) )
          {
            storedVowels[udp_bytes-1] = '\0';
            udp_bytes--;
          }
          int udp_len = strlen(storedVowels); // total UDP bytes
          fprintf(stderr, "\nServer recieved %i bytes through UDP and the vowels are the following:\n%s\n", udp_len, storedVowels);

          #if SIMPLE // simple envowel technique
          envowelSimple(sendMessage, storedVowels, storedOthers);
          #endif

          #if ADVANCED // advanced envowel technique
          envowelAdvanced(sendMessage, storedVowels, storedOthers);
          #endif

          int total_len = strlen(sendMessage);
          tcp_bytes = send(child_fd, sendMessage, total_len, 0);
          if(tcp_bytes == -1)
          {
            error_num = errno;
            fprintf(stderr, "send() has failed and the error number is: %i\n", error_num);
          }
          else
          {
            fprintf(stderr, "\nServer sent %i bytes to the client and the merged message is the following:\n%s\n", total_len, sendMessage);
          }

          // clear out message strings again to be safe 
          bzero(sendMessage, SIZE);
          bzero(storedVowels, SIZE);
          bzero(storedOthers, SIZE);
          bzero(choice, 1);

          fprintf(stderr, "\nAwaiting the client's next selection ...\n");
          byte = recv(child_fd, choice, 1, 0);
          if(byte == -1)
          {
            error_num = errno;
            fprintf(stderr, "recv() failed and the error number is: %i\n", error_num);
          }
          selection = choice[0];
          bzero(choice, 1);
          break;
        }
        if(tcp_bytes == -1)
        {
          error_num = errno;
          fprintf(stderr, "recv() failed and the error number is: %i\n", error_num);
        }
        if(udp_bytes == -1)
        {
          error_num = errno;
          fprintf(stderr, "recvfrom() has failed and the error number is: %i\n", error_num);
        }
      }
      if(selection == 0) // server can keep on accepting clients or can close its sockets with CTRL + C
      {
        fprintf(stderr, "\nThe client has selected %d as their choice which is exit.\n", selection);
        fprintf(stderr, "The client has exited. Bye for now.\n");
        fprintf(stderr, "The server will continue accepting clients until CTRL + C is used to signal closing its sockets.\n");
        selection = -1;
        break;
      }
    }
  }
  close(parent_fd); // close parent socket fd
  close(child_fd); // close child socket fd
  return 0;
}
