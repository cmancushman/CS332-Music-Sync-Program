#include <stdlib.h> //for atoi()
#include <ctype.h>  // For isspace
#include <openssl/sha.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <dirent.h>//for directories
#include <vector>
#include <time.h>
#include <iomanip>
#include <typeinfo>
#include <stdio.h>//for printf and fprintf
#include <sys/socket.h>//for socket and bind
#include <arpa/inet.h>//for sockaddr_in and inet_ntoa
#include <string>//for to_string
#include <netdb.h> //for gethostbyname()
#include <unistd.h>//for close()
#include <iostream>//for I/O
#include <signal.h>//for sigaction()
#include <errno.h>//for errno, EINTR
#include <sstream>//for hex
#include <bitset>//for binary
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

#define FILE_NAME_SIZE 66

#define CONSTANT_SIZE 6 //header size in bytes

#define ECHOMAX 100000//max echo length string

#define SONG_SIZE 10000000

#define SONG_HEADER_TITLE_SIZE 100

#define SONG_LENGTH_SIZE 8

#define SONG_TOTAL_SIZE 10000108

#define SA struct sockaddr

#define SIN struct sockaddr_in

/* Struct S, used as our datagram to send UDP messages */
struct S {
  unsigned short version : 4;//version, set to 0110
  unsigned short type : 4;//type   0000 for SERVER LIST, 0001 for DIFF, 0010 for SYNC, 0011 for LEAVE, 0100 for SERVER download,
                          //0101 for CLIENT LIST, 0110 for CLIENT DOWNLOAD
  unsigned char length : 8;//length (number of objects pased in data)
  unsigned short queryID : 16;//queryID, a randomly generated number
  unsigned short checksum : 16;//checksum of the packet
  char* data;//the data being sent
};

//Struct Song used to store data about songs including the song title and the hash
struct Song {
  string title;
  string hash;
};

int AcceptTCPConnection(int servSock);//accept client

int CreateTCPServerSocket(unsigned short port);//create server socket

bool stringsAreTheSame(string string1, string string2);//check if two strings are the same(with buffer differences)

void HandleTCPClient(int clntSock);//handle client

void compareFiles(vector<string> &files, int recvMsgSize, struct S *demo, bool both);//looks for differences between arrays of files

int retreiveFileToSend(string fileName, struct S *songs);//puts data of single local song into a packet

void downloadSongs(struct S *songs);//downlaods songs to current directory

void formatFileNames(char** names, int numberOfUsernames, char* buffer);//takes the usernames and login times from the 2D char array and fits them in a 1D char array

void printPacket(struct S *demo, int recvMsgSize);//prints a S struct (packet) in a formatted manner

void constructPacket(char echoBuffer[], struct S *demo, int recvMsgSize);//builds a struct from a char[]

void constructCharArray(char echoBuffer[], struct S *demo, int size);

unsigned short getChecksum(struct S demo, int sizeOfData);//constructs a checksum from a packet and returns it

int getdir (string dir, vector<string> &files);//get files in a directory and their hashes

int calc_sha256 (char* path, char output[65]);//hash

void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]);//hash
