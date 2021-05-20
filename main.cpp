#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
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
	bool gameOver = 0;
	int x,y;
	int score = 0;
	int lowestchangedx =200;
	int highestchangedx = 0;
	int lowestchangedy = 200;
	int highestchangedy = 0;
	int delay = 300;
	bool harddrop = false;
	bool rotate = false;
	double rotationdegrees = 0;
	int digitcounter = 0;
	int Currentmodel[4][2] = {};

// 2d arrays containing the block positions for the tetris blocks

int shipModel[4][2] ={
	{0,0},{4,0},{-4,0},{0,4}
};
int stickModel[4][2] ={
	{0,0},{-4,0},{4,0},{8,0}
};
int LModel[4][2] ={
	{0,0},{4,0},{8,0},{0,4}
};
int cubeModel[4][2] ={
	{0,0},{4,0},{0,-4},{4,-4}
};
int ZModel[4][2] ={
	{0,0},{-4,0},{0,-4},{4,-4}
};

// 2d arrays containing the positions of blocks to be able to display digits and letters
int G[15][2] ={  // 5x4
	{1,0},{2,0},{3,0},
	{0,-1},{0,-2},{0,-3},{0,-4},
	{0,-4},{1,-4},{2,-4},{3,-4},
	{3,-4},{3,-3},{3,-2},{2,-2}	
};
int A[12][2] ={  // 5x4
	{1,0},{2,0},{1,-1},
	{2,-1},{3,-2},{0,-2},{0,-3},{0,-4},
	{1,-3},{2,-3},{3,-3},{3,-4},
};
int M[11][2] ={  // 5x4
	{1,0},{3,0},{1,-1},
	{3,-1},{0,-2},{2,-2},{4,-2},
	{0,-3},{4,-3},{4,-4},{0,-4}
};
int O[10][2] ={  // 5x4
	{1,0},{2,0},{0,-1},
	{3,-1},{0,-2},{3,-2},{0,-3},
	{3,-3},{1,-4},{2,-4}
};
int V[9][2] ={  // 5x4
	{0,0},{0,-1},{0,-2},
	{0,-3},{1,-4},{2,-3},{2,-2},
	{2,-1},{2,0}
};
int E[10][2] ={  // 5x4
	{0,0},{1,0},{2,0},
	{0,-1},{0,-2},{1,-2},{0,-3},
	{0,-4},{1,-4},{2,-4}
};
int R[7][2] ={  // 5x4
	{0,0},{0,-1},{0,-2},
	{0,-3},{0,-4},{1,-1},{2,0}
};

int one[4][2] ={  // 5x4
	{0,0},{0,-1},{0,-2},
	{0,-3}
};

int two[9][2] ={  // 5x4
	{1,0},{2,0},{0,-1},
	{3,-1},{2,-2},{0,-3},{1,-3},
	{2,-3},{3,-3}
};

int three[10][2] ={  // 5x4
	{0,0},{1,0},{2,0},
	{1,-1},{2,-1},{1,-2},{2,-2},
	{0,-3},{1,-3},{2,-3}
};

int four[8][2] ={  // 5x4
	{0,0},{0,-1},{0,-2},
	{1,-2},{2,-2},{2,0},{2,-1},
	{2,-3}
};

int five[8][2] ={  // 5x4
	{0,0},{1,0},{2,0},
	{0,-1},{1,-1},{2,-2},{0,-3},
	{1,-3}
};
int six[9][2] ={  // 5x4
	{1,0},{2,0},{0,-1},
	{0,-2},{0,-3},{1,-2},{2,-2},
	{1,-3},{2,-3}
};
int seven[6][2] ={  // 5x4
	{0,0},{1,0},{2,0},
	{2,-1},{1,-2},{0,-3}
};
int eight[7][2] ={  // 5x4
	{1,0},{0,-1},{2,-1},
	{1,-2},{0,-3},{2,-3},{1,-4}
};
int nine[7][2] ={  // 5x4
	{1,0},{0,-1},{2,-1},
	{1,-2},{2,-2},{2,-3},{2,-4}
};

// framebuffer used to buffer the frames and pull position data
static uint8_t frameBuffer[1024] = {};

// 4 wire spi functions used to be able to communicate with the ssd1306 display driver using the onboard spi handling.
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
// functions to set the write range for the display driver
void setWriteRange(uint32_t xStart,uint32_t xEnd,uint32_t  yStart, uint32_t yEnd)
{
			 spi_writeCmd(SETPAGEADDRESS);
			 spi_writeCmd(xStart);
			 spi_writeCmd(xEnd);
   
			 spi_writeCmd(SETCOLUMNADDRESS);
			 spi_writeCmd(yStart);
			 spi_writeCmd(yEnd);
			
}
//function used to copy a array to another one
void copyArrayTo(int Src[5][2],int Dst[5][2]){
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t k = 0; k < 2; k++)
		{
			Dst[i][k] = Src[i][k];
		}		
	}
}
// initiates the spi interface.
void spi_init (void)
{
   DDRB = 0xFF;
	SPCR |= (1<<SPE | 1<<MSTR );
}
//resets the max and low position values keep to figure out the changed section that needs to be updated

void resetmaxlow(){
	lowestchangedy = 200;
	lowestchangedx = 200;
	highestchangedy = 0;
	highestchangedx = 0;
}
//toggle a Pixel on or off, records the position of the changed pixel and updated the frame buffer

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

//checks if a pixel in the frame buffer is on or off

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
//draws the game border

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

//draws a 4x4 block made up of pixels

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
// figures out the section of recent changeds and sends the section to be updated on the display
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

void displaydigit (int digit[7][2],int rows){
	for (size_t i = 0; i < rows; i++)
	{
		drawblock((48-digitcounter*20)+4*digit[i][0],72+4*digit[i][1]);
	}
	digitcounter++;	
}
// displays the gameover screen containing the text "gam over" and the score
void gameover(){
	setWriteRange(0,7,0,127);
	for (size_t i = 0; i < 1024; i++)
	{
		frameBuffer[i] = 0;
		spi_writedata(0x00);
		
	}
		for (size_t i = 0; i < 15; i++)
	{
		drawblock(0+4*G[i][0] ,120+4*G[i][1]);
	}
		for (size_t i = 0; i < 12; i++)
	{
		drawblock(20+4*A[i][0],120+4*A[i][1]);
	}
	
	for (size_t i = 0; i < 11; i++)
	{
		drawblock(40+4*M[i][0],120+4*M[i][1]);
	}
	
	for (size_t i = 0; i < 10; i++)
	{
		drawblock(0+4*O[i][0],96+4*O[i][1]);
	}

	for (size_t i = 0; i < 9; i++)
	{
		drawblock(20+4*V[i][0],96+4*V[i][1]);
	}
	
	for (size_t i = 0; i < 10; i++)
	{
		drawblock(36+4*E[i][0],96+4*E[i][1]);
	}
	for (size_t i = 0; i < 7; i++)
	{
		drawblock(52+4*R[i][0],96+4*R[i][1]);
	}
	   while (score) {
		switch (score%10)
		{
		case 0:
			displaydigit(O,10);
			break;
		case 1:
			displaydigit(one,4);
			break;
		case 2:
			displaydigit(two,9);
			break;
		case 3:
			displaydigit(three,10);
			break;
		case 4:
			displaydigit(four,8);
			break;
		case 5:
			displaydigit(five,8);
			break;
		case 6:
			displaydigit(six,9);
			break;
		case 7:
			displaydigit(seven,6);
			break;
		case 8:
			displaydigit(eight,7);
			break;
		case 9:
			displaydigit(nine,7);
			break;
		}
        score /= 10;    
    }
}

//draws a tetris block/object with the parameters of position and if it should draw or remove
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

bool checkcollisonobject(int x ,int y,int model[4][2])
{
	for (size_t i = 0; i < 4; i++)
	{
		if (checkblock(x+model[i][0],y+model[i][1]))
		{
			return true;
	        }
				
	}	
	return false;				
}

void clearRow(int row){
	for (size_t i = 0; i < 14; i++)
	{
		clearblock(4+4*i,6+4*row);
	}
}

void moveRowDown(int row){
	for (size_t i = 0; i < 14; i++)
	{
		if (checkblock(4+4*i,6+4*row))
		{
			clearblock(4+4*i,6+4*row);
			drawblock(4+4*i,(6+4*row)-4);
		}		
	}	
}

bool IsRowFull(int row){
	for (size_t i = 0; i < 14; i++)
	{
		if (!checkblock(4+4*i,6+4*row))
		{
			return false;
		}		
	}
	return true;	
}

void checkAndClearFullRow(int rowstart,int rowend){
	for (size_t i = rowstart; i < rowend; i++)
	{
		if (IsRowFull(i))
		{
			clearRow(i);
			updatesection();
			for (size_t k = i; k < 29; k++)
			{
				moveRowDown(k);
				updatesection();	
			}
			i-=1;			
		}	
	}	
}

void rotateCurrentModel(){
	int temparray[4][2] = {};
	copyArrayTo(Currentmodel,temparray);
	for (size_t i = 0; i < 4; i++)
	{				
		int tempx=temparray[i][0];
		int tempy=temparray[i][1];
		temparray[i][0]= roundf((tempx*cos((rotationdegrees * M_PI) / 180)) - tempy*sin((rotationdegrees * M_PI) / 180));
		temparray[i][1]=roundf(tempx*sin((rotationdegrees * M_PI) / 180)+ tempy*cos((rotationdegrees * M_PI) / 180));
	}

	if (!checkcollisonobject(x,y,temparray))
	{
		copyArrayTo(temparray,Currentmodel);
	}			
}

void rotate(){
	drawobject(x,y,false,Currentmodel);	
	rotateCurrentModel();
	drawobject(x,y,true,Currentmodel);
	updatesection();				
	rotate = false;
}

void movesideways(){
	drawobject(x,y,false,Currentmodel);	
	x= x+sideways;
	drawobject(x,y,true,Currentmodel);
	if (sideways+highestchangedx < 64 &&  sideways+lowestchangedx >-4  )
	{
	updatesection();				
	} else
	{
	drawobject(x,y,false,Currentmodel);	
	x= x-sideways;
	drawobject(x,y,true,Currentmodel);
	}
	sideways = 0;
}
//function used to instantiate a while loop for a  a new tetris block that falls until collision
void newFall(){
	rotationdegrees = 0;
   	x =28;
	y =114; 
	harddrop = false;
	bool collision = 0;
	while(!collision){
	if (!checkcollisonobject(x,y,Currentmodel))
	{
		drawobject(x,y,true,Currentmodel);
		updatesection();
		for (size_t i = 0; i < 30; i++)
		{
			if (harddrop)
			{
				goto Drop;
			}
			 
			if (rotate)
			{
				rotate();
			}
			
			if(sideways != 0)
			{
				movesideways();
			}		
			_delay_ms(10); 
		 }
		 Drop:
		//if harddrop button is pressed make block fall down fast.
		//possible improvements to harddrop
		 if (harddrop)
		 {
			drawobject(x,y,false,Currentmodel);
			do
			{
				y-=4;
			} while (!checkcollisonobject(x,y,Currentmodel));			
			harddrop = false;
		 }
		
		 else
		 {
			drawobject(x,y,false,Currentmodel); 	  
		 	y-=4;
		 }		
	}else
	{
		//collision happend, draw block at last position and continue to next fall
		drawobject(x,y+4,true,Currentmodel);
		collision = true;
		score += 1;
		updatesection();
		
		//checking if blocks exist on the top if so end game
			for (size_t i = 0; i < 14; i++)
		{
			if (checkblock(4+4*i,114))
			{
			   gameOver = true;

			}
		
		}
		//checking if row where block is placed if full if so clear it
		if (y < 16)
		{
			checkAndClearFullRow(0,(y/4));
		}else
		{
			checkAndClearFullRow((y/4)-4,(y/4));
		}

		updatesection();	
	}
	}
}

void gameLoop(){	
	while(!gameOver){
		//spawn object in top middle
		switch (rand() % 5)
		{
		case 0:
		copyArrayTo(cubeModel,Currentmodel);
			break;
		case 1:
		copyArrayTo(stickModel,Currentmodel);
			break;
		case 2:
		copyArrayTo(LModel,Currentmodel);
			break;
		case 3:
		copyArrayTo(ZModel,Currentmodel);
			break;
		case 4:
		copyArrayTo(shipModel,Currentmodel);
			break;
		}
		newFall();
	}
	gameover();

}

/*
handling of the interrupts, interrupts are used as variable changing functions to avoid clashing with the draw and data transfer.
interrupts change variables depending on what button was pressed and that is used to change the tetris object through a for loop
checking if a variable has changed or not. this was used to avoid bugs concerning interrupts happenining in the process of data transfer
or object updates.
*/	
ISR (PCINT0_vect){
	_delay_ms(10); 
	switch (PINA)
	{
	case StepRight:
		sideways = 4;
		break;
	case StepLeft:
	    sideways = -4;
		break;
	case Rotate:
		rotate = true;
		rotationdegrees +=90;
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
	updatesection();
 }
