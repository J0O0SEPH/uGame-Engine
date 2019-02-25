# Game Engine
>This concept project is an independent proposition, a part of a distance online course for embedded systems and microcontrollers. Code writing and designing process was guided by Dr. Jonathan W Valvano. Using the following references:

-   Introduction to the MSP432 Microcontroller-- ISBN-10: 1512185671.
-   Real-Time Interfacing to the MSP432 Microcontroller-- ISBN-10: 1514676583.
-   [https://courses.edx.org/courses/course-v1:UTAustinX+UT.6.20x+2T2018/course/](https://courses.edx.org/courses/course-v1:UTAustinX+UT.6.20x+2T2018/course/)

Requirements document is translated to:

 - ADC "12-bit": to interface a potentiometer detecting the joystick
   movements.
 - DAC "4-bit": to interface a headphone playing game's sound effects.
 - SSI: Serial port interfacing Nokia5110 LCD.
 - UART: Serial port Simulating Nokia5110 LCD as a stream through UART0
   ports.
 - LED: indicating life and another indicating special firing switch
 - Multi-level 80's style video game.

## Designing Considerations

 - Game was designed to occupy memory les than the limited maximum "32
   KB"
 - Interupt Latency should stick to the minimal for better visual
   effects.
 - Using various types of interrupts for demonstrative purposes.
 - Modular programming, for Concurrent product development.
 - The game to foloow basic gaming psychological effects (eg: keeping
   score, increasing difficulty).
 - Foregroud thread to be interrupted the least.

## Testing Procedure

Simulation SW "TExaS", used for debugging throughout the development phase. also Keil logic analyzer helped detect the DAC output signal. 
