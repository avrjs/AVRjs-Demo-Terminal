/*The MIT License (MIT)

Copyright (c) 2015 Julian Ingram

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "avrjs_uart.h"

#include "cirq.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <stdio.h>

volatile unsigned char _uart0_rx_buffer[UART0_RX_BUFFER_WIDTH];
volatile unsigned char _uart0_tx_buffer[UART0_TX_BUFFER_WIDTH];

struct cirq uart0_rx_buffer;
struct cirq uart0_tx_buffer;
volatile unsigned char uart0_rx_ovf_flag = 0;

size_t uart0_rx(uint8_t *const buffer, const size_t size)
{
	size_t recd = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		while (cirq_empty(&uart0_rx_buffer) == 0) 
		{
			buffer[recd] = cirq_pop_front(&uart0_rx_buffer);
			++recd;
		}
	}
	return recd;
}

size_t uart0_tx(const uint8_t *const data, const size_t size)
{
	size_t sent = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		while (cirq_space(&uart0_tx_buffer) && (sent < size))
		{
			cirq_push_back(&uart0_tx_buffer, data[sent]);
			++sent;
		}
	}
	UCSR0B |= (1 << UDRIE0); // enable UDR empty interrupt
	return sent;
}

void uart0_init(const uint16_t brr)
{
	uart0_rx_buffer = cirq_init(UART0_RX_BUFFER_WIDTH, _uart0_rx_buffer);
	uart0_tx_buffer = cirq_init(UART0_TX_BUFFER_WIDTH, _uart0_tx_buffer);
	UBRR0H = (uint8_t)(brr >> 8); // setup baud rate register
	UBRR0L = (uint8_t)brr;

	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // enable rx and tx, enable rx interrupt
	UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8N1
}

void uart0_destroy(void)
{
	UBRR0H = 0x00;
	UBRR0L = 0x00;

	UCSR0A = 0x20;
	UCSR0B = 0x00;
	UCSR0C = 0x06;
}

ISR (USART0_UDRE_vect)
{
	if(cirq_empty(&uart0_tx_buffer) == 0)
	{
		UDR0 = cirq_pop_front(&uart0_tx_buffer);
	}
	else
	{
		UCSR0B &= ~(1 << UDRIE0); // disable UDR empty interrupt
	}
}

ISR (USART0_RX_vect)
{
	if (cirq_space(&uart0_rx_buffer) != 0)
	{
		cirq_push_back(&uart0_rx_buffer, UDR0);
	}
	else
	{
		volatile uint8_t n = UDR0;
		(void) n;
		uart0_rx_ovf_flag = 1;
	}
}

static int uart_putchar_printf(char var, FILE *stream)
{
	return (uart0_tx((uint8_t*)&var, 1) != 1) ? 0 : -1;
}

void printf_init(void)
{
	uart0_init(1);
	static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
	stdout = &mystdout;
}
