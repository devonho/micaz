#include "blink.h"
#include <avr/io.h>
#include <util/delay.h>

#define BAUD 115200

#include <stdio.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>
#include "i2c.h"

//#define EN_I2C_TEST

void uart_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}

void uartTest()
{
	unsigned char count = 0;	
	while(1)
	for(count='0';count<'9';count++)
	{		
		uart_putchar('t');
		uart_putchar('e');
		uart_putchar('s');
		uart_putchar('t');
		uart_putchar(count);
		uart_putchar('\r');
		uart_putchar('\n');		
	};
}

void LEDinit() // function declaration
{
	DDRA = 0xFF; // set up pin direction	
}

void blinkLEDTest() // function declaration
{
	while(1)
	{
		PORTA ^= 0x03 ; // toggle the pins
		_delay_ms(100); //delay		
	}
}

void setupTimer0()
{
	ASSR |= _BV(AS0);
	
	 // set timer0 counter initial value to 0
	TCNT0=0x00;
	
	TCCR0 = (1<<CS02) | (1<<CS00); // start timer0 with /128 prescaler
	//TCCR0 = (1<<CS02) | (1<<CS01) | (1<<CS00); // start timer0 with /1024 prescaler
	
	// enable timer overflow interrupt for both Timer0 and Timer1
	TIMSK=(1<<TOIE0);	
}
// timer1 overflw

ISR(TIMER0_OVF_vect) {

	// process the timer1 overflow here
	PORTA ^= 0x04 ; // toggle the pins
	TCNT0=0x00;
}

void readIMUI2C()
{
	// Read MPU9150 register WHO_AM_I 0x75
	// MPU9150 I2C address when pin AD0=0 : 0x68 & 0xFE = 0x68
	uint8_t whoami = i2cReadRegister(0x68,0x75);
	
	char acWhoAmI[5];
	sprintf(acWhoAmI, "%2x", whoami);
	
	uart_putchar(acWhoAmI[1]);
	uart_putchar(acWhoAmI[0]);
	uart_putchar('\r');
	uart_putchar('\n');	
}

void imuI2CTest()
{
	while(1)
	{
		readIMUI2C();
	};
}


void TogglePD0PD1()
{	
	DDRD = 0xFF; // set up pin direction
	while(1)
	{
		PORTD ^= 0x03 ; // toggle the pins
		_delay_us(1); //delay for 100kHz		
	}
}


int spiInit(void)
{
	// Set MOSI and SCK as output, others as input
	DDRB = (1<<PB2)|(1<<PB1)|(1<<PB0);

	/*
	PORTB &= ~(1<<PB0);

	// Enable SPI, Master, set clock rate fck/2 (maximum)
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = (1<<SPI2X);
	*/
	return 0;
}

// CC2420 is big endian. Read MANFIDL (0x1E) - Manufacturer ID, Lower 16 Bit
// ---
// dataout bit    23 - ram/reg
// dataout bit    22 - r/w
// dataout bit 21:16 - addr
// dataout bit 15:0  - data
// ---
// datain  bit 23:16 - status byte S7:S0
// datain  bit 15:0  - data D15:D0

// PB0 SSn
// PB1 SCK
// PB2 MOSI
// PB3 MISO

unsigned char* SPIRead(unsigned char addr) 
{
	int i = 0;
	static unsigned char datain[3]; // 24bits
	unsigned char dataout[3]; // 24bits

	dataout[2]=addr;
	dataout[1]=0x00;
	dataout[0]=0x00;

	// assert CSn
	PORTB &= ~(1<<PB0);

	for(i=2;i>=0;i--)
	{
		// Start transmission (MOSI)
		SPDR = dataout[i];
		// Wait for transmission complete
		while(!(SPSR & (1<<SPIF)));

		// Get return Value;
		datain[i] = SPDR;		
	}

	// de-assert CSn
	PORTB |= (1<<PB0);

	_delay_us(1);             // Hold pulse for 1 micro second

	// Return Serial In Value (MISO)
	return datain;
}

void spiTest()
{
	unsigned char* pcmanid;
	char acmanid[5];
	while(1) {
		
		pcmanid = SPIRead(0x1E);
		
		sprintf(acmanid, "%2x%2x", pcmanid[1], pcmanid[0]);
		
		uart_putchar(acmanid[0]);
		uart_putchar(acmanid[1]);
		uart_putchar(acmanid[2]);
		uart_putchar(acmanid[3]);
		uart_putchar(acmanid[4]);
		uart_putchar('\r');
		uart_putchar('\n');
		
	} 
	
}

void pbToggleTest()
{
	while(1)
	{
		PORTB ^= 0x07;
		_delay_us(1); //delay for 100kHz
	}
}

int main (void)
{
	LEDinit();
	uart_init();
	setupTimer0();
	
	#ifdef EN_I2C_TEST
	i2cInit();
	#endif //EN_I2C_TEST
	spiInit();

	sei();
	
	//uartTest();
	//spiTest();
	//TogglePD0PD1();
	//pbToggleTest();
	#ifdef EN_I2C_TEST
	imuI2CTest();
	#endif //EN_I2C_TEST
	blinkLEDTest();
}