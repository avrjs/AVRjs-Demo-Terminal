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

#ifndef MCU_TERM_H
#define	MCU_TERM_H

#include <stdlib.h>

#define MCU_TERM_BUFFER_SIZE 81

struct mcu_term_cmd
{
    void(*cb)(void*, size_t, char**);
    void* cb_arg;
    char* cmd;
};

struct mcu_term
{
    int(*print)(char*);
    char line_buffer[MCU_TERM_BUFFER_SIZE];
    size_t line_buffer_population;
    char* prompt;
    struct mcu_term_cmd* cmds;
    size_t cmds_size;
    char** argv;
    size_t argc;
    char last_char;
};

int mcu_term_add_command(struct mcu_term* const mt, const char* const cmd,
                         void(* const cb)(void*, size_t, char**), void* const cb_arg);
int mcu_term_remove_command(struct mcu_term* const mt, const char* const cmd);
int mcu_term_write_char(struct mcu_term* const mt, const char c);
void mcu_term_destroy(struct mcu_term* const mt);
int mcu_term_init(struct mcu_term* const mt, const char* const prompt, int(* const print)(char*));

#endif

