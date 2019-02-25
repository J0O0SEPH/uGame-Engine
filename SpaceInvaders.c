/*  
	SpaceInvaders.c
	Runs on LM4F120/TM4C123

 ******* Required Hardware I/O connections*******************
 PA1, PA0 UART0 connected to PC through USB cable
 Slide pot pin 1 connected to ground
 Slide pot pin 2 connected to PE2/AIN1
 Slide pot pin 3 connected to +3.3V 
 fire button connected to PE0
 special weapon fire button connected to PE1
 8*R resistor DAC bit 0 on PB0 (least significant bit)
 4*R resistor DAC bit 1 on PB1
 2*R resistor DAC bit 2 on PB2
 1*R resistor DAC bit 3 on PB3 (most significant bit)
 LED on PB4
 LED on PB5  */
	
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "Sound.h"
#include "TExaS.h"
#include "Main.h"
#include "ADC.h"


void DisableInterrupts(void);
void EnableInterrupts(void);

unsigned long FrameCount=0;
unsigned long AnyLife = 1;
unsigned long TimerCount;
unsigned long Semaphore;
unsigned long Bullet;
unsigned long level;
unsigned long Vic;

/* Structure holding informations of Enemy sprites 
(X&Y Positions, BMP array, and rendering decision)
 */
typedef struct {
  unsigned long x;      			 // x coordinate
  unsigned long y;      			 // y coordinate
  const unsigned char *image[2]; 	 // two pointers to images
  long life;           				 // 0=dead, 1=alive
	}STyp;         STyp Enemy[4];

/* Structure holding shots information 
(x&Y positions, BMP array) 
 */
typedef struct {
	unsigned char xpos;
	unsigned char ypos;
	const unsigned char *type;		//pointer to BMP array
}shots;				shots pos[100]; //Maximum number of shots

/* The Main thread will initiate [PLL, SysTick, 
Random Number Generator(Linear congruential generator),
DAC, ADC, Fire Switches]

LCD initiated with opening frame of enemies and fighting ship.

Level are added on the base of number of Enemy sprites shot.

Score showing screen will appear after finishing all level,
or fail to achieve target to pass to next level.
 */

int main(void){ 
  TExaS_Init(NoLCD_NoScope); 	   // Activate Phase Lock Loop (PLL) @ 80 MHz
  SysTick_Init(80000000/30);	   // SysTick timer initiate interrupts @ 30 Hz
  Nokia5110_Init();				   // Intiate Nokia LCD, Conditional Compiling used to to simulate LCD
  Random_Init(1);				   // Random numbers are used to determine Enemy Sprites moves
  init_switch();				   // Firing Switches polled @ 30 Hz
  ADC0_Init();					   // 12 bit ADC interfaced with Potentiometer to determine the fighting ship position
  Sound_Init();					   // Initiates 4-bit DAC, Sound is played @ 11 KHz
  EnableInterrupts(); 			   // virtual Nokia uses UART0 interrupts
  
  Nokia5110_ClearBuffer();
  Nokia5110_DisplayBuffer();       // draw buffer

  Nokia5110_PrintBMP(32, 47, PlayerShip0, 0);     // Append vlaues into Screen buffer to be rendered using fn DisplayBuffer
  Nokia5110_PrintBMP(33, 47 - PLAYERH, Bunker0, 0);

  Nokia5110_PrintBMP(0, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(16, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(32, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(48, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(64, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_DisplayBuffer();        // draw buffer
  Sound_JohnCena();
  Delay100ms(5);

  if (AnyLife){Level();}		    // First level 
								    // Conditions could be set to add or remove levels
									// Levels difficulty are determined within fn Draw
  if (AnyLife==0&&Vic>=3){AnyLife=1;level=1;Level();} 
				
  if(AnyLife==0){PB4=0;PB5=0;		// Game over screen displays scored points *100
  Nokia5110_Clear();
  Nokia5110_SetCursor(1, 0);
  Nokia5110_OutString("YOU Scored:");
  Nokia5110_SetCursor(2, 1);
  Nokia5110_OutUDec(Vic*100);
  Nokia5110_SetCursor(0, 3);
  Nokia5110_OutString("ARE YOU NOT ENTERTAINED!");
	}
 }
 
/* 
When called, Renders the screen with 4 Enemy Sprites, 
difficulty determined in fn Draw. 
append bullets into struct "shot", when it's corresponding switch is pressed.

A semaphore triggered by Systick timer @ 30 Hz, check life of Enemy's sprite
 */
void Level (void){
	Init();							// 4 Enemy sprites to be added in Enemy array
  while(AnyLife){int s,i;			
		PB4=0x10;					// LED are set to indicate life
		if (TimerCount%20==0){PB5=0x20;}	// Two flags are set to control shooting rate.
		if (TimerCount%10==0){s=1;}			// One for each switch
		
		if (PE0&&s){pos[Bullet].xpos=currentxpos+9;		// When SW0 is pressed one shot is fired
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Shoot();s=0;}
		
		if (PE1&&PB5){pos[Bullet].xpos=currentxpos;		// When SW1 is pressed two shots are fired
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Fastinvader1();
				
				pos[Bullet].xpos=currentxpos+18;
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Shoot();PB5=0x00;
		}
		
		while(Semaphore == 0){}		// A semaphore polls the life of Enemy's Sprites
		  Semaphore = 0; 			// runs at 30 Hz
			AnyLife = 0;
			for(i=0; i<4 ; i++){
				AnyLife |= Enemy[i].life;
			}
			Draw();					// Data in Buffers and Arrays to be diplayed using this function
  }

}

/* 
When called the information for 4 enemy Sprites to be placed in the screen buffer
 */
void Init(void){ int i;
  for(i=0;i<4;i++){
    Enemy[i].x = 20*i;
    Enemy[i].y = 10;
    Enemy[i].image[0] = SmallEnemy30PointA;
    Enemy[i].image[1] = SmallEnemy30PointB;
    Enemy[i].life = 1;
   }
}

/* 
The function rules the moves of Sprite in Level one
A new function to be added for each level required.
 */

void Move(void){ int i;
  for(i=0;i<4;i++){
    if(Enemy[i].x < 72){
      Enemy[i].x += 1; 		// move to right
    }else{
      Enemy[i].life = 0;
    }
  }
}


/* 
Designed for Sprites in level 2 to move down in random order
Random number generator is called to determine the Sprite which jumps down 2 positions
 */
void MoveDwn(void){ int i=(Random()>>24)%4;		//Random Choice
  //for(i=0;i<4;i++){
    if(Enemy[i].y < 40){
      Enemy[i].y += 2; 							// move down
    }else{
      Enemy[i].life = 0;
    }
  }
/*
Called Periodically @ 30 Hz, 
displays the updated screen buffer according the satisfied condition
 */
void Draw(void){unsigned int i;
  Nokia5110_ClearBuffer();
  for(i=0;i<4;i++){
    if(Enemy[i].life > 0){
     Nokia5110_PrintBMP(Enemy[i].x, Enemy[i].y, Enemy[i].image[FrameCount], 0);}
  }
	if (level==0){joystick(PlayerShip0);Shoot();}		// when satisfied initiates ADC (joystick) to update 
	else{joystick(PlayerShip0);Shoot1();}				// the fighting ship postions coming from potentiometer
  Nokia5110_DisplayBuffer();      						// and calls (shoot) to detect collision with shots
  FrameCount = (FrameCount+1)&0x01; // 0,1,0,1,...		
}
/* 
	Initiates input from fire switches
 */
void init_switch(void){volatile unsigned long delay;
SYSCTL_RCGC2_R|=SYSCTL_RCGC2_GPIOE;
delay=SYSCTL_RCGC2_R;
GPIO_PORTE_AFSEL_R&=~0x03;
GPIO_PORTE_AMSEL_R&=~0x03;
GPIO_PORTE_DEN_R|=0x03;
GPIO_PORTE_DIR_R&=~0x03;
GPIO_PORTE_PCTL_R&=~0x000000FF;
GPIO_PORTE_PDR_R|=0x03;
}
/* 
Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
 */
void SysTick_Init(unsigned long period){
NVIC_ST_CTRL_R=0x0;
NVIC_ST_CURRENT_R=0x01;
NVIC_ST_RELOAD_R=period-1;
NVIC_SYS_PRI3_R|=0xFE000000;
NVIC_ST_CTRL_R=0x07;	
}

/* 
executes every 25 ms, collects a sample, converts and stores in mailbox
 */
void SysTick_Handler(void){
  TimerCount++;
  Semaphore = 1; 				// trigger
	newxpos=((ADC0_In()/62));	// Translates the values fetched from the ADC into Distance into the LCD x-axis
  if (level==0){Move();}		// Determines Sprites movements
	else{MoveDwn();}
}

/* 
Decision to change enemy's life is made according to the satisfied condition
X&Y Positions of shots&enemy sprites are cheched if intersect or not
*/
void Shoot(void){unsigned int i,u;
		for(u=0;u<4;u++){
			for(i=0;i<Bullet;i++){if (Enemy[u].x<=pos[i].xpos&&Enemy[u].x+20>=pos[i].xpos&&pos[i].ypos<8&&pos[i].ypos>0&&Enemy[u].life>0){
				Enemy[u].life=0;
				Nokia5110_PrintBMP(Enemy[u].x,Enemy[u].y,SmallExplosion0, 0);Vic++;Sound_Explosion();				
				}else{
				Nokia5110_PrintBMP(pos[i].xpos,pos[i].ypos,pos[i].type, 0);
				if(pos[i].ypos>0){pos[i].ypos--;}
			}
		}
	}
}
/*  
one function to be added for each level, to detect collision if movements are similar
function could be reused
if no collision, bullets y-position is changed to create the animation 
*/
void Shoot1(void){unsigned int i,u;
		for(u=0;u<4;u++){
			for(i=0;i<Bullet;i++)
			{if (Enemy[u].life>0&&Enemy[u].x+18>=pos[i].xpos&&
				Enemy[u].x<=pos[i].xpos&&pos[i].ypos>0&&Enemy[u].y>=pos[i].ypos-9)
				{
				Enemy[u].life=0;
				Nokia5110_PrintBMP(Enemy[u].x,Enemy[u].y,SmallExplosion0, 0);Vic++;Sound_Explosion();				
				}else{
				Nokia5110_PrintBMP(pos[i].xpos,pos[i].ypos,pos[i].type, 0);
				if(pos[i].ypos>0){pos[i].ypos--;}
			}
		}
	}
}
/* 
input is multiplied by 100 msec
*/
void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
