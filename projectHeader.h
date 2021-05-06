#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

//

#define BUFSIZE 1024
#define IP_ADRESS "193.6.135.162"
#define PORT_NO 80

//
char* VERSION =  "Verzio: 1.0\nDatum: 2020.04.24\nSzerzo: Rigo Gyorgy Janos";
char* HELP = "A program futtatasahoz meg kell adni parameterkent a kivant fajt dekodolasat.Ha nem ad meg parametert kikeresheti a mappakbol. A '--version' kapcsoloval lekerheti a program verzioszamat, keszitesenek idejet és a szerzo nevet!";

int BrowseForOpen(){

  DIR *dp;
  struct dirent *e;
  char* currentDir;
  char route[100];
  char browseFileName[50];
  int f;
  chdir(getenv("HOME"));
  dp = opendir(".");
  while(1){
    getcwd(route, sizeof(route));
    printf("%s\n", route);

    while( (e=readdir(dp)) != NULL ){
      printf("%s\n",e->d_name );
    }

    printf("Fajl vagy Konyvtar nev:\n");
    scanf("%s",browseFileName);

    if (strstr(browseFileName,".bmp") == 0) {

      chdir(browseFileName);
      closedir(dp);
      dp = opendir(".");
    }else{
      f = open(browseFileName,O_RDONLY);
      closedir(dp);
      break;
    }
  }
  return f;
}

int ArgumentumCheck(int argc, char const *argv[]){
  int f = 0;
  if (argc == 1) {
    f = BrowseForOpen();
    return f;
  }
  if (argc == 2) {
    char* version = "--version";
    if (strcmp(argv[1],version) == 0) {
      printf("%s\n", VERSION);
      return -100;
    }
    char* help = "--help";
    if (strcmp(argv[1],help) == 0) {
      printf("%s\n", HELP);
      return -100;
    }
  }
  else{
    fprintf(stderr, "Parameter hiba!\n");
    exit(2);
  }

  if(argc == 2 && strstr(argv[1],".bmp")){
    f = open(argv[1], O_RDONLY);
    if( f == -1){
      fprintf(stderr, "Nem talalhato a megadott fajl.\n");
      exit(3);
    }
  }
    else{
      fprintf(stderr, "Csak '.bmp' kiterjesztesu fajlokat lehet megadni\n");
      exit(3);
  }
  return f;

}


char* Unwrap(char* Pbuff, int NumCh){
  char* result = (char*)malloc(NumCh);
  if(result == NULL){
    fprintf(stderr, "Nincs eleg memoria!\n");
    exit(1);
  }

  #pragma omp parallel for schedule(guided)
  for(int i = 0; i < NumCh; i++){
    result[i] = (Pbuff[i*3]) << 6 | (Pbuff[(i*3)+1] & 7) << 3 | Pbuff[(i*3)+2] & 7;
    //printf("%c\n",result[i]);
  }

  free(Pbuff);
  return result;
}

char* ReadPixels(int f, int* NumCh){
  int num;
  lseek(f,6,SEEK_SET);
  int headerSize;
  read(f, &num, 4);

  char* pixels = (char*)malloc(num * 3);
  if(pixels == NULL){
    fprintf(stderr, "Nincs eleg memoria!\n");
    exit(1);
  }
  read(f, &headerSize, 4);
  lseek(f,headerSize, SEEK_SET);
  read(f,pixels,num * 3);
  close(f);
  *NumCh = num;
  return pixels;

}

int Post( char *neptunID, char *message, int NumCh ){
  int s;
  int flag;
  int bytes;
  int err;
  unsigned int server_size;
  char on;
  char buffer[BUFSIZE];
  struct sockaddr_in server;

  on = 1;
  flag = 0;
  server.sin_family =         AF_INET;
  server.sin_addr.s_addr =    inet_addr(IP_ADRESS);
  server.sin_port =           htons(PORT_NO);
  server_size =               sizeof(server);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if ( s < 0 )
  {
      fprintf(stderr, " Socket hiba.\n");
      return 11;
  }

  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
  setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

  err = connect( s, (struct sockaddr *) &server, server_size);
  if ( err < 0 )
  {
      fprintf(stderr, " Kapcsolodasi hiba.\n");
      return 12;
  }

  char* contLen;

  sprintf(contLen, "%d", (NumCh + 27)); // 27 = len of these : 'NeptunID=' ,neptunID, '&PostedText='
  strcpy(buffer, "POST /~vargai/post.php HTTP/1.1\r\nHost: irh.inf.unideb.hu\r\nContent-Length: ");
  strcat(buffer, contLen);
  strcat(buffer, "\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n");
  strcat(buffer, "NeptunID=");
  strcat(buffer, neptunID);
  strcat(buffer, "&PostedText=");
  strcat(buffer, message);

  free(message);

  bytes = send(s, buffer, strlen(buffer) + 1, flag);
  if (bytes = 0) {
    fprintf(stderr, "Kuldes hiba!\n");
    return 13;
  }
  bytes = recv(s, buffer, BUFSIZE, flag);
  if (bytes < 0) {
    fprintf(stderr, "Fogadasi hiba!\n");
    return 14;
  }
  close(s);
  if (strstr(buffer,"The message has been received.")) {
    return 0;
  }
  else{
    fprintf(stderr, "Nincs engedely!\n");
    return 15;
  }

}

void WhatToDo(int sig)
{
  if(sig == SIGALRM)
  {
    fprintf(stderr, "Idokorlat tullepes!");
    exit(1);
  }
  else if(sig == SIGINT)
  {
    pid_t pid;
    pid = fork();
    if(pid == 0)
    {
      printf("Nem megszakithato utasitas!.\n");
      raise(SIGKILL);
    }
  }
}
