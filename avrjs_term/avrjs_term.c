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
#include "mcu_term.h"

#include <avr/io.h>

#include <stdio.h>
#include <limits.h>

int term_print_str(char* c)
{
	while (*c)
	{
		uart_tx((unsigned char) *c);
		++c;
	}
	return 1;
}

long int gcd(long int a, long int b)
{
	if ((a == 0) && (b == 0))
	{
		return -1;
	}
	
	if (a < 0)
	{
		a = -a;
	}
	if (b < 0)
	{
		b = -b;
	}
	long int r;
	while(b != 0)
	{
		r = a % b;
		a = b;
		b = r;
	}
	return a;
}

void gcd_cmd_cb(void* arg, size_t argc, char** argv)
{
	(void) arg;
	if (argc != 3)
	{
		printf("Invalid number of args, gcd requires 2\n");
		return;
	}
	char* clamp_str = "%s clamped to %ld to fit into 32 bits\n";
	long int arg0 = strtol(argv[1], 0, 0);
	if ((arg0 == LONG_MIN) || (arg0 == LONG_MAX))
	{
		printf(clamp_str, argv[1], arg0);
	}
	long int arg1 = strtol(argv[2], 0, 0);
	if ((arg1 == LONG_MIN) || (arg1 == LONG_MAX))
	{
		printf(clamp_str, argv[2], arg1);
	}
	
	printf("%ld\n", gcd(arg0, arg1));
}

void lcm_cmd_cb(void* arg, size_t argc, char** argv)
{
	(void) arg;
	if (argc != 3)
	{
		printf("Invalid number of args, lcm requires 2\n");
		return;
	}
	char* clamp_str = "%s clamped to %ld to fit into 32 bits\n";
	long int arg0 = strtol(argv[1], 0, 0);
	if ((arg0 == LONG_MIN) || (arg0 == LONG_MAX))
	{
		printf(clamp_str, argv[1], arg0);
	}
	long int arg1 = strtol(argv[2], 0, 0);
	if ((arg1 == LONG_MIN) || (arg1 == LONG_MAX))
	{
		printf(clamp_str, argv[2], arg1);
	}
	
	long int tmp = (arg0 / gcd(arg0, arg1));
	long int result = tmp * arg1;
	
	if (tmp != (result / arg1))
	{ // overflow
		printf("overflow detected, result > %ld\n", LONG_MAX);
	}
	else
	{
		printf("%ld\n", result);
	}
}

int main(void)
{
	printf_init();
	struct mcu_term mt;
	if (mcu_term_init(&mt, "$", &term_print_str) != 0)
	{
		return -1;
	}

	mcu_term_add_command(&mt, "gcd", &gcd_cmd_cb, 0);
	mcu_term_add_command(&mt, "lcm", &lcm_cmd_cb, 0);

	printf("This terminal is connected to the UART0 port of a simulated AVR. This text and the prompt below are printed by the default program, you can load your own program by using the load button above.\n\n"
	"There is a copy of this default program as an Atmel Studio project on the AVRjs GitHub page: https://github.com/avrjs/avrjs it contains UART routines and implements printf to get you started printing things to this terminal. Please note that interrupts are not currently supported, this should change soon.\n\n"
	"THIS IS A TEST, please report any bugs to the AVRjs GitHub page linked in the footer. Thanks!\n\n"
	"Demo terminal commands:\n"
	"\"gcd a b\"\n"
	"where a and b are integers, this command will print the greatest common divisor of the 2 numbers providing they can fit in signed 32 bit ints\n"
	"\"lcm a b\"\n"
	"where a and b are integers, this command will print the lowest common multiple of the 2 numbers providing it can fit in a signed 32 bit int\n");

	unsigned char last_nl = 0;

	mcu_term_write_char(&mt, '\n');

    while(1)
    {
		unsigned char c;
        if (uart_rx(&c) > 0)
		{
			// change "\r", "\r\n" and "\n\r" to "\n"
			if (c == '\r')
			{
				if (last_nl == '\n')
				{
					last_nl = 0;
					continue;
				}
				last_nl = c;
				c = '\n';
			}
			else if  (c == '\n')
			{
				if (last_nl == '\r')
				{
					last_nl = 0;
					continue;
				}
				last_nl = c;
			}
			// echo
			uart_tx(c);
			// parse char
			if (mcu_term_write_char(&mt, (char) c) < 0)
			{
				mcu_term_destroy(&mt);
				return -1;
			}
		}
    }
	mcu_term_destroy(&mt);
	return 0;
}