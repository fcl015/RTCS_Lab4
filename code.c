/*
 * code.c
 *
 *  Created on: 26/08/2014
 *      Author: L01073411
 */


// Include libraries
#include "ee.h"		   //Cabecera
#include "ee_irq.h"	   //Cabecera
#define FCY 40000000UL //Frecuencia de trabajo
#include <libpic30.h>  //Libreria incluida


// Global variables
unsigned char status_task1=0;
unsigned char status_task2=0;
unsigned char status_task3=0;

unsigned char status_task1_ant=0;
unsigned char status_task2_ant=0;
unsigned char status_task3_ant=0;

unsigned int exec_time_task1=0;
unsigned int exec_time_task2=0;
unsigned int exec_time_task3=0;

static unsigned char BufferOut[80];
static unsigned int my_time=0;


// MIPS40 - Run CPU at maximum speed 40MIPS (25ns), oscillator with PLL at 80Mhz
// MIPS4 - Run CPU at clock speed 4MIPS (250ns), oscillator without PLL at 8Mhz
#define MIPS40

#ifdef MIPS40
	// Primary (XT, HS, EC) Oscillator with PLL
	_FOSCSEL(FNOSC_PRIPLL);
#endif
#ifdef MIPS4
	// Primary (XT, HS, EC) Oscillator without PLL
	_FOSCSEL(FNOSC_PRI);
#endif

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystanl
_FOSC(OSCIOFNC_ON & POSCMD_XT);
// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);
// Disable Code Protection
_FGS(GCP_OFF);


/* Program the Timer1 peripheral to raise interrupts */
void T1_program(void)
{
	T1CON = 0;		/* Stops the Timer1 and reset control reg	*/
	TMR1  = 0;		/* Clear contents of the timer register	*/
#ifdef MIPS40
	PR1   = 0x9c40;		/* PR1=40000 Load the Period register with the value of 1ms	*/
#endif
#ifdef MIPS4
	PR1   = 0x0fa0;		/* PR1=4000 Load the Period register with the value of 1ms	*/
#endif
	IPC0bits.T1IP = 5;	/* Set Timer1 priority to 1		*/
	IFS0bits.T1IF = 0;	/* Clear the Timer1 interrupt status flag	*/
	IEC0bits.T1IE = 1;	/* Enable Timer1 interrupts		*/
	T1CONbits.TON = 1;	/* Start Timer1 with prescaler settings at 1:1
						* and clock source set to the internal
						* instruction cycle			*/
}

/* Clear the Timer1 interrupt status flag */
void T1_clear(void)
{
	IFS0bits.T1IF = 0;
}

/* This is an ISR Type 2 which is attached to the Timer 1 peripheral IRQ pin
 * The ISR simply calls CounterTick to implement the timing reference
 */
ISR2(_T1Interrupt)
{
	/* clear the interrupt source */
	T1_clear();

	/* count the interrupts, waking up expired alarms */
	CounterTick(myCounter);
}


/* Writes an initial message in the LCD display first row */
void put_LCD_initial_message()
{
	EE_lcd_goto( 0, 0 );

	EE_lcd_putc('R');
	EE_lcd_putc('e');
	EE_lcd_putc('a');
	EE_lcd_putc('l');
	EE_lcd_putc(' ');
	EE_lcd_putc('T');
	EE_lcd_putc('i');
	EE_lcd_putc('m');
	EE_lcd_putc('e');
	EE_lcd_putc(' ');
	EE_lcd_putc('-');
	EE_lcd_putc(' ');
	EE_lcd_putc('L');
	EE_lcd_putc('a');
	EE_lcd_putc('b');
	EE_lcd_putc('4');

}

/******************************************************************************************
 * Funci�n:	Serial_init()						     										  *
 * Descripci�n:	Configura Serial Port			 		          								  *
 ******************************************************************************************/
void Serial_Init(void)
{
	/* Stop UART port */
	U2MODEbits.UARTEN = 0;

	/* Disable Interrupts */
	IEC1bits.U2RXIE = 0;
	IEC1bits.U2TXIE = 0;

	/* Clear Interrupt flag bits */
	IFS1bits.U2RXIF = 0;
	IFS1bits.U2TXIF = 0;

	/* Set IO pins */
	TRISFbits.TRISF12 = 0;  // CTS Output
	TRISFbits.TRISF13 = 0;  // RTS Output
	TRISFbits.TRISF5 = 0;   // TX Output
	TRISFbits.TRISF4 = 1;   // RX Input

	/* baud rate */
	U2MODEbits.BRGH = 0;
	U2BRG  = 21; // 42 -> 57600 baud rate / 21-> 115200 baud rate

	/* Operation settings and start port */
	U2MODE = 0;
	U2MODEbits.UEN = 0; //2
	U2MODEbits.UARTEN = 1;

	/* TX & RX interrupt modes */
	U2STA = 0;
	U2STAbits.UTXEN=1;
}

/******************************************************************************************
 * Send one byte						     										  *
 ******************************************************************************************/
int Serial_Send(unsigned char data)
{
	while (U2STAbits.UTXBF);
	U2TXREG = data;
	while(!U2STAbits.TRMT);
	return 0;
}

/******************************************************************************************
 * Send a group of bytes						     										  *
 ******************************************************************************************/
void Serial_Send_Frame(unsigned char *ch, unsigned char len)
{
   unsigned char i;

   for (i = 0; i < len; i++) {
	Serial_Send(*(ch++));
   }
}


/******************************************************************************************
 * TASKS					     										  *
 ******************************************************************************************/


TASK(Task1)
{
	LATAbits.LATA0=1;

	status_task1=2;
	status_task2_ant=status_task2;
	status_task3_ant=status_task3;
	if(status_task2==2)
		status_task2=1;
	else if(status_task3==2)
		status_task3=1;


	__delay_ms(exec_time_task1);

	status_task1=0;
	status_task2=status_task2_ant;
	status_task3=status_task3_ant;

	LATAbits.LATA0=0;
}

TASK(Task2)
{
	LATAbits.LATA1=1;

	status_task1_ant=status_task1;
	status_task2=2;
	status_task3_ant=status_task3;
	if(status_task1==2)
		status_task1=1;
	else if(status_task3==2)
		status_task3=1;

	__delay_ms(exec_time_task2);

	status_task1=status_task1_ant;
	status_task2=0;
	status_task3=status_task3_ant;

	LATAbits.LATA1=0;
}

TASK(Task3)
{
	LATAbits.LATA2=1;

	status_task1_ant=status_task1;
	status_task2_ant=status_task2;
	status_task3=2;
	if(status_task1==2)
		status_task1=1;
	else if(status_task2==2)
		status_task2=1;

	__delay_ms(exec_time_task3);

	status_task1=status_task1_ant;
	status_task2=status_task2_ant;
	status_task3=0;

	LATAbits.LATA2=0;
}


TASK(Task4)
{
	// Update task status in display
	put_LCD_initial_message();
	EE_lcd_goto( 4, 1 );
	EE_lcd_putc(status_task3+'0');
	EE_lcd_putc(status_task2+'0');
	EE_lcd_putc(status_task1+'0');

	// Send task status to PC
	BufferOut[0]=0x01;
	BufferOut[1]=(char)(my_time>>8);
	BufferOut[2]=(char)my_time;
	BufferOut[3]=status_task1;
	BufferOut[4]=status_task2;
	BufferOut[5]=status_task3;
	Serial_Send_Frame(BufferOut,6);
	my_time++;
}


// Main function
int main(void)
{
#ifdef MIPS40
	/* Clock setup for 40MIPS */
	/* PLL Configuration */
	PLLFBD=38; 				// M=40
	CLKDIVbits.PLLPOST=0; 	// N1=2
	CLKDIVbits.PLLPRE=0; 	// N2=2
	OSCTUN=0; 				// FRC clock use
	RCONbits.SWDTEN=0; 		//watchdog timer disable
	while(OSCCONbits.LOCK!=1); //wait for PLL LOCK
#endif
#ifdef MIPS4
	/* Clock setup for 4MIPS */
	/* No PLL Configuration */
#endif

	/* Program Timer 1 to raise interrupts */
	T1_program();

	/* Init leds */
	TRISAbits.TRISA0=0;
	TRISAbits.TRISA1=0;
	TRISAbits.TRISA2=0;

	Serial_Init();

	/* Init LCD */
	EE_lcd_init();
	EE_lcd_clear();

	/* Program cyclic alarms which will fire after an initial offset, and after that periodically */
	exec_time_task1=5000;
	exec_time_task2=2500;
	exec_time_task3=2500;

	SetRelAlarm(Alarm1, 2000,  10000);
	SetRelAlarm(Alarm2, 1500,  10000);
	SetRelAlarm(Alarm3, 1000,  10000);

	SetRelAlarm(Alarm4, 1000,  100);

	 /* Forever loop: background activities (if any) should go here */
	for (;;);

	return 0;
}
