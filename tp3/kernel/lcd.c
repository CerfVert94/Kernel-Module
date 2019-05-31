#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>      /* MKDEV,register_chrdev_region, alloc_chrdev_region,MAJOR */
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>        /* copy_*_user */
#include <asm/io.h>
#include <linux/delay.h>  /* udelay */


#include "lcd.h"

int port;

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

#define LCD_NR_PORTS	3
#define DEVICE_NAME "lcd"
MODULE_LICENSE("Dual BSD/GPL");

/*
 * all of the parameters have no "shortp_" prefix, to save typing when
 * specifying them at load time
 */
static int major = 124; /* dynamic by default */

/* default is the first printer port on PC's. "shortp_base" is there too
   because it's what we want to use in the code */
static unsigned long base = 0x378;
unsigned long shortp_base = 0;

unsigned char kernel_buf[100];

int read_count = 0;

int lcd_open(struct inode *inode, struct file *filp)
{
    if(MINOR(inode->i_rdev) != 0) {
        printk("lcd_c : mauvais numero d'unite\n");
        return -EINVAL;
    }
    printk(KERN_INFO "Y&R : LCD Open");
    tme_tempo(250);
    LCD_INIT();
    return 0;
	
}

int lcd_release(struct inode *inode, struct file *filp)
{
        printk(KERN_INFO "lcd: release\n");
        return 0;
}



ssize_t lcd_read(struct file *filp, char __user *user_buf, size_t count, loff_t *f_pos)
{
    int i = 0;
	int temp;
	char tempstring [16];


	DS1620_RESET_HIGH();
	DS1620_WRITECOMMAND(STARTCONVERT);
	DS1620_RESET_LOW();

    DS1620_RESET_HIGH();
	DS1620_WRITECOMMAND(READTEMP);
	temp=DS1620_READ();
	DS1620_RESET_LOW();
	tme_tempo(1000);
   
    temp /= 2;
    i = 15;
    tempstring[i--] = '\0';

    while(temp > 0) {
        tempstring[i--] = '0' + (temp % 10);
        temp /= 10;
    }
    if(i < 0)
        i = 0;

	LCD_CLEAR();
	LCD_STRING(&tempstring[i + 1]);

    if (!read_count) {
        read_count++;
        return 16 - i;
    }
    read_count = 0;
	return 0;
}

ssize_t lcd_write(struct file *filp, const char __user *user_buf, size_t count,
                loff_t *f_pos)
{
    int i = 0; 
	char tempstring [16];

	printk(KERN_INFO "Y&R: Write");
    tme_tempo(250);

	LCD_CLEAR();
    
    count = (count > 16 ? 16 : count);


	for (i = 0; i < count; i++)
        tempstring[i] = user_buf[i];

	LCD_STRING(tempstring);

    LCD_CMD(0x40);
    LCD_CMD(0b00100);
    LCD_CMD(0b01010);
    LCD_CMD(0b10001);
    LCD_CMD(0b01010);
    LCD_CMD(0b00100);
    LCD_CMD(0b00100);
    LCD_CMD(0b00100);
    LCD_CMD(0b00100);
	// if count>16, truncate
	// copy_from_user(kernel_buf, user_buf, count)
	// modify kernel_buf to insert \0
	// LCD_STRING(...)
    return count + 1;
}

struct file_operations lcd_fops = {
       .owner = THIS_MODULE,
       .read = lcd_read,
       .write = lcd_write,
       .open = lcd_open,
       .release = lcd_release
};

static int lcd_init(void)
{
	shortp_base = base;
    
    int err = 0;
	printk(KERN_ALERT "lcd:lcd_init\n");
	printk(KERN_ALERT "lcd:basePortAddress is 0x%lx\n", shortp_base);

	// Register the device
    if ((err = register_chrdev(major,DEVICE_NAME,&lcd_fops)) != 0) 
    {
	    pr_alert("*** unable to get major OSK_MAJOR for osk devices ***\n");
        return major;
    }


    printk(KERN_INFO "Y&R: registered correctly with major number %d\n", major);
  
//    PORT_INIT();
    

	return 0;
}

static void lcd_exit(void)
{
	// Unregister the device
    unregister_chrdev(major, DEVICE_NAME);             // unregister the major numbe
	printk(KERN_ALERT "lcd:lcd_exit\n");
}

module_init(lcd_init);
module_exit(lcd_exit);




void tme_tempo(int d)
{
    udelay(d * 1000);
}

void PORT_INIT(void)
{
  // Q3) En utilisant l'appel systeme ioperm(), permettre au processus d'avoir
  // acces aux ports commencant a l'adresse 0x378.
    port = BASE_ADDRESS;
//	ioperm(port, 3, 1);
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
     printk(KERN_INFO "Y&R: Write = %c", pdata[i]);
//     printk("%c",pdata[i]);
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
	int i = 0, data; 

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
