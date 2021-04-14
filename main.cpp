#include <inttypes.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>


#define SETPAGEADDRESS 0x22
#define SETCOLUMNADDRESS 0x21
#define SCREENLENGTHMAX 116
#define SCREENLENGTHMIN 29
bool StickModel[13] ={
	0,0,0,0,
};
static uint8_t frameBuffer[1024] = {};



 
ISR (PCINT0_vect){
		switch (PINA)
	{
		//check what pin changed/pressed;
		case 0x01:
		/* Your code here */
		break;
		case 0x02:
		break;
		case 0x04:
		break;
		case 0x08:
		break;
		case 0x10:
		break;
		default:
		/* Your code here */
		break;
	}



}
 struct CurrentTetrisBlock{
	int x;
	int y;
	int rotation;
};

void positionsToData(int x,int y){
   //example with x and y = 0


}
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

void gameLoop(){

}
void DisplayBuffer(){
	setWriteRange(0,7,0,127);
	for (int i = 0; i < 1024; i++)
	{
		spi_writedata(frameBuffer[i]);
	}
	
   
   }

void drawPixel(int x, int y){
   int page = (x/8);
   int bitindex = (x%8);
   uint8_t data = 1;
   for (int i = 0; i < bitindex; i++)
   {
	   data= data*2;
   }
   
   if (data&frameBuffer[(page*128)+y])
   {
	  //collision
   }else
   {
	  frameBuffer[(page*128)+y] = (frameBuffer[(page*128)+y]^data);
   }
   

   
}
void drawBorder(){
 for (int i = 0; i < 127; i++)
 {
	 frameBuffer[i] = 0x07; 
 }
  for (int i = 897; i < 1023; i++)
 {
	 frameBuffer[i] = 0xE0; 
 }
 for (size_t i = 0; i < 8; i++)
 {
	 frameBuffer[128*i] = 0xFF;
	 frameBuffer[128*i+1] = 0xFF;
	 frameBuffer[128*i+2] = 0xFF;
	 frameBuffer[128*i+3] = 0xFF;
	 frameBuffer[128*i+4] = 0xFF;
	 frameBuffer[128*i+5] = 0xFF;
 }
  for (size_t i = 0; i < 9; i++)
 {
	 frameBuffer[128*i] = 0xFF;
	 frameBuffer[128*i-1] = 0xFF;
	 frameBuffer[128*i-2] = 0xFF;
	 frameBuffer[128*i-3] = 0xFF;
	 frameBuffer[128*i-4] = 0xFF;
	 frameBuffer[128*i-5] = 0xFF;
 }

}

void drawblock(int x, int y){

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t k = 0; k < 4; k++)
		{
          drawPixel(x+i,y+k);	
		}
		
	}
	


}
void drawship(int x, int y){
		drawblock(x+4,y);
		drawblock(x,y);
		drawblock(x,y+4);
		drawblock(x-4,y);
}

   
int main()
 { 
    
    PORTB |= (1 << PB2);
    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB2);
    
    spi_init();
    
    
    DDRC = 0xFF;
    PORTC = 0xFF;
    PCICR = 0x01;
    PCMSK0 = 0x1F;
	_delay_ms(50);
	sei();
    spi_writeCmd(0x20);
    spi_writeCmd(0x00);
    
    
   
    spi_writeCmd(0xAF);
	drawBorder();
	for (size_t i = 3; i < 61; i++)
	{
		drawPixel(i,64);
	}
		for (size_t i = 6; i < 64; i++)
	{
		drawPixel(32,i);
	}

	//push tetris Block into
		for (size_t i = 0; i < 4; i++)
	{
		frameBuffer[128*2+85+i] = frameBuffer[128*2+85+i]^0x78;
		frameBuffer[128*2+81+i] = frameBuffer[128*2+81+i]^0xF8;
		frameBuffer[128*3+81+i] = (frameBuffer[128*3+81+i]^0x3F);
	}		

// 	  Single(0x00 + ((x + 2) & 0x0F));        // Column low nibble
//   Single(0x10 + ((x + 2)>>4));            // Column high nibble
//   Single(0xB0 + (y >> 3));                // Page
		DisplayBuffer();
		_delay_ms(100);	
		
drawship(31,110);
		DisplayBuffer();
	// // remove	
	// 	for (size_t i = 0; i < 4; i++)
	// {
	// 	frameBuffer[128*3+85+i] = 0x00;
	// 	frameBuffer[128*3+81+i] = 0x00;
	// 	frameBuffer[128*4+81+i] = 0x00;
	// }

	// 	DisplayBuffer();
	// 	_delay_ms(100);	
	// 	for (size_t i = 0; i < 4; i++)
	// {
	// 	frameBuffer[128*3+85-3+i] = 0xF0;
	// 	frameBuffer[128*3+81-3+i] = 0xF0;
	// 	frameBuffer[128*4+81-3+i] = 0xFF;
	// }
	// DisplayBuffer();
	
	
	
        
     
      


     
    

   
    
         
 }
 
