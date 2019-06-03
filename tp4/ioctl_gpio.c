#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#define PWM_SET 0

int main(int argc,char *argv[])
{
  int fd;
  double t1,t2;
  unsigned short gpio_val;
  if (argc != 3)
    printf("Usage %s fichier output \n",argv[0]);

  t1=clock();  
  gpio_val=(unsigned int)atoi(argv[2]);
  fd = open(argv[1],O_RDWR);
  ioctl(fd,gpio_val);
  close(fd);
  t2=clock();
  printf("set gpio 0x%4x -- time to exec : %.1f ns\n",gpio_val, (t2-t1)*1000000/CLOCKS_PER_SEC );
  
  return 0;
  
}
