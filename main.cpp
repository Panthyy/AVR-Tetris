#include <inttypes.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>
#include <string.h>
#include <stdio.h>


#define SETPAGEADDRESS 0x22
#define SETCOLUMNADDRESS 0x21

static uint8_t frameBuffer[1024] = {};
 
 struct CurrentTetrisBlock{
	int x;
	int y;
	int rotation;
};


 void spi_writeCmd (uint8_t temp)
{      
        PORTB &= ~(1 << PB3);
	SPDR = temp;
	while(!(SPSR & (1<<SPIF)));
}
 void spi_writedata (uint8_t temp)
{      
        PORTB |= (1 << PB3);
        
	SPDR = temp;
	while(!(SPSR & (1<<SPIF)));
}


void setWriteRange(uint32_t xStart,uint32_t xEnd,uint32_t  yStart, uint32_t yEnd)
{
			 spi_writeCmd(SETPAGEADDRESS);
			 spi_writeCmd(xStart);
			 spi_writeCmd(xEnd);
   
			 spi_writeCmd(SETCOLUMNADDRESS);
			 spi_writeCmd(yStart);
			 spi_writeCmd(yEnd);
			
}

void spi_init (void)

{
   DDRB = 0xFF;
	SPCR |= (1<<SPE | 1<<MSTR );
}
void drawBorder(){
   for (size_t i = 0; i < 128; i++)
   {
      frameBuffer[i] = 0b00000001;
      frameBuffer[897+i] = 0b10000000;
   }
      for (size_t i = 1; i < 9; i++)
   {
      frameBuffer[126*i] = 0b11111111;
   }
     
    setWriteRange(0,7,0,0);
    for (int i = 0; i < 8; ++i)
    {
    spi_writedata(0xFF);
    }
    setWriteRange(0,7,127,126);
    for (int i = 0; i < 8; ++i)
    {
    spi_writedata(0xFF);
    }
    setWriteRange(0,1,0,127);
    spi_writedata(0xFF);
    for (int i = 0; i < 126; ++i)
    {
    spi_writedata(0x01);
    }
    
    setWriteRange(7,6,0,127);
    spi_writedata(0xFF);
    for (int i = 0; i < 126; ++i)
    {
    spi_writedata(0x80);
    }
    
   }
void gameLoop(){

}
void drawpixel(int x , int y){


   uint8_t count= 0;
   uint8_t data = 2;
   uint8_t temp = 0;
   setWriteRange(x/8,x/8,y,y);
        do
   {
     count++;
     x-=8;
      
   } while (x > 8);
   
   
      for (size_t i = 0; i < temp; i++)
   {
      data *=2;
   }       
    if (frameBuffer[count+1]&data)
     {
      //collision
     }else {
      uint8_t t = data^frameBuffer[count+1];
     spi_writedata(t);
       }
    

   
   
}
   
int main()
 { 
    
    PORTA = 0xFF;
    PORTB |= (1 << PB2);
    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB2);
    
    spi_init();
    spi_writeCmd(0x20);
    spi_writeCmd(0x00);
    //border write
    
    
    drawBorder();
    spi_writeCmd(0xAF);

    drawpixel(7,30);



    
    
    
    
    
    




       
    
 }
 
