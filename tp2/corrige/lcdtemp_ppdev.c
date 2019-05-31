
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/io.h>

#include <fcntl.h>
#include <errno.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>


/******** DS1620 COMMAND DEFINITIONS ***************************************/

#define READTEMP 0xaa                //Read Temperature
#define READCOUNTER 0xa0             //Read Counter register
#define READSLOPE 0xa9               //Read Slope counter register
#define STARTCONVERT 0xee            //Start temperature conversion
#define STOPCONVERT 0x22             //Stop temperature conversion

#define WRITETH  0x01                //Write to TH register
#define WRITETL  0x02                //Write to TL register
#define READTH   0xa1                //Read TH register
#define READTL   0xa2                //Read TL register
#define WRITECONFIG 0x0c             //Write to Configuration register
#define READCONFIG 0xac              //Read Configuration register

void tme_tempo(int d);                   //delay function
void PORT_INIT(void);                    //parallel port initialization
void LCD_INIT(void);                     //LCD initialization
void LCD_CMD(unsigned char data);        //write to LCD command register
void LCD_DATA(unsigned char data);       //write to LCD data register
void LCD_STRING(unsigned char *string);  //write string to LCD
void LCD_CLEAR(void);                    //clear LCD and home cursor
void LCD_HOME(void);                     //home cursor on LCD
void LCD_LINE2(void);                    //move cursor to line 2, position 1
void LCD_RS_HIGH(void);
void LCD_RS_LOW(void);
void LCD_E_HIGH(void);
void LCD_E_LOW(void);

void DS1620_CONFIG(unsigned char data);          //DS1620 configuration
void DS1620_WRITECOMMAND(unsigned char command); //Write command to DS1620
void DS1620_WRITEDATA(int data);                 //Write data to DS1620
int DS1620_READ(void);                          //Read data from DS1620
void DS1620_RESET_HIGH(void);
void DS1620_RESET_LOW(void);
void DS1620_CLK_HIGH(void);
void DS1620_CLK_LOW(void);
void DS1620_DATA_HIGH(void);
void DS1620_DATA_LOW(void);
int powerof2(int);

int port=0x378;
int fd;

void myoutb(char v,unsigned int adr)
{
	switch (adr)
	{
		case 0x378:
			ioctl(fd,PPWDATA,&v);
			break;
		case 0x37A:
			ioctl(fd,PPWCONTROL,&v);
			break;
	}
	//outb(v,adr);
}

char myinb(unsigned int adr)
{
	char b;
	switch (adr)
	{
		case 0x378:
			ioctl(fd,PPRDATA,&b);
			break;
		case 0x379:
			ioctl(fd,PPRSTATUS,&b);
			break;
		case 0x37A:
			ioctl(fd,PPRCONTROL,&b);
			break;
	}
	return b;

	//return inb(adr);
}

/* delay function */

void tme_tempo(int d)
{
	usleep(d*1000);
}

/* Initialize parallel port pins */

void PORT_INIT(void)
{
/*
  	if (ioperm(port,3,1))
	{
		printf("ioperm error\n");
		exit(1);
	}
*/
  	myoutb(0x03,port+2);     //SLCTIN high = DS1620 CLK high
                             //INIT low = DS1620 reset low
                             //AUTO low = LCD RS low
                             //STROBE low = LCD Enable low
  	tme_tempo(10);
}

/* Initialize LCD - 8 bit interface, 2 lines, 5x7 dots */

void LCD_INIT(void)
{
  	LCD_CMD(0x38);              //8 bit interface, 2 lines, 5x7 dots
  	LCD_CMD(0x38);              //do it 3 times
  	LCD_CMD(0x38);
  	LCD_CMD(0x0c);              //display on, cursor off, cursor not blinking
  	LCD_CMD(0x06);              //move cursor with data write
  	LCD_CLEAR();                //clear display and home cursor
}

/* Write 8-bit data to LCD command register */

void LCD_CMD(unsigned char data)
{
  	myoutb(data,port);
  	tme_tempo(1);
  	LCD_RS_LOW();
  	tme_tempo(1);
  	LCD_E_HIGH();
  	tme_tempo(1);
  	LCD_E_LOW();
  	tme_tempo(1);
}

/* Write 8-bit data to LCD data register */

void LCD_DATA(unsigned char data)
{
  	myoutb(data,port);
  	tme_tempo(1);
  	LCD_RS_HIGH();
  	tme_tempo(1);
  	LCD_E_HIGH();
  	tme_tempo(1);
  	LCD_E_LOW();
  	tme_tempo(1);
}

/* Write string to LCD */

void LCD_STRING(unsigned char *string)
{
  	int i;

  	for (i=0; i<strlen(string); i++) 
		LCD_DATA(string[i]);
}

/* Clear LCD and home cursor */

void LCD_CLEAR(void)
{
  	LCD_CMD(0x01);                 //LCD Clear Display command
  	tme_tempo(2);                     //wait for command to execute
}

/* Home cursor on LCD */

void LCD_HOME(void)
{
  	LCD_CMD(0x02);                 //LCD Home Cursor command
  	tme_tempo(2);                     //wait for command to execute
}

/* Write 8-bit command to DS1620 */

void DS1620_WRITECOMMAND(unsigned char command)
{
  	int i;

  	tme_tempo(2*1);

  	for (i=0; i<8; i++)         //write 8-bit command byte to chip
    	{
      		DS1620_CLK_LOW();
      		if (command&0x01)
			DS1620_DATA_HIGH();
      		else 
			DS1620_DATA_LOW();
      		tme_tempo(2*1);
      		DS1620_CLK_HIGH();
      		tme_tempo(2*1);
      		command=command>>1;     //ready to write next bit
    	}
}

/* Read 9 bits of data from DS1620 chip */

int DS1620_READ(void)
{
  	int i;
  	unsigned int data,temp;
  	char car;

  	DS1620_DATA_HIGH();           //setup to read from data pin
  	tme_tempo(2*1);
  	data=0;                     //initialize data byte

  	for (i=0; i<9; i++)
    	{
      		DS1620_CLK_LOW();
      		tme_tempo(2*1);
		car=myinb(port+1);
      		if (car&0x40)
			data+=1<<i;  //if bit is high, add its value to total
      		DS1620_CLK_HIGH();
      		tme_tempo(2*1);
    	}

  	tme_tempo(2*1);
  	return data;
}

/* LCD RS input high */

void LCD_RS_HIGH(void)
{
  myoutb(myinb(port+2)&0xfd,port+2);
}

/* LCD RS input low */

void LCD_RS_LOW(void)
{
  myoutb(myinb(port+2)|0x02,port+2);
}

/* LCD ENABLE input high */

void LCD_E_HIGH(void)
{
  myoutb(myinb(port+2)&0xfe,port+2);
}

/* LCD ENABLE input low */

void LCD_E_LOW(void)
{
  myoutb(myinb(port+2)|0x01,port+2);
}

/* DS1620 RESET input high */

void DS1620_RESET_HIGH(void)
{
  myoutb(myinb(port+2)|0x04,port+2);
}

/* Set DS1620 RESET input low */

void DS1620_RESET_LOW(void)
{
  myoutb(myinb(port+2)&0xfb,port+2);
}

/* Set DS1620 CLK input high */

void DS1620_CLK_HIGH(void)
{
  myoutb(myinb(port+2)&0xf7,port+2);
}

/* Set DS1620 CLK input low */

void DS1620_CLK_LOW(void)
{
  myoutb(myinb(port+2)|0x08,port+2);
}

/* Set DS1620 DATA input high */

void DS1620_DATA_HIGH(void)
{
  myoutb(myinb(port+2)&0xfd,port+2);
}

/* Set DS1620 DATA input low */

void DS1620_DATA_LOW(void)
{
  myoutb(myinb(port+2)|0x02,port+2);
}

int main(int argc,char *argv[])
{
	int temp;
	float temp1;
	char tempstring [16];
	char res;

	tme_tempo(250);
	fd=open("/dev/parport0", O_RDWR);
	ioctl(fd,PPCLAIM);
	PORT_INIT();
	LCD_INIT();

	LCD_STRING("BUENOS DIAS");
	tme_tempo(1000);

	DS1620_RESET_HIGH();
	DS1620_WRITECOMMAND(STARTCONVERT);
	DS1620_RESET_LOW();
	tme_tempo(1000);

	while (1)
	{
		DS1620_RESET_HIGH();
		DS1620_WRITECOMMAND(READTEMP);
		temp=DS1620_READ();
		DS1620_RESET_LOW();
		tme_tempo(1000);
		if (temp&0x100)
		{
			temp=~temp+1;
			temp=temp&0xff;
			temp=temp*(-1);
		}
		temp1=temp/2.0;
		sprintf(tempstring,"%1.1f",temp1);
		LCD_CLEAR();
		LCD_STRING(tempstring);
	}
	ioctl(fd,PPRELEASE);
	close(fd);

	return 0;
}
