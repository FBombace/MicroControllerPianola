/*
 * Copyright 2015 Fabio Bombace
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*** Libraries ***/
#include <pic16f1825.h>
#include <xc.h>

/*** PIC16F1825 Configuration Bit Settings ***/
#define _XTAL_FREQ 16000000		// Oscillator frequency in Hz after any internal PLL multiplier.
#pragma config FOSC = INTOSC	// Internal oscillator will be used.
#pragma config WDTE = OFF			// Disable watchdog Timer.
#pragma config PWRTE = OFF		// Disable power-up Timer.
#pragma config MCLRE = ON			// Master clear on.
#pragma config CP = OFF				// Disable code protection.
#pragma config CPD = OFF			// Disable data code Protection.
#pragma config BOREN = ON			// Enable Brown-out Reset.
#pragma config IESO = OFF			// Disable internal external switchover oscillator.
#pragma config FCMEN = OFF		// Disable fail-safe clock monitor.

/*** Variables ***/
int new_frequency = 0;
int threshold_frequency = 0;
int selected_cps_pin = 0;
int last_pin_touched = -1;
int step_count = 0;
int MAX_STEP_SOUND_LENGTH = 13;

void waitTimer2()
{
	PIR1bits.TMR2IF = 0; // Clear timer2 interrupt bit
	while(PIR1bits.TMR2IF == 0) // Wait until the value of timer2 and the period are equal
	{
		continue;
	}
}

void enableCCP1Out()
{
	TRISCbits.TRISC5 = 0;
}

void disableCCP1Out()
{
	TRISCbits.TRISC5 = 1;
}

void lightLed()
{
	PORTCbits.RC4 = 1;
}

void disableLed()
{
	PORTCbits.RC4 = 0;
}

void finishNote()
{
	disableLed();
	disableCCP1Out();
	last_pin_touched = -1;
	step_count = 0;
}

/*** Interrupt service routine ***/
void interrupt isr(void)
{
	/*** Routine for checking if someone has touched a key (cps input) and play the note ***/
	if(INTCONbits.T0IF == 1)
	{
		new_frequency = ((TMR1H << 8) + TMR1L);

		// To avoid timing problem (if any) with timer2 interrupt and don't send a
		// complete duty cycle and period, comment the following #define.
		#define FULL_DUTY_CYCLE_PERIOD

		/* Formulas from the datasheet:
		 *
		 * Frequency = Fosc / ((PR2 + 1) * (Timer2 prescaler) * 4).
		 * Period = 1 / F.
		 * Frequency = 1 / Period.
		 * Pulse Width = (CCPRxL : CCPxCON<5:4>) * Tosc * TMRx Prescale Value
		 * Tosc = 1 / Fosc.
		 * Duty cycle ratio = (CCPRxL:CCPxCON<5:4>) / 4*(PR2 + 1).
		 * Resolution = log(4*PR2 + 1) / log2.
		 */

		// Iterate the pin of the cps to determine the musical note that has to be played (if a touch has been done).
		switch(selected_cps_pin)
		{
			// CPS0, musical note DO, 1046 Hz
			case 0 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 0)
					{
						last_pin_touched = 0;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b01111110; // Load the period register
						CCP1CON = 0b00011100; // PWM mode, single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b00111111; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0001; // Set the next pin on CPS.
				break;
			// CPS1, musical note RE, 1175 Hz.
			case 1 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 1)
					{
						last_pin_touched = 0;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b10001101; // Load the period register.
						CCP1CON = 0b00111100; // PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01000110; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0010; // Set the next pin on CPS
				break;
			// CPS2, musical note MI, 1319 Hz.
			case 2 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 2)
					{
						last_pin_touched = 2;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b10011110; // Load the period register.
						CCP1CON = 0b00011100; // PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01001111; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0011; // Set the next pin on CPS.
				break;
			// CPS3, musical note FA, 1397 Hz.
			case 3 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 3)
					{
						last_pin_touched = 3;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b10110010; // Load the period register.
						CCP1CON = 0b00011100; // PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01011001; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0100; // Set the next pin on CPS.
				break;
			// CPS4, musical note SOL, 1568 Hz.
			case 4 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 4)
					{
						last_pin_touched = 4;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b10111100; // Load the period register.
						CCP1CON = 0b00011100; // PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01011110; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0101; // Set the next pin on CPS.
				break;
			// CPS5, musical note LA, 1760 Hz.
			case 5 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 5)
					{
						last_pin_touched = 5;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b11010100; // Load the period register.
						CCP1CON = 0b00011100; // PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01101010; // Set the duty cycle.

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0110; // Set the next pin on CPS.
				break;
			// CPS6, musical note SI, 1976 Hz
			case 6 :
				if(new_frequency < threshold_frequency)
				{
					if(last_pin_touched != 6)
					{
						last_pin_touched = 6;
						lightLed();
						disableCCP1Out();

						/* Play the musical note through setup for PWN generation */
						PR2 = 0b11101110; // Load the period register.
						CCP1CON = 0b00011100; //PWM mode, Single output; bit 4-5 are the LSB of duty cycle.
						CCPR1L = 0b01110111; // Set the duty cycle at 50%

						#ifdef FULL_DUTY_CYCLE_PERIOD
						waitTimer2();
						#endif

						enableCCP1Out();
					}
					step_count = 0;
				}
				else  // if enter here the pin has not been touched
				{
					step_count++;
					if(step_count == MAX_STEP_SOUND_LENGTH)
					{
						finishNote();
					}
				}
				CPSCON1bits.CPSCH = 0b0000; // Set the next pin on CPS.
				break;
		}
		selected_cps_pin = (selected_cps_pin + 1) % 7; // switch on the next CPS pin.

		T1CONbits.TMR1ON = 0;   // Disable timer1.
		TMR1L = 0;              // Reset low byte timer1.
		TMR0 = 0;               // Reset timer0; N.B.: delay of two instruction.
		TMR1H = 0;              // Reset high byte timer1.
		T1CONbits.TMR1ON = 1;   // Enable timer1.

		INTCONbits.T0IF = 0;    // reset timer0 interrupt bit.
	}
}

/*** Main ***/
void main()
{
	float cps_frequency = 0;

	/*** Clock PIC, set at 16 Mhz ***/
	OSCCON = 0b01111000;	// 16 Mhz, no PLL moltiplier.
	while(OSCSTATbits.HFIOFR == 0){ continue; } // Wait until the oscillator get ready.

	/*** Port input & output ***/
	TRISA = 0b00010111;		// Set PORTA bits 0, 1, 2, 4 to inputs
							// N.B. The 2 MSB bit are unimplemented in hardware, and always read as '0'.
	ANSELA = 0b00010111;	// Set analog input for CPS0, CPS1, CPS2 and CPS3.
	TRISC = 0b00000111;		// Setting PORTC; bit 0, 1, 2 as input, 3 to 7 as output
	ANSELC = 0b00000111;	// Set remaining analog input, for CPS4, CPS5 and CPS6.

	/*** Empiric prescaler and clock source Timer0 ***/
	/* Setting example:
	 * - With 16Mhz oscillator and no prescaler -> 16/4 = 4 Mhz -> (1/4)*10^-6 = 0.25 microsec. per istruction;
	 * overflow TMR0: 0.25*256 = 64 microsec
	 * - With 16Mhz oscillator and prescaler set at 32 -> (4/16)*32 = 8 microsec. per istr.;
	 * overflow TMR0: 8*256 = 2048 microsec = 2.048 ms. = 64*32 */
	OPTION_REGbits.TMR0CS = 0;  // Timer0 clock source is internal instruction cycle clock (FOSC/4).
	OPTION_REGbits.PSA = 0;		// Prescaler is assigned to the Timer0 module.
	OPTION_REGbits.PS = 0b111;	// PS<2:0> value, from 000 to 111; higher binary number -> higher prescale value;
								// in this case, prescaler set is 256.

	/*** CPS Setting ***/
	T1CON = 0b11000101 ;	// Timer1 clock source is Capacitive Sensing Oscillator, 1:1 Prescale, Dedicated Timer1
							// oscillator circuit disabled, Don't synchronize external clock input, enable timer1.
	T1GCON = 0b00000000;    // Disable Timer1 gate function, Timer1 counts regardless of gate function.

	CPSCON0bits.CPSRM = 0;		// CPS module is in the low range; Internal oscillator voltage references are used.
	CPSCON0bits.CPSRNG = 0b10;	// Oscillator is in Medium Mode.
	CPSCON0bits.CPSON = 1;		// Enable CPS module.
	CPSCON1bits.CPSCH = 0b0010;	// Set CPS0 as touch input.

	/*** CPS Oscillator frequency ***/
	T1CONbits.TMR1ON = 0;	// Disable timer1.
	TMR1L = 0;				// Reset low byte timer1.
	TMR0 = 0;				// Reset timer0; N.B.: delay of two instruction.
	TMR1H = 0;				// Reset high byte timer1.
	T1CONbits.TMR1ON = 1;	// Enable timer1.
	INTCONbits.TMR0IF = 0;	// Set the Timer0 Overflow Interrupt bit at 0.

	while(INTCONbits.TMR0IF == 0){ continue; } // wait until the timer0 set overflow bit
	cps_frequency = ((TMR1H << 8) + TMR1L);

	/*** Threshold frequency, empiric  ***/
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.1); // Set the threshold frequency at 90% of cps_frequency.
	threshold_frequency = (int)(cps_frequency - cps_frequency*0.2); // Set the threshold frequency at 80% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.3); // Set the threshold frequency at 70% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.4); // Set the threshold frequency at 60% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.5); // Set the threshold frequency at 50% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.6); // Set the threshold frequency at 40% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.7); // Set the threshold frequency at 30% of cps_frequency.
	// threshold_frequency = (int)(cps_frequency - cps_frequency*0.8); // Set the threshold frequency at 20% of cps_frequency.

	/*** Setting timer2 for PWM generation support ***/
	T2CON = 0b00000111;			// Enable timer2 and set his prescaler at 16
	CCPTMRSbits.C1TSEL = 0b00;	// Choose timer2 as timer resource for CCP1 (for PWM generation).

	/*** Resetting the timer0/1 value and Abiliting the interrupt ***/
	T1CONbits.TMR1ON = 0;	// Disable timer1
	TMR1L = 0;              // Reset low byte timer1
	TMR0 = 0;				// Reset timer0; Delay of two instruction.
	TMR1H = 0;				// Reset high byte timer1.
	T1CONbits.TMR1ON = 1;	// Enable timer1.
	INTCONbits.TMR0IF = 0;	// Set the Timer0 Overflow Interrupt bit at 0.

	INTCONbits.TMR0IE = 1; // Enables the Timer0 interrupt.
	INTCONbits.GIE = 1; // Global interrupt enable.

	/*** Infinite Loop, let's enjoy playing ***/
	for(;;){}
}