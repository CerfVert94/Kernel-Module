
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


#define BROCHE_E 0
#define BROCHE_RS 1


#define BROCHE_RESET 2
#define BROCHE_CLK 3 //0b1000
#define BROCHE_DQ 1
#define BROCHE_ACK 6// 0b1000000
#define BASE_ADDRESS 0x378
#define ADDRESS_DATA (BASE_ADDRESS)
#define ADDRESS_STATUS (BASE_ADDRESS + 1)
#define ADDRESS_COMMAND (BASE_ADDRESS + 2)



int port;
int fd; // File descriptor (global variable) 

void tme_tempo(int d);                   //delay function
void my_open();                          // open function
void my_close();                         // close function
void my_outb(char byte, int addr);                          // write function
char my_inb(int addr);                          // read function
void PORT_INIT(void);                    //parallel port initialization

void LCD_E_HIGH(void);
void LCD_E_LOW(void);
void LCD_DATA (unsigned char data);
void LCD_RS_HIGH(void);
void LCD_RS_LOW(void);
void LCD_CMD(char data);        //write LCD command register
void LCD_CHAR(unsigned char data);       //write LCD command register
void LCD_STRING(unsigned char *string);  //write string to LCD
void LCD_INIT(void);                     //LCD initialization
void LCD_CLEAR(void);                    //clear LCD and home cursor
void LCD_HOME(void);                     //home cursor on LCD


/* delay function */

void tme_tempo(int d)
{
	usleep(d*1000);
}
/* open function */
void my_open()
{
    fd = open("/dev/parport0", O_RDWR);
    if (fd == -1) {
        printf("erreur : ouverture du  peripherique port parallele\n");
        exit(1);
    }
    if (ioctl(fd, PPCLAIM)) {
        printf("erreur : reclamation de l'usage exclusif du peripherique port parallele\n");
        exit(1);
    }
}

/* close function */
void my_close()
{
    if (ioctl(fd, PPRELEASE) == -1) {
        printf("erreur : liberation de l'usage exclusif du peripherique port parallele\n");
        exit(1);
    }
    close(fd);
}
/* write function */
void my_outb(char byte, int addr)
{
    switch (addr) {
        case ADDRESS_DATA :
            addr = PPWDATA;
            break;
        case ADDRESS_COMMAND :
            addr = PPWCONTROL;
            break;
    }

    if (ioctl(fd, addr, &byte) == -1)
    {
        printf("erreur : ecriture\n");
        exit(1);
    }
}
/* read function */
char my_inb(int addr)
{
    char byte = 0;
    switch (addr) {
        case ADDRESS_DATA :
            addr = PPRDATA;
            break;
        case ADDRESS_STATUS :
            addr = PPRSTATUS;
            break;
        case ADDRESS_COMMAND :
            addr = PPRCONTROL;
            break;
    }
    
    if (ioctl(fd, addr, &byte) == -1)
    {
        printf("erreur : lecture\n");
        exit(1);
    }
    return byte;
}

void PORT_INIT(void)
{
    port = ADDRESS_COMMAND;
    my_outb(0x03, port); // 0b0011
	tme_tempo(10);
}

void LCD_E_HIGH(void) 
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) & (~(1 << BROCHE_E)), port);
}
void LCD_E_LOW(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) | (1 << BROCHE_E), port);
}


void LCD_DATA (unsigned char data) {
  /* Positionne les données sur les entrées D0 à D7 du LCD
     On écrit le caractère dans le registre 0x378 */
    port = ADDRESS_DATA;
    my_outb(data, port);
    tme_tempo(1);
}


void LCD_RS_HIGH(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) & (~(1 << BROCHE_RS)), port);
}

void LCD_RS_LOW(void)
{
    port = ADDRESS_COMMAND;

    my_outb(my_inb(port) | (1 << BROCHE_RS), port);
}

void LCD_CMD(char data)
{
    LCD_DATA(data);
    LCD_RS_LOW();
    tme_tempo(1);

    LCD_E_HIGH();
    tme_tempo(1);

    LCD_E_LOW();
    tme_tempo(1);

}

// Q10) Ecrire la fonction LCD_CHAR() qui permet d'envoyer un caractere
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CHAR(unsigned char data)
{
    LCD_DATA(data);
    LCD_RS_HIGH();
    tme_tempo(1);

    LCD_E_HIGH();
    tme_tempo(1);

    LCD_E_LOW();
    tme_tempo(1);
}

// Q11) Ecrire la fonction LCD_STRING() qui permet d'envoyer une
// chaine de caracteres vers le LCD

void LCD_STRING(unsigned char* pdata) {
  /* Affiche une chaîne de caractères sur l'écran */
   int i = 0;
   while(pdata[i] != '\0') {
        printf("%c",pdata[i]);
        LCD_CHAR(pdata[i++]);
    }
    printf("\n");
}

// Q12) Ecrire la fonction LCD_CLEAR() qui permet d'effacer l'ecran du LCD

void LCD_CLEAR(void)
{
    LCD_CMD(0x1);   
}

// Q13) Ecrire la fonction LCD_HOME() qui remet le curseur du LCD
// en premiere ligne, premiere colonne

void LCD_HOME(void)
{
    LCD_CMD(0x2);   
}

// Q14) Ecrire la fonction LCD_INIT() qui reinitialise le LCD
// La reinitialisation consiste a envoyer la sequence de commandes
// 0x38, 0x38, 0x38, 0x0C, 0x06

void LCD_INIT(void) {
    LCD_CMD(0x38);
    LCD_CMD(0x38);
    LCD_CMD(0x38);
    LCD_CMD(0x0C);
    LCD_CMD(0x06);
}



// Q15) Ecrire la fonction DS1620_RESET_HIGH() qui permet 
// de mettre RESET a 1 pour le thermometre

void DS1620_RESET_HIGH(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) | 1 << BROCHE_RESET, port);
}

// Q16) Ecrire la fonction DS1620_RESET_LOW() qui permet 
// de mettre RESET a 0 pour le thermometre

void DS1620_RESET_LOW(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) & (~(1 << BROCHE_RESET)), port);
}

// Q17) Ecrire la fonction DS1620_CLK_HIGH() qui permet 
// de mettre CLK a 1 pour le thermometre

void DS1620_CLK_HIGH(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) & (~(1 << BROCHE_CLK)), port);
}

// Q18) Ecrire la fonction DS1620_CLK_LOW() qui permet 
// de mettre CLK a 0 pour le thermometre

void DS1620_CLK_LOW(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) | 1 <<  BROCHE_CLK, port);
}

// Q19) Ecrire la fonction DS1620_DQ_HIGH() qui permet 
// de mettre DQ a 1 pour le thermometre quand celui ci est en entree

void DS1620_DQ_HIGH(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) & (~(1 << BROCHE_DQ)), port);
}

// Q20) Ecrire la fonction DS1620_DQ_LOW() qui permet 
// de mettre DQ a 0 pour le thermometre quand celui ci est en entree

void DS1620_DQ_LOW(void)
{
    port = ADDRESS_COMMAND;
    my_outb(my_inb(port) | 1 <<  BROCHE_DQ, port);
}

// Q21) Ecrire la fonction DS1620_WRITECOMMAND() qui permet 
// d'envoyer une commande sur 8 bits au thermometre.
// La commande s'envoie bit apres bit, en commencant par le
// bit de poids faible

void DS1620_WRITECOMMAND(unsigned char command)
{
    int i = 0; 

    tme_tempo(2);
    
    for (i = 0; i < 8; i++) {
        // Mettre le signal clk en etat bas.
        DS1620_CLK_LOW();
        tme_tempo(2);

        // Envoyer la commande
        if(command & 0x1)
            DS1620_DQ_HIGH();    
        else 
            DS1620_DQ_LOW();   
        
        tme_tempo(2);  

        // clk en etat haut = front montant
        DS1620_CLK_HIGH();
        tme_tempo(2);
        command >>= 1;
    }
}


// Q22) Ecrire la fonction DS1620_READ() qui permet 
// de lire une temperature sur 9 bits depuis le thermometre
// la temperature se lit bit apres bit en commencant par
// le bit de poids faible.

int DS1620_READ(void)
{
	int ret=0, i = 0, data; 

  	DS1620_DQ_HIGH();           //setup to read from data pin
  	tme_tempo(2*1);
  	data=0;                     //initialize data byte

    for (i = 0; i < 9; i++) {
        DS1620_CLK_LOW();
   		tme_tempo(2*1);


        port = ADDRESS_STATUS;    
        if(my_inb(port) & (1 << BROCHE_ACK))
			data += 1 << i;  //if bit is high, add its value to total

        DS1620_CLK_HIGH();
   		tme_tempo(2*1);
    }
 	tme_tempo(2*1);
  	return data;
}


int main(int argc,char *argv[])
{
	int temp;
	float temp1;
	char tempstring [16];

	tme_tempo(250);
    my_open();
	PORT_INIT();
	LCD_INIT();

	LCD_CLEAR();

	
	LCD_STRING("Y & R : Bonjour !");
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
    my_close();


	return 0;
}




