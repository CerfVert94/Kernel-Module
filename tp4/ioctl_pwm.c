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
  unsigned short pwm_val;
  if (argc != 3)
    printf("Usage %s fichier duty \n",argv[0]);
  
  pwm_val=(unsigned int)atoi(argv[2]);
  t1=clock();
  fd = open(argv[1],O_RDWR);
  ioctl(fd,PWM_SET,pwm_val);
  close(fd);
  t2=clock();
  printf("set pwm 0x%4x -- time to exec : %.1f ns\n",pwm_val, (t2-t1)*1000000/CLOCKS_PER_SEC );
  
  return 0;
  
}
