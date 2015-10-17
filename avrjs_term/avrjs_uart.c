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

#include <avr/io.h>

#include <stdio.h>

void uart_tx(const unsigned char data)
{
	while (!(UCSR0A & (1 << UDRE0)))
	{
	}
	UDR0 = data;
}

int uart_rx(unsigned char* const data)
{
	if (UCSR0A & (1 << RXC0))
	{
		*data = UDR0;
		return 1;
	}
	return 0;
}

static int uart_putchar_printf(char var, FILE *stream)
{
	uart_tx((unsigned char) var);
	return 0;
}

void uart_init(const uint16_t ubrr)
{
	UBRR0L = ubrr & 0xFF;
	UBRR0H = ubrr >> 8;

	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
}

void printf_init(void)
{
	uart_init(51);
	static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar_printf, NULL, _FDEV_SETUP_WRITE);
	stdout = &mystdout;
}
