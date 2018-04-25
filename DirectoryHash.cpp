
#include "WhoHeader.h"


int retreiveFileToSend(string fileName, struct S *songs){

  FILE* file = fopen(fileName.c_str(), "rb");
  const int bufSize = SONG_SIZE;
  char* buffer = (char*)malloc(bufSize);
  int bytesRead = 0;
  int temp = 0;
  while(bytesRead = fread(buffer,sizeof(char),bufSize,file)){temp = bytesRead;}
  bytesRead = temp;
  int offset = (int)songs->length * (SONG_SIZE + SONG_LENGTH_SIZE + SONG_HEADER_TITLE_SIZE);
  for(int x = SONG_LENGTH_SIZE-1; x >= 0; x--){
    songs->data[x + offset] = '0' + (bytesRead % 10);
    bytesRead = bytesRead/10;
  }


  bytesRead = temp;

  offset+= SONG_LENGTH_SIZE;

  for(int x = 0; x < SONG_HEADER_TITLE_SIZE; x++){
    if(x >= fileName.size()){
        songs->data[x + offset] = '\0';
    }else{
        songs->data[x + offset] = fileName.at(x);
    }
  }

  offset+= SONG_HEADER_TITLE_SIZE;

  for(int x = 0; x < bytesRead; x++)
      songs->data[x + offset] = buffer[x];

  songs->length = songs->length + 1;

  free(buffer);

}


int getdir(string dir, vector<string> &files){
  DIR *dp;
  struct dirent *dirp;
  if((dp  = opendir(dir.c_str())) == NULL) {
      cout << "Error(" << errno << ") opening " << dir << endl;
      return errno;
  }

  while ((dirp = readdir(dp)) != NULL) {

      if(string(dirp->d_name).find(".mp3")!= string::npos){

          files.push_back(string(dirp->d_name));

          char calc_hash[FILE_NAME_SIZE];
          calc_sha256(dirp->d_name, calc_hash);
          files.push_back(string(calc_hash));
          //cout<<"file "<<dirp->d_name<<endl;

      }

  }
  closedir(dp);
  return 0;
}


//caluclate the SHA256 hash of the file at the specificed path
int calc_sha256 (char* path, char output[65])
{
  FILE* file = fopen(path, "rb");
  if(!file) return -1;

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  const int bufSize = 10000000;
  char* buffer = (char*)malloc(bufSize);
  int bytesRead = 0;
  if(!buffer) return -1;
  while((bytesRead = fread(buffer, 1, bufSize, file)))
  {
      SHA256_Update(&sha256, buffer, bytesRead);
  }
  SHA256_Final(hash, &sha256);

  sha256_hash_string(hash, output);
  fclose(file);
  free(buffer);
  return 0;
}

void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65])
{
  int i = 0;

  for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
      sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }

  outputBuffer[64] = 0;
}
