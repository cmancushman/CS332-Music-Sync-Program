
#include "WhoHeader.h"

void DieWithError(char *errorMessage);//method failed, reports error

unsigned long getHostByName(char name[]);//resolve non IPV4-style hostnames

int ensureDataIsEven(char *data);//adds padded byte space to data if it is not even



int main(int argc, char *argv[]){

  int sock;//socket descriptor
  struct sockaddr_in echoServAddr;//echo server address
  struct sockaddr_in fromAddr;//source address of echo
  unsigned short echoServPort = 30500;//echo server port
  unsigned int fromSize;//in-out of address size for recvfrom()
  char *servIP = (char*)"mathcs01";//IP address of server
  struct sigaction myAction;//for setting signal handler
  int timeout;//duration before timeout
  int maxRetries;//maximum number of retries
  SIN serverAddress;

  if((argc < 7) || (argc > 11)){//test for correct number of arguments

    fprintf(stderr, "Usage: %s [-h <serverIP>] [-p <port>] -t <timeout> -i <max-retries>", argv[0]);
    exit(1);

  }

  //handles user input
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      char c = argv[i][1];
      switch (c) {
        case 'h':
        servIP = argv[i+1];//-h for serverIP
        break;
        case 'p':
        echoServPort = atoi(argv[i+1]);//-p for port
        break;
        case 't':
        timeout = atoi(argv[i+1]);//-t for timeout time
        break;
        case 'i':
        maxRetries = atoi(argv[i+1]);//-i for max number of retries
        break;
        default:
        break;
      }
    }
  }

  //create UDP Socket
  cout<<"Creating Socket..."<<endl;
  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    DieWithError((char*)"socket() failed");}
    cout<<"Socket Created..."<<endl<<endl;

    //Construct the server address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = getHostByName(servIP);
    serverAddress.sin_port = htons(echoServPort);

    //connect socket to server
    if(connect(sock, (SA *) &serverAddress, (sizeof(serverAddress))) < 0){

      DieWithError((char *)"Socket Connect Failed");

    }else{

      cout<<"Connected to host..."<<endl;

    }

    for(;;){
      //input command
      cout<<"Enter command(LIST, DIFF, SYNC, LEAVE): ";
      string temp;
      cin>>temp;
      cout<<endl;

      if(temp.compare("DIFF") != 0 && temp.compare("SYNC") != 0 && temp.compare("LIST") != 0 && temp.compare("LEAVE") != 0){

        cout<<"Invalid command..."<<endl;
        continue;

      }

      //generating random queryID
      srand(time(NULL));
      unsigned short queryID = rand() % 65536;

      //initialize our demo -- 6 is the version, 3 is the type,
      //0 is the length, queryID is the query ID,
      //and 0 is the uncreated checksum
      struct S demo = {6, 3, 0, queryID, 0};
      demo.data = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);


      char *fileList[ECHOMAX];
      int size = 0;

      if(temp.compare("LIST") == 0) demo.type = (unsigned short)0;

      if(temp.compare("DIFF") == 0 || temp.compare("SYNC") == 0){
        //for DIFF and SYNC, send files to be compared

        demo.type = (unsigned short)1;

        string dir = string(".");
        vector<string> files = vector<string>();
        getdir(dir,files);
        size = files.size() * FILE_NAME_SIZE;
        demo.length = files.size();
        for (unsigned int i = 0;i < files.size();i++) {
          fileList[i] = (char*)(files[i].data());
        }

        formatFileNames(fileList, files.size(), *(&demo.data));

      }

      if(temp.compare("SYNC") == 0) demo.type = (unsigned short)2;

      /*
      * Construct our struct to send to the server
      */
      //generate qurey ID through a random number less tahn 65536 (16 bit)
      cout<<endl<<"Composing Packet..."<<endl<<endl;

      //forming the checksum of the data using getChecksum()
      unsigned short checksum = getChecksum(demo, size);
      demo.checksum = checksum;

      cout<<"Packet Created"<<endl;
      printPacket(&demo, size + CONSTANT_SIZE);
      cout<<endl;

      //send the UDP datagram
      cout<<"Sending Packet..."<<endl;

      //send packet
      char* echoBuffer = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);
      constructCharArray(echoBuffer, &demo, size);
      if(send(sock, echoBuffer, CONSTANT_SIZE + size, 0)!= (CONSTANT_SIZE + size))
      DieWithError((char*)"sendto() sent a different number of bytes than expected!");
      cout<<"Packet Sent..."<<endl<<endl;

      if(demo.type == 3) break;//LEAVE command executed

      for(;;){

        /*Send and receive the packets from the client*/
        char* receiveBuffer = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);//buffer to be written to
        char* receiveBufferPlaceholder = receiveBuffer;//placeholder for buffer to return to after receiving
        struct S response;//init receiving struct
        response.data = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);
        fromSize = sizeof(fromAddr);//size of server address

        /*Handle connnection by taking in as many bytes as possible at a time*/
        int messageMaxBytes = 10000000;//default max size of a message in bytes
        bool messageMaxSet = false;//tells whether the actual length of packet has been set
        int bytesReceived = 0;//number of bytes received
        int recvMsgSize = 0;//size of message blocks being sent one at a time
        while(bytesReceived < messageMaxBytes){
          if((recvMsgSize = recv(sock, receiveBuffer, SONG_TOTAL_SIZE * 10 + CONSTANT_SIZE, 0)) < 0){
            DieWithError((char*)"recvfrom() failed");
          }
          bytesReceived += recvMsgSize;//increment bytes received by incoming amount
          if(!messageMaxSet){
            //get the length and type fromthe first two bytes and use them to read the rest of the packet
            string firstPart = bitset<8>(receiveBuffer[0]).to_string();
            string type = firstPart.substr(4,4);
            unsigned short typeShort = (unsigned short)bitset<4>(type).to_ulong();
            unsigned short length = (unsigned short)bitset<8>(receiveBuffer[1]).to_ulong();
            messageMaxBytes = (int)length * ((typeShort == 4 || typeShort == 6) ? SONG_TOTAL_SIZE : FILE_NAME_SIZE) + CONSTANT_SIZE;
            messageMaxSet = true;
          }

          receiveBuffer += recvMsgSize;//point to the next part of memory as to avoid overwrites
        }
        receiveBuffer = receiveBufferPlaceholder;//return to the start of the array

        //construct the packet from our buffer
        constructPacket(receiveBuffer, &response, bytesReceived);

        //free(receiveBuffer);

        //source is not the same as that which we sent data to
        if(echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
          fprintf(stderr, "Error: received a packet from unknown source.\n");
          exit(1);
        }

        cout<<"Packet Received..."<<endl;
        printPacket(&response, bytesReceived);

        //for LIST and DIFF no more work is necessary
        if(response.type == 0 || response.type == 1) break;

        //for receiving files, download songs
        if(response.type == 6) {
          cout<<"Downloading songs..."<<endl;
          downloadSongs(&response);
          cout<<"Songs downloaded!"<<endl;
          break;
        }

        //get local file names and hashes
        string dir = string(".");
        vector<string> files = vector<string>();
        getdir(dir,files);

        //create response packet
        struct S clientResponse = {6,6, 0, demo.queryID, 0};
        clientResponse.data = (char*) malloc(sizeof(char) * SONG_TOTAL_SIZE * 10);

        int size = 0;
        if(response.type == 2){
          //send files for the server to download

          clientResponse.type = 4;
          compareFiles(files, bytesReceived, &response, false);
          for(int x = 0; x < files.size();x+=2){
            retreiveFileToSend(files[x], &clientResponse);
          }
          size = SONG_TOTAL_SIZE * (int)clientResponse.length;

        }else{
          //send list of local files to server

          clientResponse.length = (unsigned char)(files.size());
          //convert vector to char array
          char *userList[1000];
          for (unsigned int i = 0;i < files.size();i++) {
            userList[i] = (char*)(files[i].data());
          }
          //assign the file names to the data
          formatFileNames(userList, files.size(), clientResponse.data);
          size = files.size() * FILE_NAME_SIZE;

        }

        //send response packet
        clientResponse.checksum = getChecksum(clientResponse, size);
        printPacket(&clientResponse, size + CONSTANT_SIZE);
        char* newBuffer = (char*) malloc(size + CONSTANT_SIZE);
        constructCharArray(newBuffer, &clientResponse, size);
        int newsize = send(sock, newBuffer, (size + CONSTANT_SIZE), 0);
        if(newsize != (size + CONSTANT_SIZE)){
          DieWithError((char*)"sendto() sent a different number of bytes than expected!");
        }
        cout<<"Packet Sent..."<<endl<<endl;

        //free(response.data);
        //free(newBuffer);

      }

    }

    //end
    close(sock);
    exit(0);

  }







  unsigned long getHostByName(char name[]){

    struct hostent *host;

    if((host = gethostbyname(name)) == NULL){
      fprintf(stderr, "gethostbyname() failed");
      exit(1);
    }

    return *((unsigned long *) host->h_addr_list[0]);

  }

  void DieWithError(char *errorMessage){

    printf("Error: %s", errorMessage);
    exit(1);

  }
