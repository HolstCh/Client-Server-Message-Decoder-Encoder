# Vowel Message Decoder/Encoder
Progam developed on Linux which encodes and decodes vowels using a TCP and UDP connection
## User manual
1. In vowelizerS.cpp, select a directive for your desired string technique. Select a directive labelled
SIMPLE or ADVANCED at the top of the file. 1 will enable all corresponding string technique
functions and 0 will disable/comment out all other corresponding string technique functions.

2. Compile vowelizerS.cpp and vowelizerC.cpp with the following command on a CPSC Linux
server (server usually ran on csx3 and the client connects to csx3 by default but other options are
available as directives to connect to a running server in vowelizerC.cpp):

    g++ -o vowelizerS vowelizerS.cpp
  
    g++ -o vowelizerC vowelizerC.cpp

3. First, execute the server (S) with your choice of port number as the command line argument. For
this example, we will choose port number 11140 and it will always be the same TCP and UDP
port. Then, execute the client (C) as follows:

    ./vowelizerS 11140
    
    ./vowelizerC 11140
  
4. As client, select 2 for devowel, 1 for envowel, or 0 to exit. Enter text for each corresponding
function as a complete message (2) or non-vowels message and then vowels message (1).

5. When finished, exit is selected to close client socket. The server will connect with another client
serially or CTRL + C can then be used to signal the server to close its sockets.
