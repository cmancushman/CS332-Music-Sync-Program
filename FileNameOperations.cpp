#include "WhoHeader.h"


void formatFileNames(char** names, int numberOfUsernames, char* buffer){

  for(int x = 0; x < numberOfUsernames; x++){

    //separate usernames and their respective login times by strtok on the colon
    char* name = strtok(names[x], ":");
    int sizeOfName = strlen(name);//length of the name in characters

    //print 8 bytes to our buffer, if the name is not that long,
    //add null characters to the rest
    for(int y = 0; y <FILE_NAME_SIZE; y++){
      buffer[y + (x*FILE_NAME_SIZE)] = name[y];
      if(y >= sizeOfName) buffer[y + (x*FILE_NAME_SIZE)] = '\0';
    }

  }

}
