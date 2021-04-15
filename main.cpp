#include <inttypes.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>


#define SETPAGEADDRESS 0x22
#define SETCOLUMNADDRESS 0x21

#define StepRight 0xFE
#define StepLeft 0xFD
#define Rotate 0xFB
#define HardDrop 0xF7


    int sideways;
	int x;
	int y;
	int lowestchangedx =200;
	int highestchangedx = 0;
	int lowestchangedy = 200;
	int highestchangedy = 0;
	int delay = 300;
	bool harddrop = false;

	int Currentmodel[5][2] = {};

int shipModel[5][2] ={
	{4,0},{0,0},{-4,0},{0,4}
};
int stickModel[5][2] ={
	{-4,0},{0,0},{4,0},{8,0}
};
int LModel[5][2] ={
	{4,0},{0,0},{8,0},{0,4}
};
int cubeModel[5][2] ={
	{0,0},{4,0},{0,-4},{4,-4}
};
int ZModel[5][2] ={
	{0,0},{-4,0},{0,-4},{4,-4}
};



static uint8_t frameBuffer[1024] = {};

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
void resetmaxlow(){
	lowestchangedy = 200;
	lowestchangedx = 200;
	highestchangedy = 0;
	highestchangedx = 0;
}


void DisplayBuffer(){
	setWriteRange(0,7,0,127);
	for (int i = 0; i < 1024; i++)
	{
		spi_writedata(frameBuffer[i]);
	}
	
   
   }

 void togglePixel(int x, int y){
	 
   int page = (x/8);
   int bitindex = (x%8);
   uint8_t data = 1;
   for (int i = 0; i < bitindex; i++)
   {
	   data= data*2;
   }  
      if (x < lowestchangedx)
	  {
		  lowestchangedx = x;
	  }
	  if (x > highestchangedx)
	  {
		  highestchangedx = x;
	  }
	  if (y < lowestchangedy)
	  {
		  lowestchangedy = y;
	  }
	  if (y> highestchangedy)
	  {
		 highestchangedy = y;
	  }	  	  	  
	  frameBuffer[(page*128)+y] = (frameBuffer[(page*128)+y]^data); 
	  
}

 bool checkPixel(int x, int y){
   int page = (x/8);
   int bitindex = (x%8);
   uint8_t data = 1;
   for (int i = 0; i < bitindex; i++)
   {
	   data= data*2;
   }
   
   if (data&frameBuffer[(page*128)+y])
   {
	   return true;
   }
   else
   {
	  return false;
   }  
}


void drawBorder(){
	for (size_t i = 0; i < 128; i++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			togglePixel(k,i);
			togglePixel(60+k,i);

		}
	}
			for (size_t i = 0; i < 56; i++)
		{
			for (size_t k = 0; k < 6; k++)
			{
				togglePixel(4+i,122+k);
				togglePixel(4+i,0+k);
			}
	
		}
	

	

}

bool drawblock(int x, int y){

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t k = 0; k < 4; k++)
		{
                   togglePixel(x+i,y+k);
		}
		
	}
	return false;
}
bool checkblock(int x, int y){

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t k = 0; k < 4; k++)
		{
                   if(checkPixel(x+i,y+k)){
					   return true;
				   }
		}
		
	}
	return false;
}
void clearblock(int x, int y){

	for (size_t i = 0; i < 4; i++)
	{
		for (size_t k = 0; k < 4; k++)
		{
                  togglePixel(x+i,y+k);	
		}
		
	}
}
void updatesection(){
	setWriteRange((lowestchangedx/8),(highestchangedx/8),lowestchangedy,highestchangedy);
	for (size_t i = (lowestchangedx/8); i <= (highestchangedx/8); i++)
	{
		for (size_t k = lowestchangedy; k <= highestchangedy; k++)
		{
			spi_writedata(frameBuffer[i*128+k]);
		}
		
	}
	resetmaxlow();

}
//returns true if next move is collision
void drawobject(int x ,int y,bool draw,int model[4][2]){
	if (draw)
	{
		
			for (size_t i = 0; i < 4; i++)
			{
				drawblock(x+model[i][0],y+model[i][1]);
			}					
	}else
	{
			for (size_t i = 0; i < 4; i++)
			{
				clearblock(x+model[i][0],y+model[i][1]);
			}
	}

				
	
}

bool checkcollisonobject(int x ,int y,int model[4][2]){

		
			for (size_t i = 0; i < 4; i++)
			{
				if (checkblock(x+model[i][0],y+model[i][1]))
				{
					return true;
				}
				
			}	
			return false;				
	

}
void newFall(){
   	x =28;
	y =110;
	//harddrop fÃ¶rbÃ¤ttras
	harddrop = false;
	bool collision = 0;
	while(!collision){
	if (!checkcollisonobject(x,y,Currentmodel))
	{
	        
		
		if(sideways != 0){
			if (sideways+x > 123 || sideways+x <4)
			{
				sideways= 0;
			}
			
			x= x+sideways;
		drawobject(x,y,true,Currentmodel);
		if (!harddrop)
		{
			updatesection();
		    _delay_ms(300);
		}
		
		

		drawobject(x,y,false,Currentmodel);		 	  
		y-=4;
		sideways = 0;
		}
		else
		{
			
		drawobject(x,y,true,Currentmodel);
					if (!harddrop)
		{
		updatesection();
		_delay_ms(300);
		}
		drawobject(x,y,false,Currentmodel);		 	  
		y-=4;
		}
		


		
	     
		
	}else
	{
		drawobject(x,y+4,true,Currentmodel);
		collision = true;
		updatesection();
	}
	
	}
}
void copyArrayTo(int Src[5][2],int Dst[5][2]){
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t k = 0; k < 2; k++)
		{
			Dst[i][k] = Src[i][k];
		}
		
		
	}
	

}
void gameLoop(){
	while(1){
		
		//spawn object in top middle
		switch (rand() % 5)
		{
		case 0:
		copyArrayTo(shipModel,Currentmodel);
			break;
		case 1:
		copyArrayTo(stickModel,Currentmodel);
			break;
		case 2:
		copyArrayTo(LModel,Currentmodel);
			break;
		case 3:
		copyArrayTo(cubeModel,Currentmodel);
			break;
		case 4:
		copyArrayTo(ZModel,Currentmodel);
			break;
		
		}
		newFall();

	
	}
	//make object fall untill collision


}

	
ISR (PCINT0_vect){
	_delay_ms(10); 
	switch (PINA)
	{
	case StepRight:
		sideways = 4;

		break;
	case StepLeft:
	    sideways = -4;
		/* code */
		break;
	case Rotate:
		/* code */
		break;
	case HardDrop:
		harddrop = true;
		break;
	}

}

   
int main()
 { 
    
    PORTB |= (1 << PB2);
    PORTB &= ~(1 << PB2);
    PORTB |= (1 << PB2);
	
    
    spi_init();
	DDRA = 0x00;
	PORTA = 0xFF;
	DDRA = 0x00;
        EICRA |= (1<<ISC10);
    	PCICR |= (1<<PCIE0);
        PCMSK0 =0xFF;
   

    	_delay_ms(50);	
	sei();
    spi_writeCmd(0x20);
    spi_writeCmd(0x00);
    
    
   
    spi_writeCmd(0xAF);

	drawBorder();
	updatesection();
	gameLoop();
	



	

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
 
