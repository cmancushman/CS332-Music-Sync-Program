#include "WhoHeader.h"



unsigned short getChecksum(struct S demo, int sizeOfData){

  //add the demo, version and type with length, and then add that with queryID (16bit int + 16bit int)
  string version = (bitset<4>((unsigned short)demo.version)).to_string();;
  string type = (bitset<4>((unsigned short)demo.type)).to_string();;
  string length = (bitset<8>((unsigned short)demo.length)).to_string();;
  string staticFields = version + type  + length;

  int staticFieldsInt = (int)(bitset<16>(staticFields).to_ulong());
  //cout<<staticFields<<" to int "<<staticFieldsInt<<endl;
  int bigChecksum = (staticFieldsInt + (int)demo.queryID);

  //cout<<"added "<<(int)demo.queryID<<endl;


  bitset<16> cutoffChecksum = bitset<16>(bigChecksum);

  //if the int is greater than 65535, overflow occured.
  // resolve this by taking the mod of 65535 (1111111111111111) and adding 1 (000000000000001)
  bigChecksum = (bigChecksum > 65535) ? (int)cutoffChecksum.to_ulong() + 1: (int)cutoffChecksum.to_ulong();


  //cout<<"checksum "<<bigChecksum<<endl;
  //add the current sum with the packet's checksum (16bit int + 16bit int)
  bigChecksum = bigChecksum + (int)demo.checksum;
  cutoffChecksum = bitset<16>(bigChecksum);
  bigChecksum = (bigChecksum > 65535) ? (int)cutoffChecksum.to_ulong() + 1 : (int)cutoffChecksum.to_ulong();
  //cout<<"checksum "<<bigChecksum<<" "<<endl;
  //repeat this process with the data itself,
  //converting the chars in the array to their respective short values
  for(int x = 0; x < sizeOfData;x+=2){
    bitset<8> binary1 = (bitset<8>((unsigned short)demo.data[x]));
    bitset<8> binary2 = bitset<8>((unsigned short)demo.data[x+1]);
    bitset<16> trueBinary = (binary1.to_ulong() * 0x100 + binary2.to_ulong());
    int newNum = (int)(trueBinary.to_ulong());
    bigChecksum += (newNum);
    cutoffChecksum = bitset<16>(bigChecksum);
    bigChecksum = (bigChecksum > 65535) ? (int)cutoffChecksum.to_ulong() + 1 : (int)cutoffChecksum.to_ulong();
    //cout<<demo.data[x]<<" "<<demo.data[x+1]<<" "<<trueBinary<<" "<<newNum<<endl;
    //cout<<"checksum "<<bigChecksum<<endl;


  }
  //cout<<"checksum "<<bigChecksum<<endl;
  //run the one's compliment of the result and return
  unsigned short checksum = (unsigned short)bigChecksum;
  checksum = ~checksum;

  return checksum;

}
