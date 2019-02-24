// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the edX Lab 15
// In order for other students to play your game
// 1) You must leave the hardware configuration as defined
// 2) You must not add/remove any files from the project
// 3) You must add your code only this this C file
// I.e., if you wish to use code from sprite.c or sound.c, move that code in this file
// 4) It must compile with the 32k limit of the free Keil

// This virtual Nokia project only runs on the real board, this project cannot be simulated
// Instead of having a real Nokia, this driver sends Nokia
//   commands out the UART to TExaSdisplay
// The Nokia5110 is 48x84 black and white
// pixel LCD to display text, images, or other information.

// April 19, 2014
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Required Hardware I/O connections*******************
// PA1, PA0 UART0 connected to PC through USB cable
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5
	
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "Sound.h"
#include "TExaS.h"
#include "Main.h"
#include "ADC.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

unsigned long FrameCount=0;
unsigned long AnyLife = 1;
unsigned long TimerCount;
unsigned long Semaphore;
unsigned long Bullet;
unsigned long level;
unsigned long Vic;

typedef struct {
  unsigned long x;      // x coordinate
  unsigned long y;      // y coordinate
  const unsigned char *image[2]; // two pointers to images
  long life;            // 0=dead, 1=alive
	}STyp;         STyp Enemy[4];

typedef struct {
	unsigned char xpos;
	unsigned char ypos;
	const unsigned char *type;
}shots;				shots pos[100];

int main(void){ 
  TExaS_Init(NoLCD_NoScope);  // set system clock to 80 MHz
	SysTick_Init(80000000/30);	// you cannot use both the Scope and the virtual Nokia (both need UART0)
  Nokia5110_Init();
  Random_Init(1);
	init_switch();
	
	ADC0_Init();
  Sound_Init();
	EnableInterrupts(); // virtual Nokia uses UART0 interrupts
  
  Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();      // draw buffer

  Nokia5110_PrintBMP(32, 47, PlayerShip0, 0); // player ship middle bottom
  Nokia5110_PrintBMP(33, 47 - PLAYERH, Bunker0, 0);

  Nokia5110_PrintBMP(0, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(16, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(32, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(48, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_PrintBMP(64, ENEMY30H - 1, SmallEnemy30PointA, 0);
  Nokia5110_DisplayBuffer();   // draw buffer
	Sound_JohnCena();
	Delay100ms(5);

  if (AnyLife){Level();}
	
	if (AnyLife==0&&Vic>=3){AnyLife=1;level=1;Level();}
			
		
	if(AnyLife==0){PB4=0;PB5=0;
  Nokia5110_Clear();
  Nokia5110_SetCursor(1, 0);
	Nokia5110_OutString("YOU Scored:");
  Nokia5110_SetCursor(2, 1);
  Nokia5110_OutUDec(Vic*100);
  Nokia5110_SetCursor(0, 3);
  Nokia5110_OutString("ARE YOU NOT ENTERTAINED!");
	}
 }
void Level (void){
	Init();
  while(AnyLife){int s,i;
		PB4=0x10;
		if (TimerCount%20==0){PB5=0x20;}
		if (TimerCount%10==0){s=1;}
		
		if (PE0&&s){pos[Bullet].xpos=currentxpos+9;
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Shoot();s=0;}
		
		if (PE1&&PB5){pos[Bullet].xpos=currentxpos;
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Fastinvader1();
				
				pos[Bullet].xpos=currentxpos+18;
				pos[Bullet].ypos=40;pos[Bullet].type=Missile0;
				Bullet++;Sound_Shoot();PB5=0x00;
		}
		
		while(Semaphore == 0){}
		  Semaphore = 0; // runs at 30 Hz
			AnyLife = 0;
			for(i=0; i<4 ; i++){
      AnyLife |= Enemy[i].life;
		}
			Draw();
  }

}
void Init(void){ int i;
  for(i=0;i<4;i++){
    Enemy[i].x = 20*i;
    Enemy[i].y = 10;
    Enemy[i].image[0] = SmallEnemy30PointA;
    Enemy[i].image[1] = SmallEnemy30PointB;
    Enemy[i].life = 1;
   }
}

void Move(void){ int i;
  for(i=0;i<4;i++){
    if(Enemy[i].x < 72){
      Enemy[i].x += 1; // move to right
    }else{
      Enemy[i].life = 0;
    }
  }
}

void MoveDwn(void){ int i=(Random()>>24)%4;
  //for(i=0;i<4;i++){
    if(Enemy[i].y < 40){
      Enemy[i].y += 2; // move down
    }else{
      Enemy[i].life = 0;
    }
  }

void Draw(void){unsigned int i;
  Nokia5110_ClearBuffer();
  for(i=0;i<4;i++){
    if(Enemy[i].life > 0){
     Nokia5110_PrintBMP(Enemy[i].x, Enemy[i].y, Enemy[i].image[FrameCount], 0);}
  }
	if (level==0){joystick(PlayerShip0);Shoot();}
	else{joystick(PlayerShip0);Shoot1();}
  Nokia5110_DisplayBuffer();      // draw buffer
  FrameCount = (FrameCount+1)&0x01; // 0,1,0,1,...
}

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
// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
NVIC_ST_CTRL_R=0x0;
NVIC_ST_CURRENT_R=0x01;
NVIC_ST_RELOAD_R=period-1;
NVIC_SYS_PRI3_R|=0xFE000000;
NVIC_ST_CTRL_R=0x07;	
}

// executes every 25 ms, collects a sample, converts and stores in mailbox

void SysTick_Handler(void){
  TimerCount++;
  Semaphore = 1; // trigger
	newxpos=((ADC0_In()/62));
  if (level==0){Move();}
	else{MoveDwn();}
}

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

void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
