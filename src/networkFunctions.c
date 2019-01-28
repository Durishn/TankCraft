/***************************************a1.c*************************************
   Nic Durish (#0757227)(ndurish@uoguelph.ca)                  March 18th, 2015
   CIS 4820 - Game Programming                                 Assignment 4
The following Game Package contains the files; a1.c, visible.c, graphics.c,
graphics.h and a makefile. Together these files can be compiled to build a game
world. The following game world consists of a procedurally generated map, basic
collision control, gravity and cloud movement.

* LICENSE *
***********
The MIT License (MIT)

Copyright (c) 2015 Nic Durish

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


* REFERENCES *
**************
Algorithms from http://freespace.virgin.net/hugo.elias/models/m_perlin.htm have
been implemented in the following functions; Noise, CosineInterpolate,
InterpolateNoise, SmoothedNoise and PerlinNoise_2D.
********************************************************************************/
#include "tankcraft.h"

/*serverMode(): sets up instance as a server */
void serverMode(){

   int server_sockfd, server_len, client_len;
   struct sockaddr_in server_address;
   struct sockaddr_in client_address;

   /* remove old sockets, create, name and connent a socket. */
   server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(9734);
   bind(server_sockfd, (struct sockaddr *)&server_address
     , sizeof(server_address));
   listen(server_sockfd, 5);

   /* accept a connection & wait for a connection from the client */
   printf("\nWaiting for client... \n");
   client_len = sizeof(client_address);
   client_sockfd = accept(server_sockfd, 
      (struct sockaddr *)&client_address, (socklen_t*)&client_len);
}

/*clientMode(): sets up instance as a client */
void clientMode(){

   struct sockaddr_in address;
   int result;

   /* create, name, and connect a socket as agreed by the server. */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = inet_addr("127.0.0.1");
   address.sin_port = htons(9734);
   result = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
   if(result == -1) {
      perror("\nClient Error");
      exit(1);
   }
}