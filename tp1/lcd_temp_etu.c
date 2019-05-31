#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/io.h>

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

int port;

void tme_tempo(int d);                   //delay function
void PORT_INIT(void);                    //parallel port initialization
void LCD_E_HIGH(void);
void LCD_E_LOW(void);
void LCD_RS_HIGH(void);
void LCD_RS_LOW(void);
void LCD_DATA(unsigned char data);       //write to port // data  reg
void LCD_CMD(unsigned char data);        //write LCD command register
void LCD_CHAR(unsigned char data);       //write LCD command register
void LCD_STRING(unsigned char *string);  //write string to LCD
void LCD_INIT(void);                     //LCD initialization
void LCD_CLEAR(void);                    //clear LCD and home cursor
void LCD_HOME(void);                     //home cursor on LCD

void DS1620_RESET_HIGH(void);
void DS1620_RESET_LOW(void);
void DS1620_CLK_HIGH(void);
void DS1620_CLK_LOW(void);
void DS1620_DATA_HIGH(void);
void DS1620_DATA_LOW(void);
void DS1620_WRITECOMMAND(unsigned char command); //Write command to DS1620
int DS1620_READ(void);                          //Read data from DS1620

// Q1) Definir l'adresse de base des registres du port // 
#define BASE_ADDRESS 0x378
#define ADDRESS_DATA (BASE_ADDRESS)
#define ADDRESS_STATUS (BASE_ADDRESS + 1)
#define ADDRESS_COMMAND (BASE_ADDRESS + 2)

#define BROCHE_E 0
#define BROCHE_RS 1


#define BROCHE_RESET 2
#define BROCHE_CLK 3 //0b1000
#define BROCHE_DQ 1
#define BROCHE_ACK 6// 0b1000000

// Q2) Ecrire la fonction tem_tempo() qui permet d'attendre 'tempo' millisecondes
//     a l'aide de l'appel systeme usleep

void tme_tempo(int d)
{
    usleep(d * 1000);
}

void PORT_INIT(void)
{
  // Q3) En utilisant l'appel systeme ioperm(), permettre au processus d'avoir
  // acces aux ports commencant a l'adresse 0x378.
    port = BASE_ADDRESS;
	ioperm(port, 3, 1);
  // Q4) Initialiser le registre de commande de la maniere suivante
  //     -> STROBE à 0 (broche 1, bit 0 de 0x37A)
  //     -> AUTO à 0 (broche 14, bit 1 de 0x37A)
  //     -> INIT à 0 (broche 16, bit 2 de 0x37A)
  //     -> SLCTIN à 1 (broche 17, bit 3 de 0x37A)
    port = ADDRESS_COMMAND;
    outb((unsigned char)3, port); // 0b0011
	
	tme_tempo(10);
	

}

// Q5) Ecrire la fonction LCD_E_HIGH() qui permet de mettre E a 1

void LCD_E_HIGH(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) & (~(1 << BROCHE_E)), port);
}

// Q6) Ecrire la fonction LCD_E_LOW() qui permet de mettre E a 0

void LCD_E_LOW(void)
{

    port = ADDRESS_COMMAND;
    outb(inb(port) | (1 << BROCHE_E), port);
}

// Q7) Ecrire la fonction LCD_RS_HIGH() qui permet de mettre RS a 1

void LCD_RS_HIGH(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) & (~(1 << BROCHE_RS)), port);
}

// Q8) Ecrire la fonction LCD_RS_LOW() qui permet de mettre RS a 0

void LCD_RS_LOW(void)
{

    port = ADDRESS_COMMAND;
    outb(inb(port) | 1 << BROCHE_RS, port);
}

// Q9) Ecrire la fonction LCD_DATA() qui permet de mettre une valeur
//     dans le registre DATA du port //

void LCD_DATA (unsigned char data) {
  /* Positionne les données sur les entrées D0 à D7 du LCD
     On écrit le caractère dans le registre 0x378 */
    port = ADDRESS_DATA;
    outb(data, port);
    tme_tempo(1);
}

// Q9) Ecrire la fonction LCD_CMD() qui permet d'envoyer une commande
//     vers le port //. Ne pas oublier d'inserer des tempos de 1ms
//     pour generer le pulse sur E

void LCD_CMD(unsigned char data)
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
    tme_tempo(1);

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
    outb(inb(port) | 1 << BROCHE_RESET, port);
}

// Q16) Ecrire la fonction DS1620_RESET_LOW() qui permet 
// de mettre RESET a 0 pour le thermometre

void DS1620_RESET_LOW(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) & (~(1 << BROCHE_RESET)), port);
}

// Q17) Ecrire la fonction DS1620_CLK_HIGH() qui permet 
// de mettre CLK a 1 pour le thermometre

void DS1620_CLK_HIGH(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) & (~(1 << BROCHE_CLK)), port);
}

// Q18) Ecrire la fonction DS1620_CLK_LOW() qui permet 
// de mettre CLK a 0 pour le thermometre

void DS1620_CLK_LOW(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) | 1 <<  BROCHE_CLK, port);
}

// Q19) Ecrire la fonction DS1620_DQ_HIGH() qui permet 
// de mettre DQ a 1 pour le thermometre quand celui ci est en entree

void DS1620_DQ_HIGH(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) & (~(1 << BROCHE_DQ)), port);
}

// Q20) Ecrire la fonction DS1620_DQ_LOW() qui permet 
// de mettre DQ a 0 pour le thermometre quand celui ci est en entree

void DS1620_DQ_LOW(void)
{
    port = ADDRESS_COMMAND;
    outb(inb(port) | 1 <<  BROCHE_DQ, port);
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
        if(inb(port) & (1 << BROCHE_ACK))
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
	char tempstring [16];

	tme_tempo(250);
	PORT_INIT();
	LCD_INIT();

	LCD_CLEAR();

	
	LCD_STRING("Buenos dias");
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
		sprintf(tempstring,"%1.1f",temp/2.0);
		LCD_CLEAR();
		LCD_STRING(tempstring);
	}


	return 0;
}
