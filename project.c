#include "projectHeader.h"


int main(int argc, char const *argv[]) {
  int f = ArgumentumCheck(argc, argv);

  if (f == -100) {
    return 1;
  }
  //printf("%d\n",f );
  //BrowseForOpen();
  alarm(1);
  int NumCh = 0;
  char* Pbuff = ReadPixels(f, &NumCh);
  signal(SIGALRM, f==0 ? SIG_DFL : WhatToDo);
  signal(SIGINT,  f==0 ? SIG_DFL : WhatToDo);
  if (NumCh == 0) {
    fprintf(stderr, "Nem talalta a kodolt karaktereket!\n");
    return 1;
  }
  char* msg = Unwrap(Pbuff,NumCh);
  alarm(0);
  //printf("%s\n",msg );

  char* nID = "TR795Z";
  //printf("most jon a post\n");
  int send = Post(nID, msg, NumCh);

  if (send == 0) {
    printf("Sikeres kuldes!\n" );
  }
  else{
    fprintf(stderr, "Sikertelen kuldes!\n");
    return 1;
  }
  return 0;
}
