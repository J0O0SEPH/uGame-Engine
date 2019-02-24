#define PE0					(*((volatile unsigned long *)0x40024004))
#define PE1					(*((volatile unsigned long *)0x40024008))
#define PB4					(*((volatile unsigned long *)0x40005040))
#define PB5					(*((volatile unsigned long *)0x40005080))

unsigned long newxpos;
unsigned long currentxpos;

void ADC0_Init(void);
unsigned long ADC0_In(void);
void joystick(const unsigned char *pt);
