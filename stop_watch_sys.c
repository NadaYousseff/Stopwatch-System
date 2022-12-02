/*
 * stop_watch_sys.c
 *
 *  Created on: Sep 17, 2022
 *      Author: Nada Youssef
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
unsigned char sec=0;
unsigned char min=0;
unsigned char hour=0;
void seven_seg_display(unsigned char value,unsigned char pin);

/* TIMER1 Interrupt Service Routine */
ISR(TIMER1_COMPA_vect){
	sec++;
	if(sec==60){     /* timer counted 1 minute */
		min++;
		sec=0;
	}
	if(min==60){     /* timer counted 1 hour */
		hour++;
		min=0;
	}
	if(hour==99){
		hour=0;
	}
}
/* INT0 Interrupt Service Routine */
ISR(INT0_vect){
	/* reset the stop watch */
	sec=0;
	min=0;
	hour=0;
}
/* INT1 Interrupt Service Routine */
ISR(INT1_vect){
	/* pause the stop watch */
	/* No clock source by clearing the clock select bits */
	TCCR1B &=~(1<<CS11)&~(1<<CS10);
}
/* INT2 Interrupt Service Routine */
ISR(INT2_vect){
	/* resume the stop watch */
	/* set the clock select bits again to prescaler 64 */
	TCCR1B|=(1<<CS11)|(1<<CS10);
}
/*
 * CPU clock=1Mhz and prescaler F_CPU/64.
 * F_timer= 15,625Khz, T_timer = 64us
 * It's required 15625 counts to reach 1sec.
 * which is possible with timer 1,
 * and no need for bigger prescaler for better accuracy
 */
/* TIMER1 COMPA initialization */
void TIMER1_COMPA_init(void){

	TCNT1=0;              /* setting Timer1 initial value to 0 */
	OCR1A=15625;          /* setting Timer1 Compare A value to 15625 */
	TIMSK|=(1<<OCIE1A);   /* Enable Timer1 Compare A Interrupt*/
	/* Configure timer control registers TCCR1A/TCCR1B
	 * mode 4: WGM13=0, WGM12=0, WGM11=0, WGM10=0
	 * Prescaler = F_CPU/64 CS10=1 CS11=1 CS12=0
	 * */
	TCCR1A|=(1<<FOC1A);
	TCCR1B|=(1<<WGM12)|(1<<CS11)|(1<<CS10);

}
/* INT0 initialization */
void INT0_init(void){
	DDRD &=~(1<<2);       /* Configure PD2(INT0) as Input */
	PORTD |=(1<<2);       /* Enable the Internal Pull-up Resistor */
	MCUCR |=(1<<ISC01);   /* Trigger INT0 with the falling edge */
	MCUCR &=~(1<<ISC00);
	GICR |=(1<<INT0);     /* Enable INT0 */
}
/* INT0 initialization */
void INT1_init(void){
	DDRD &=~(1<<3);                  /* Configure PD3(INT1) as Input */
	MCUCR |=(1<<ISC10)|(1<<ISC11);   /* Trigger INT1 with the raising edge */
	GICR |=(1<<INT1);                /* Enable INT1 */
}
/* INT2 initialization */
void INT2_init(void){
	DDRB &=~(1<<2);        /* Configure PB2(INT2) as Input */
	PORTB |=(1<<2);        /* Enable the Internal Pull-up Resistor */
	MCUCSR &=~(1<<ISC2);
	GICR |=(1<<INT2);      /* Enable INT2 */
}

int main (void){
	/*** Initialization Code ***/

	SREG |=(1<<7);        /* setting the I-Bit (Enable Interrupts) */
	DDRA |=0x3F;          /* configure the first 6-bits of PORTA as Output  (The six 7-Segments Enables) */
	PORTA &=~(0x3F);      /* initial value 0 */
	DDRC |=0x0F;          /* configure the first 4-bits of PORTC as Output  (Connected to the decoder) */

	INT0_init();             /* Enable INT0 */
	INT1_init();             /* Enable INT1 */
	INT2_init();             /* Enable INT2 */
	TIMER1_COMPA_init();     /* Enable TIMER1 COMPA */

	while(1){
		/*** Application Code***/

		seven_seg_display((sec%10),0);
		seven_seg_display((sec/10),1);
		seven_seg_display((min%10),2);
		seven_seg_display((min/10),3);
		seven_seg_display((hour%10),4);
		seven_seg_display((hour/10),5);

	}
}

/* Function to display the values on the right specific 7-Segment */
void seven_seg_display(unsigned char value,unsigned char pin){

	PORTC=(PORTC & 0xF0)|(value & 0x0F);  /* Set the output on the decoder to the specific value */
	PORTA |=(1<<pin);                     /* Enable just the specific 7-Segment */
	_delay_us(5);
	PORTA &=~(1<<pin);                    /* Disable the 7-Segment */

}


