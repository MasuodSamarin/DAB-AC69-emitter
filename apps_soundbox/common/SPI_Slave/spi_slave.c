#include "sdk_cfg.h"
#include "spi_slave.h"
#include "irq_api.h"




#define SPI2_ISR_PRIORITY      3

void spi2_isr(void)
{
    u8 buf = JL_SPI2->BUF;
    JL_SPI2->CON |= (BIT(14));
    putchar('2');
    put_u8hex(buf);
}
IRQ_REGISTER(IRQ_SPI2_IDX, spi2_isr);

void SPI2_init(bool is3Wrie, u8 speed)
{
    IRQ_REQUEST(IRQ_SPI2_IDX, spi2_isr, SPI2_ISR_PRIORITY);
    JL_SPI2->BAUD = speed;

//    JL_SPI2->CON = 0x4021;//0100 0000 0010 0001
    JL_SPI2->CON  = 0x0000;//0100 0000 0010 0001
    JL_SPI2->CON |= (BIT(14)|BIT(5)|BIT(0));

    SPI2_SET_GROUP();
    SPI2_SET_CLK();
    SPI2_SET_DATAOUT();
    SPI2_SET_DATAIN();

    JL_SPI2->CON |= (BIT(13));

    puts("SPI2_INIT_OVER:JL_SPI2->CON:");
    put_u32hex(JL_SPI2->CON);
}

