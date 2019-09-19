
/*
 * main.c
 *
 * Created: 12/7/2018 2:37:04 PM
 *  Author: Hussam
 */

#include <avr/io.h>
#define  F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

#define counter  50000



int timeDisplay = 0 ;
int adjusting = 1 ;
int processing = 2 ;
int paused = 3 ;
int finished = 4 ;

volatile int state=0;

int timeNow = 0;
int sec = 0;
int reminder = 0;
volatile int stopWatch = 0;
volatile int* displayValue;

void display();
void buzzerOn();


ISR(TIMER0_OVF_vect)
{
	TCCR0B = 0;
	PORTD &= ~(1<<PD5);
}

ISR(TIMER1_OVF_vect)
{
	sec++;
	if (sec == 60)
	{
		sec = 0;
		timeNow++;
		if (reminder !=0)
		{
			buzzerOn();
		}
	}

	PORTD ^= (1<<PD7);
	if (state == adjusting)	
	{
		displayValue = &stopWatch;
		PORTD &= ~(1<<PD6);

		PORTC |= (1<<PD4);
		PORTC &= ~(1<<PD5);
	
	}
	else if (state == paused)
	{
		displayValue = &stopWatch;
		PORTD &= ~(1<<PD6);

		PORTC &= ~(1<<PD4);
		PORTC &= ~(1<<PD5);
	
	}
	else if (state == timeDisplay)
	{
		displayValue = &timeNow;
		PORTD &= ~(1<<PD6);

		PORTC &= ~(1<<PD4);
		PORTC &= ~(1<<PD5);
	
	}
	else if (state == processing)
	{
		displayValue = &stopWatch;
		PORTD |= (1<<PD6);

		PORTC &= ~(1<<PD4);
		PORTC |= (1<<PD5);
	
		if (stopWatch > 0)
		{
			stopWatch--;
		}
		else
		{
	
			state = finished;
			reminder = 1;
	
		}
	}
	else if (state == finished )
	{
		PORTD &= ~(1<<PD6);
		displayValue = &finished;
		state = timeDisplay;
		PORTC &= ~(1<<PD4);
		PORTC &= ~(1<<PD5);
		buzzerOn();
	}
	TCNT1= counter;
}

ISR(INT0_vect)
{
	if(state == timeDisplay)
	{
		state = adjusting;
	}
	else if (state == adjusting)
	{
		state = timeDisplay;
		stopWatch = 0;
	}
}


ISR(INT1_vect)
{
	if (state == processing)
	{
		state = paused;
	}
}


int main(void)
{
	DDRB = 0xFF;
	DDRC = 0XFF;
	DDRD = 0b11100000;
	PORTD |= ~(0b11100000); // enable pulling

	

	//adjusting clock at starting
	int steps []= {1,10,60,600};
	int step = 0;
	displayValue = &timeNow;
	while(1)
	{
		display();
		if(!(PIND & (1<<PD0)))
		{
			while(!(PIND & (1<<PD0)));
			timeNow += steps[step];
		}
		if(!(PIND & (1<<PD1)))
		{
			while(!(PIND & (1<<PD1)));
			if (step < 3)
			{
				step++;
			}
			else
			{
				step = 0;
			}
		}
		if(!(PIND & (1<<PD4)))
		{
			while(!(PIND & (1<<PD4)));
			//clock adjusting finished
			PORTD |= (1<<PD7);
			break;
		}
	}

	//init interrupts and timers
	sei();

	
	EIMSK |= (1<<INT1) |  (1<<INT0);
	EICRA |=  (1<< ISC11) |  (1<< ISC01) ;
	

	TCNT1=counter;
	TCCR1B = 0x05;
	TIMSK1 = (1<<TOIE1);

	while(1)
	{
		display();
		if (state == adjusting)
		{
			if(!(PIND & (1<<PD0)))
			{
				while(!(PIND & (1<<PD0)));
				stopWatch += 10;
			}
			else if(!(PIND & (1<<PD1)))
			{
				while(!(PIND & (1<<PD1)));
				if ((PIND & (1<<PD3)))
				{
					state = processing;
				}
			}
		}

		    
		else if (state == paused)
		{
			if(!(PIND & (1<<PD4)))
			    {
				    while(!(PIND & (1<<PD4)));
				    stopWatch = 0;
					buzzerOn();
				    state = timeDisplay;
			    }
			    else if(!(PIND & (1<<PD1)))
			    {
				    while(!(PIND & (1<<PD1)));
					if ((PIND & (1<<PD3)))
					{
						state = processing;
					}
			    }
			    
		    }

		    else if (state == processing)
		    {
			    if(!(PIND & (1<<PD4)))
			    {
				    while(!(PIND & (1<<PD4)));
				    state = paused;
			    }
		    }

		    
	    }
	
	

}

void display()
{
	int minutes;
	int seconds;
	if (displayValue != &finished)
	{
		minutes = *displayValue / 60;
		seconds = *displayValue % 60;
	}
	else
	{	
		minutes = 0;
		seconds = 0;
	}
		
	PORTB = seconds%10;
	PORTC |= (1<<PD1);
	_delay_ms(1);
	PORTC &= ~(1<<PD1);
	
	PORTB = seconds/10;
	PORTC |= (1<<PD0);
	_delay_ms(1);
	PORTC &= ~(1<<PD0);

	PORTB = minutes%10;
	PORTC |= (1<<PD2);
	_delay_ms(1);
	PORTC &= ~(1<<PD2);

	PORTB = minutes/10;
	PORTC |= (1<<PD3);
	_delay_ms(1);
	PORTC &= ~(1<<PD3);
	
}

void buzzerOn()
{
	PORTD |= (1<<PD5);
	_delay_ms(500);
	PORTD &= ~(1<<PD5);
	//In case we use timer zero to generate sqaure wave as input to buzzer
	//TCNT0=0;
	//TIMSK0 = (1<<TOIE0);
	//TCCR0B = 0x05;
}