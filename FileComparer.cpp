#include "WhoHeader.h"

void compareFiles(vector<string> &files, int recvMsgSize, struct S *demo, bool both){

  //Create a list of all songs locally stored on the host
  //At the end of the for loop, localSongs  will contain all of the songs stored locally
  vector<Song> localSongs = vector<Song>();
  for(int x = 0; x < files.size();x+=2){
    Song song = {files[x],files[x+1]};
    localSongs.push_back(song);
  }

  int numberOfReceivedFiles = (recvMsgSize - CONSTANT_SIZE)/(FILE_NAME_SIZE);

  vector<Song> compareVector = vector<Song>();

  //Extract song titles and hashes from the receiveMessgae struct in this for loop
  //At the end, compareVector will contain a list of song titles and their data hashes
  for(int x = 0; x < numberOfReceivedFiles;x+=2)
  {
    string title = "";
    string hash = "";
    for(int y = 0; y < FILE_NAME_SIZE;y++){
      title+=demo->data[y + x * FILE_NAME_SIZE];
    }
    for(int y = FILE_NAME_SIZE; y < FILE_NAME_SIZE*2;y++){
      hash+=demo->data[y + x * FILE_NAME_SIZE];
    }
    Song song = {title, hash};
    compareVector.push_back(song);
  }

  //checks for incoming files that are not found in local files--excluded for syncing
  vector<Song> differenceSongs = vector<Song>();
  if(both){
    for(int x = 0; x < compareVector.size();x++){
      bool found = false;
      for(int y = 0; y < localSongs.size();y++){
        if(stringsAreTheSame(localSongs[y].hash, (compareVector[x].hash))){
          found = true;
          break;
        }
      }
      if(!found) differenceSongs.push_back(compareVector[x]);
    }
  }

 //finding local files that are not in the incoming files
  for(int x = 0; x < localSongs.size();x++){
    bool found = false;
    for(int y = 0; y < compareVector.size();y++){
      if(stringsAreTheSame(compareVector[y].hash, localSongs[x].hash)){
        found = true;
        break;
      }
    }
    if(!found) differenceSongs.push_back(localSongs[x]);
  }

  //add all our files that are not found to a vector and return that vector
  files = vector<string>();
  for(int x = 0; x < differenceSongs.size();x++){
    files.push_back(differenceSongs[x].title);
    files.push_back(differenceSongs[x].hash);
  }

}




//A helper function that compares 2 strings and returns true if theyre equivalent
bool stringsAreTheSame(string string1, string string2){

  for(int x = 0; x< min(string2.size(), string1.size());x++){

    if(string1.at(x) != string2.at(x)){
      return false;
    }

  }

  return true;

}
