#ifndef _LCD_H_
#define _LCD_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#ifndef LCD_MAJOR
#define LCD_MAJOR 0   /* dynamic major by default */
#endif

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


#define BASE_ADDRESS 0x378
#define ADDRESS_DATA (BASE_ADDRESS)
#define ADDRESS_STATUS (BASE_ADDRESS + 1)
#define ADDRESS_COMMAND (BASE_ADDRESS + 2)

#define BROCHE_E 0
#define BROCHE_RS 1


#define BROCHE_RESET 2
#define BROCHE_CLK 3 
#define BROCHE_DQ 1
#define BROCHE_ACK 6

#endif /* _LCD_H_ */
