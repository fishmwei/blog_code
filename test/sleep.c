
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int max_length = 128;

char * generate(int length){
  int i;
  char * buffer = (char*) malloc (length+1);
  if (buffer == NULL)
    return NULL;
  for (i=0; i<length; i++){
    buffer[i]=rand()%26+'a';
  }
  buffer[length]='\0';
  return buffer;
}

int main(int argc, char *argv[])
{
  int num;
  char * buffer;

  printf ("Input the string length : ");
  scanf ("%d", &num);

  if(num > max_length){
    num = max_length;
  }

  buffer = generate(num);

  printf ("Random string is: %s\n",buffer);

  sleep(100);

  free (buffer);

  return 0;
}