/*
 * This file is part of Antz.
 * Copyright (C) 2017  Albert Farres

 * Antz is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Antz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Antz.  If not, see <http://www.gnu.org/licenses/>.
 */
#if !defined( __AVR_ATmega328P__ )
#warning "This code is intended to be built for an atmega328p"
#endif
#define F_CPU 8000000UL
#include <avr/io.h>

#include "config.h"
#include "register.h"

#include "antz.hpp"
#include "ifaces.hpp"

#define DDR_SPI  DDRB
#define DD_MOSI  DDB3
#define DD_SCK   DDB5
#define CV1_REG     B
#define CV1_PIN     2
#define CV2_REG     B
#define CV2_PIN     7
#define NOTE_REG    C
#define NOTE_PIN    1
#define GATE_REG    D
#define GATE_PIN    2
#define SW1_REG     D
#define SW1_PIN     5
#define SW2_REG     D
#define SW2_PIN     6
#define SW3_REG     D
#define SW3_PIN     7
#define VEL_REG     B
#define VEL_PIN     0

#define BAUD_PRESCALE( _f_cpu_, _baudrate_ ) ((( _f_cpu_ / (_baudrate_ * 8UL))) - 1)
#define USART_BAUDRATE 31250UL

static void usart_init( void )
{
	const uint16_t baud = BAUD_PRESCALE( F_CPU, USART_BAUDRATE );

	UCSR0A = _BV( U2X0 ); /* Enable double transmission speed */
	
	UBRR0H = (uint8_t)(baud>>8); /* Set baud rate */
	UBRR0L = (uint8_t) baud;

	UCSR0B = _BV( RXEN0 )                               ;/* Enable receiver */
	UCSR0C = _BV( USBS0)| _BV( UCSZ00 ) | _BV( UCSZ01 ); /* Set frame format: 8data, 2stop bit */
}

static inline uint8_t usart_receive( void )
{
	while( !(UCSR0A & _BV( RXC0)) ); /* Wait for data to be received */
	return UDR0;                     /* Get and return received data from buffer */
}

static inline uint8_t spi_master_sndrcv( const uint8_t data )
{
	SPDR = data;                    /* Start transmission */
	while( !(SPSR & _BV( SPIF )) ); /* Wait for reception complete */
	return SPDR;                    /* Return Data Register */
}

static inline void spi_update( const uint8_t value, volatile uint8_t * const port, volatile uint8_t pin )
{
	*port &=~_BV( pin ); /* Drive CV low  */
	spi_master_sndrcv( 0b00110000 | (value >> 4) );
	spi_master_sndrcv(               value << 4  );
	*port |= _BV( pin ); /* Drive CV high */
}

static void spi_init( void )
{
	/* Set outputs */
	DDR_SPI     |= _BV( DD_SCK ) | _BV( DD_MOSI );
	DDR(  CV1 ) |= _BV( DD( CV1 ) );
	DDR(  CV2 ) |= _BV( DD( CV2 ) );
	PORT( CV1 ) |= _BV( P( CV1 ) ) ; /* Drive CV high */
	PORT( CV2 ) |= _BV( P( CV2 ) ) ; /* Drive CV high */

	SPCR |= _BV( SPE ) | _BV( MSTR ); /* Enable SPI, Master */

	// Reset cv
	spi_update( 0, &PORT( CV1 ), P( CV1 ) );
	spi_update( 0, &PORT( CV2 ), P( CV2 ) );
}

struct AVR_MIDI_packet_parser: public Iface_MIDI_packet_parser<AVR_MIDI_packet_parser>
{
	static void get_packet_impl( uint8_t (&buffer)[3] )
	{
		buffer[0] = 0; buffer[1] = 0;
		while( 1 )
		{
			buffer[2] = usart_receive();
			if( buffer[0] >> 5 == 0b100 // Only intersted in release / depress msgs.
			 && buffer[1] >> 7 == 0
			 && buffer[2] >> 7 == 0 )
			{
				// Check channel matches
				const uint8_t ch = (READ_PIN( SW3 ) ? (1<<2) : 0)
				                 + (READ_PIN( SW2 ) ? (1<<1) : 0)
				                 + (READ_PIN( SW1 ) ?  1     : 0);
				if( (buffer[0] & 0x0f) == ch )
					break;
			}
			buffer[0] = buffer[1];
			buffer[1] = buffer[2];
		}
	}
};

struct AVR_Antz_view: public Iface_Antz_view<AVR_Antz_view>
{
	static void set_note_impl( const bool value )
	{
		if( value )
			WRT1_PIN( NOTE ); /* Drive NOTE high */
		else
			WRT0_PIN( NOTE ); /* Drive NOTE low  */
	}
	static void set_gate_impl( const bool value )
	{
		if( value )
			WRT1_PIN( GATE ); /* Drive GATE high */
		else
			WRT0_PIN( GATE ); /* Drive GATE low  */
	}
	static void set_cv1_impl( const uint8_t value )
	{
		spi_update( value, &PORT( CV1 ), P( CV1 ) );
	}
	static void set_cv2_impl( const uint8_t value )
	{
		spi_update( value, &PORT( CV2 ), P( CV2 ) );
	}
	static void set_vel_impl( const uint8_t value )
	{
		spi_update( value, &PORT( CV2 ), P( CV2 ) );
	}
};

static inline void ddr_init( void )
{
	// Set gate and note DDR
	DDR( NOTE ) |= _BV( DD( NOTE ) );
	DDR( GATE ) |= _BV( DD( GATE ) );

	// Set channel switch DDR
	DDR( SW1 ) &= ~_BV( DD( SW1 ) );
	DDR( SW2 ) &= ~_BV( DD( SW2 ) );
	DDR( SW3 ) &= ~_BV( DD( SW3 ) );
	DDR( VEL ) &= ~_BV( DD( VEL ) );
}

static uint8_t MIDI_buffer[3];

ANTZ_MODEL_STATIC_INIT( AVR_Antz_view )

typedef Antz_controller<AVR_Antz_view> Controller;
typedef AVR_MIDI_packet_parser         Parser;

int main( int argc, char * argv[] )
{
	ddr_init();
	spi_init();
	usart_init();

	while( 1 )
	{
		Parser::get_packet( MIDI_buffer );

		Controller::set_cv2( READ_PIN( VEL ) != 0 );
		Controller::process_msg( MIDI_buffer );
	}
}
