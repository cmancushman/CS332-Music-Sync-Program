
#include "WhoHeader.h"//for database

#define MAXPENDING 5//max seconds pending


void DieWithError(char *errorMessage);//error handling method
void writeToLog(char* temp1, char* temp3);



int main(int argc, char *argv[]){

  //socket components

  int *servSock;
  int clntSock;


  unsigned short echoServPort;//server port

  int maxDescriptor;
  fd_set sockSet;
  long timeout;
  struct timeval selTimeout;
  int running = 1;
  int noPorts;
  int port;
  unsigned short portNo;





  if(argc < 5){//check argument count to assure proper number of parameters

    fprintf(stderr, "Usage: %s -t <timeout> -p <Port 1>...\n", argv[0]);
    exit(1);

  }

  //handles user input
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      char c = argv[i][1];
      switch (c) {
        case 't':
        timeout = atol(argv[i+1]);//-d for filename of database
        break;
        default:
        break;
      }
    }
  }

  //get number of ports
  noPorts = argc - 4;
  //create socket for datagrams
  servSock = (int*)malloc(noPorts * sizeof(int));
  maxDescriptor = -1;
  //create ports
  for(port = 0; port < noPorts;port++){
    portNo = atoi(argv[port + 4]);
    servSock[port] = CreateTCPServerSocket(portNo);
    if(servSock[port] > maxDescriptor)
    maxDescriptor = servSock[port];
  }

  //wait for incoming connections
  while(running){
    //init ports
    FD_ZERO(&sockSet);
    FD_SET(STDIN_FILENO, &sockSet);
    for(port = 0; port < noPorts; port++){
      FD_SET(servSock[port], &sockSet);
    }
    selTimeout.tv_sec = timeout;
    selTimeout.tv_usec = 0;

    //ports have been idle for timeout time
    if(select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0){
      cout<<"Server alive, no requests for "<<timeout<<" seconds"<<endl;
    }else{
      //shut down server
      if(FD_ISSET(STDIN_FILENO, &sockSet)){
        cout<<"shutting down server"<<endl;
        getchar();
        running = 0;
      }
    }
    //receive connection, handle it accordingly
    for(port = 0; port<noPorts;port++){
      if(FD_ISSET(servSock[port], &sockSet)){
        HandleTCPClient(AcceptTCPConnection(servSock[port]));
      }
    }
  }

  //done running, close ports and shut down server, free data and exit
  for(port = 0; port < noPorts; port++){
    close(servSock[port]);
  }
  free(servSock);
  exit(0);

}

//accept connection from client
int AcceptTCPConnection(int servSock){


  int clntSock;//client socket
  struct sockaddr_in echoClntAddr;//client address
  unsigned int clntLen;
  clntLen = sizeof(echoClntAddr);

  //accept client
  if((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0){

    DieWithError((char*)"accept() failed");

  }

  //acknowledge client connection and write to log
  printf("Handling client %s... \n\n", inet_ntoa(echoClntAddr.sin_addr));
  time_t my_time = time(NULL);
  writeToLog(inet_ntoa(echoClntAddr.sin_addr), ctime(&my_time));

  return clntSock;

}

//create TCP server socket
int CreateTCPServerSocket(unsigned short port){

  int sock;
  struct sockaddr_in echoServAddr;//local address;
  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
  DieWithError((char*)"socket() failed");

  //construct local address structurehttp://www.unit-conversion.info/texttools/convert-text-to-binary/
  memset(&echoServAddr, 0, sizeof(echoServAddr));//zero out structure
  echoServAddr.sin_family = AF_INET;//internet address family
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);//any incoming interface
  echoServAddr.sin_port = htons(port);//local port

  //bind to the local address
  if(bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
  DieWithError((char*)"bind() failed");

  //start listening
  if(listen(sock, MAXPENDING) < 0)
  DieWithError((char*)"listen() failed");

  return sock;


}

void HandleTCPClient(int clntSock){

  for(;;){//run forever
    int recvMsgSize;//size of received message
    /*Listen for connections*/
    struct S demo;//struct that will receive the data
    demo.data = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);

    /*Handle connnection by taking in as many bytes as possible at a time*/
    char* echoBuffer = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);//echo buffer to be written to
    char* echoBufferPlaceholder = echoBuffer;//placeholder for buffer to return to after receiving
    int messageMaxBytes = 10000000;//default max size of a message in bytes
    bool messageMaxSet = false;//tells whether the actual length of packet has been set
    int bytesReceived = 0;//number of bytes received
    recvMsgSize = 0;//size of message blocks being sent one at a time
    while(bytesReceived < messageMaxBytes){
      if((recvMsgSize = recv(clntSock, echoBuffer, SONG_TOTAL_SIZE * 10 + CONSTANT_SIZE, 0)) < 0){
        DieWithError((char*)"recvfrom() failed");
      }
      bytesReceived += recvMsgSize;//increment bytes received by incoming amount
      if(!messageMaxSet){
        //get the length and type fromthe first two bytes and use them to read the rest of the packet
        string firstPart = bitset<8>(echoBuffer[0]).to_string();
        string type = firstPart.substr(4,4);
        unsigned short typeShort = (unsigned short)bitset<4>(type).to_ulong();
        unsigned short length = (unsigned short)bitset<8>(echoBuffer[1]).to_ulong();
        messageMaxBytes = (int)length * ((typeShort == 4) ? SONG_TOTAL_SIZE : FILE_NAME_SIZE) + CONSTANT_SIZE;
        messageMaxSet = true;
      }
      echoBuffer += recvMsgSize;//point to the next part of memory as to avoid overwrites
    }
    echoBuffer = echoBufferPlaceholder;//return to the start of the array

    //construct the packet from our buffer
    constructPacket(echoBuffer, &demo, bytesReceived);

    free(echoBuffer);

    if(demo.type == 3){
      cout<<"Ending connection..."<<endl;
      close(clntSock);
      break;
    }

    /*Receive packet, vverify checksum*/
    cout<<"Received packet..."<<endl;
    printPacket(&demo, bytesReceived);

    if(demo.type == 4) {
      cout<<"Downloading songs..."<<endl;
      downloadSongs(&demo);
      cout<<"Songs downloaded!"<<endl;
    }

    //retreive usernames and logintimes from database
    cout<<endl<<"Retrieving files..."<<endl;
    string dir = string(".");
    vector<string> files = vector<string>();
    if(demo.type!=4) getdir(dir,files);

    //construct response packet to send
    cout<<"Creating Packet..."<<endl;

    //init struct
    struct S response = {6,5, 0, demo.queryID, 0};
    response.data = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);
    response.type = (demo.type == 4) ? 5 : demo.type;

    if(demo.type == 1 || demo.type == 6){
      //form vector of local songs
      compareFiles(files, recvMsgSize, &demo, (demo.type == 1));

    }

    //assign length
    response.length = (unsigned char)(files.size());

    int size = 0;


    if(demo.type == 6){
      //client wants files to download from server, retreive those files and send
      response.length = 0;
      for(int x = 0; x < files.size();x+=2){
        retreiveFileToSend(files[x], &response);
      }
      size = SONG_TOTAL_SIZE * (int)files.size()/2;
    }else{
      //client wants some list of file names/hashes

      //convert vector to char array
      char *userList[1000];
      for (unsigned int i = 0;i < files.size();i++) {
        userList[i] = (char*)(files[i].data());
      }
      //assign usernames and login times to response.data
      formatFileNames(userList, files.size(), response.data);
      //produce checksum
      size = files.size() * FILE_NAME_SIZE;//size is the number of bytes of the response.data to be sent
    }

    response.checksum = getChecksum(response, size);
    cout<<"Packet Created..."<<endl;

    ///print packetint getdir (string dir, vector<string> &files)
    printPacket(&response, size + CONSTANT_SIZE);
    char* sendBuffer = (char*) malloc(size+ CONSTANT_SIZE);
    constructCharArray(sendBuffer, &response, size);

    cout<<endl<<"Sending Packet..."<< endl;
    //send response packet
    if(send(clntSock,sendBuffer, (CONSTANT_SIZE + size), 0)!= (CONSTANT_SIZE+ size))
      DieWithError((char*)"sendto() sent a different number of bytes than expected!");

    cout<<"Packet Sent..."<<endl<<endl;

    free(sendBuffer);
    free(response.data);

  }

}







void DieWithError(char *errorMessage){

  cout<<"Error: "<<errorMessage<<endl;
  exit(1);

}

void writeToLog(char* temp1, char* temp3)
{
  FILE* pFile = fopen("logFile.txt", "a+");
  char c = fgetc(pFile);
  if(c == EOF)
  {
    fprintf(pFile, "%-10s %-4s %-10s", "Name/Address","", "Time");
  }
  fprintf(pFile, "%-10s %-4s %-10s", temp1,"", temp3);
  fclose(pFile);
}
