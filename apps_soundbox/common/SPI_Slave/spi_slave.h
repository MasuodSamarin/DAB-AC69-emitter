#ifndef _SPI_SLAVE_H_
#define _SPI_SLAVE_H_

#include "typedef.h"

#if 1//
//SPI2A
#define SPI2_SET_GROUP()       do{JL_IOMAP->CON1 &= ~BIT(16);}while(0)
#define SPI2_SET_DATAIN()      do{JL_PORTB->DIR &= ~BIT(2);}while(0)
#define SPI2_SET_CLK()	       do{JL_PORTB->DIR &= ~BIT(0);}while(0)
#define SPI2_SET_DATAOUT()     do{JL_PORTB->DIR &= ~BIT(1);}while(0)

#else
//SPI2B
#define SPI2_SET_GROUP()       do{JL_IOMAP->CON1  |= BIT(16);}while(0)
#define SPI2_SET_DATAIN()      do{PORTC_DIR &= ~BIT(2);}while(0)
#define SPI2_SET_CLK()	       do{PORTC_DIR &= ~BIT(0);}while(0)
#define SPI2_SET_DATAOUT()     do{PORTC_DIR &= ~BIT(1);}while(0)

#endif

#define SPI_CS_OUT()	    do{JL_PORTB->DIR &= ~BIT(3);}while(0)//
#define SPI_CS_L()	        do{JL_PORTB->OUT &= ~BIT(3);}while(0)
#define SPI_CS_H()	        do{JL_PORTB->OUT |= BIT(3);}while(0)
#define SPI_CS_IN()	        do{JL_PORTB->DIR |= BIT(3);}while(0)//
#define SPI_CS_STA()	    (JL_PORTB->IN & BIT(3))

#endif/*_LCD_SPI_H_*/
