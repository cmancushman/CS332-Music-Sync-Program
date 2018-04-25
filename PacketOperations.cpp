#include "WhoHeader.h"

//downloads songs from an incoming packet--only writes one song at a time
void downloadSongs(struct S *songs){

  //start at the length offset of the packet and write the song
  for(int x = 0; x < (int)songs->length; x++){

    int offset = x * (SONG_SIZE + SONG_LENGTH_SIZE + SONG_HEADER_TITLE_SIZE);

    //get length of song from first 8 bytes
    string songLengthString = "";
    for(int y = 0; y < SONG_LENGTH_SIZE; y++) songLengthString += songs->data[y + offset];
    int sizeOfSong = atoi(songLengthString.c_str());


    //get title of song from next 100 bytes
    string songTitle = "";
    for(int y = 0; y < SONG_HEADER_TITLE_SIZE; y++) songTitle += songs->data[y + offset + SONG_LENGTH_SIZE];

    //download remaining bytes to file with song name
    ofstream sw(songTitle.c_str());
    for(int i = 0; i < sizeOfSong; i++)
    {
      sw << songs->data[i + offset + SONG_LENGTH_SIZE + SONG_HEADER_TITLE_SIZE];
    }


  }


}

//construct packet from char array
void constructPacket(char echoBuffer[], struct S *demo, int recvMsgSize){

  //break up the first byte of the echo buffer into our version and type
  string firstPart = bitset<8>(echoBuffer[0]).to_string();
  string version = firstPart.substr(0,4);
  string type = firstPart.substr(4,4);
  demo->version = (unsigned short)bitset<4>(version).to_ulong();
  demo->type = (unsigned short)bitset<4>(type).to_ulong();

  //next byte is the length
  demo->length = (unsigned short)bitset<8>(echoBuffer[1]).to_ulong();

  //construct queryID from next two bytes
  string queryID1 = bitset<8>(echoBuffer[2]).to_string();
  string queryID2 = bitset<8>(echoBuffer[3]).to_string();
  string queryID = queryID1 + queryID2;
  demo->queryID = (unsigned short)bitset<16>(queryID).to_ulong();

  //construct the checksum from next two bytes
  string checksum1 = bitset<8>(echoBuffer[4]).to_string();
  string checksum2 = bitset<8>(echoBuffer[5]).to_string();
  string checksum = checksum1 + checksum2;
  demo->checksum = (unsigned short)bitset<16>(checksum).to_ulong();

  //for all remaining bytes, construct our data
  for(int x = 6; x < recvMsgSize; x++){
    unsigned long i = bitset<8>(echoBuffer[x]).to_ulong();
    unsigned char c = static_cast<unsigned char>( i );
    demo->data[x-6] = c;
  }

}

//construct char array from packet struct
void constructCharArray(char echoBuffer[], struct S *demo, int size){

  //concatenate the packet struct's 4-bit version and 4-bit type and set that byte to the first char of the char array
  string version = bitset<4>(demo->version).to_string();
  string type = bitset<4>(demo->type).to_string();
  string firstPart = version + type;
  unsigned long i = bitset<8>(firstPart).to_ulong();
  unsigned char c = static_cast<unsigned char>( i );
  echoBuffer[0] = c;

  //create a byte with value of the packet's length and set it to the second char in the char array
  i = bitset<8>(demo->length).to_ulong();
  c = static_cast<unsigned char>( i );
  echoBuffer[1] = c;

  //deconstruct the packet's queryID into 2 8-bit strings and set them to the 3rd and 4th byte of the char array
  string queryID = bitset<16>(demo->queryID).to_string();
  string queryID1 = queryID.substr(0,8);
  string queryID2 = queryID.substr(8,8);;
  i = bitset<8>(queryID1).to_ulong();
  c = static_cast<unsigned char>( i );
  echoBuffer[2] = c;
  i = bitset<8>(queryID2).to_ulong();
  c = static_cast<unsigned char>( i );
  echoBuffer[3] = c;

  //deconstruct the packet's checksum into 2 8-bit strings and set them to the 5th and 6th byte of the char array
  string checksum = bitset<16>(demo->checksum).to_string();
  string checksum1 = checksum.substr(0,8);
  string checksum2 = checksum.substr(8,8);;
  i = bitset<8>(checksum1).to_ulong();
  c = static_cast<unsigned char>( i );
  echoBuffer[4] = c;
  i = bitset<8>(checksum2).to_ulong();
  c = static_cast<unsigned char>( i );
  echoBuffer[5] = c;

  //for all remaining bytes, put the packets data in to the char array byte by byte
  for(int x = 0; x < size; x++){

    unsigned long i = bitset<8>(demo->data[x]).to_ulong();
    unsigned char c = static_cast<unsigned char>( i );
    echoBuffer[x+6] = c;

  }

}


void printPacket(struct S *demo, int recvMsgSize){

  cout<<"Packet:"<<endl<<" size->"<<recvMsgSize<<endl<<" version->"<<demo->version<<endl<<" type->"
  <<demo->type<<endl<<" length->"<<(unsigned short)demo->length
  <<endl<<" queryID->"<<demo->queryID
  <<endl<<" checksum->"<<demo->checksum<<endl;

  cout<<" data: "<<endl;


  //print two columns of user data
  unsigned short len = (unsigned short)demo->length;

  if(demo->type != 4 && demo->type != 6){

    for(int x = 0; x< len-1; x+=2){

      cout<<"   song->";
      for(int username = 0; username < FILE_NAME_SIZE; username++){
        //cout<<username + x * FILE_NAME_SIZE;
        cout<<demo->data[username + x * FILE_NAME_SIZE];
      }
      cout<<"   hash->";
      for(int username = FILE_NAME_SIZE; username < FILE_NAME_SIZE*2; username++){
        cout<<demo->data[username + x * FILE_NAME_SIZE];
      }

      cout<<endl;
    }
  }else{

    cout<<(unsigned short)demo->length<<" mp3 files"<<endl;

  }

  cout<<endl;
  return;

}
