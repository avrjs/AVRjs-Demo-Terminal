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

#include "mcu_term.h"

#include <string.h>

static inline void* mcu_term_allocate(const size_t size)
{
    return malloc(size);
}

static inline void* mcu_term_reallocate(void* const ptr, const size_t size)
{
    return realloc(ptr, size);
}

static inline void mcu_term_deallocate(void* const ptr)
{
    free(ptr);
}

int mcu_term_print_string(const struct mcu_term * const mt, const char* str)
{
    size_t i = 0;
    while (str[i] != 0)
    {
        if (mt->print(str[i]) != 0)
        {
            break;
        }
        ++i;
    }
    return i;
}

int mcu_term_add_command(struct mcu_term * const mt, const char* const cmd,
                         void(* const cb) (void*, size_t, char**),
                         void* const cb_arg)
{
    // allocate memory for the string
    char* const cmd_cpy = mcu_term_allocate((strlen(cmd) + 1) *
                                            sizeof (*cmd_cpy));
    if (cmd_cpy == 0)
    {
        return -1;
    }
    // copy the string
    strcpy(cmd_cpy, cmd);
    // allocate memory for the struct 
    ++mt->cmds_size;
    struct mcu_term_cmd * const cmds_tmp =
            mcu_term_reallocate(mt->cmds, mt->cmds_size * sizeof (*cmds_tmp));
    if (cmds_tmp == 0)
    {
        --mt->cmds_size;
        return -1;
    }
    mt->cmds = cmds_tmp;
    // initialise the struct
    struct mcu_term_cmd * const tmp = mt->cmds + mt->cmds_size - 1;
    tmp->cmd = cmd_cpy;
    tmp->cb = cb;
    tmp->cb_arg = cb_arg;
    return 0;
}

int mcu_term_remove_command(struct mcu_term * const mt, const char* const cmd)
{
    // find the cmd
    struct mcu_term_cmd* cmds_itt = mt->cmds;
    struct mcu_term_cmd * const cmds_limit = mt->cmds + mt->cmds_size;
    while ((cmds_itt != cmds_limit) && (strcmp(cmds_itt->cmd, cmd) != 0))
    {
        ++cmds_itt;
    }
    if ((cmds_itt == cmds_limit))
    {
        return -1; // command not found
    }
    // free the string
    mcu_term_deallocate(cmds_itt->cmd);
    // shuffle the list down
    while (cmds_itt < cmds_limit - 1)
    {
        *cmds_itt = *(cmds_itt + 1);
    }
    --mt->cmds_size;
    struct mcu_term_cmd* cmds_tmp = mcu_term_reallocate(mt->cmds, mt->cmds_size
                                                        * sizeof (*cmds_tmp));
    if (cmds_tmp == 0)
    {
        ++mt->cmds_size;
        return -1;
    }
    mt->cmds = cmds_tmp;
    return 0;
}

int mcu_term_write_char(struct mcu_term * const mt, const char c)
{
    switch (c)
    {
    case '\r':
    { // process
        mt->line.arr[mt->line.population] = 0;
        size_t i = 0;
        char last_c = ' ';
        // split buffer with NULLs
        while (i < mt->line.population)
        {
            char c = mt->line.arr[i];
            if ((last_c == ' ') && (c != ' '))
            { // beginning of word
                ++(mt->argc);
                char ** argv_tmp = mcu_term_reallocate(mt->argv, mt->argc *
                                                       sizeof (*mt->argv));
                if (argv_tmp == 0)
                {
                    return -1;
                }
                mt->argv = argv_tmp;
                mt->argv[mt->argc - 1] = mt->line.arr + i;
            }

            if ((last_c != ' ') && (c == ' '))
            { // end of a word
                mt->line.arr[i] = 0;
            }
            last_c = c;
            ++i;
        }
		
		mt->print('\r');
		mt->print('\n');
        if (mt->argc > 0)
        {
            // search for command
            struct mcu_term_cmd* cmds_itt = mt->cmds;
            struct mcu_term_cmd * const cmds_limit = mt->cmds + mt->cmds_size;
            while ((cmds_itt != cmds_limit) && (strcmp(cmds_itt->cmd,
                                                       mt->argv[0]) != 0))
            {
                ++cmds_itt;
            }
            if (cmds_itt != cmds_limit)
            { // call command if it exists
                cmds_itt->cb(cmds_itt->cb_arg, mt->argc, mt->argv);
            }
            mcu_term_deallocate(mt->argv);
            mt->argc = 0;
            mt->argv = 0;
        }
        mt->line.population = 0;
        mcu_term_print_string(mt, mt->prompt);
        break;
    }
    case '\b':
        if (mt->line.population != 0)
        {
            mt->print('\b');
            mt->print(' ');
            mt->print('\b');
            --mt->line.population;
        }
        break;
    case '\n':// ignore newline
        break;
    default:
        mt->line.arr[mt->line.population] = c;
        ++mt->line.population;
        mt->print(c);
        break;
    }
    return 0;
}

void mcu_term_destroy(struct mcu_term * const mt)
{
    // deallocate command strings
    struct mcu_term_cmd* cmds_itt = mt->cmds;
    struct mcu_term_cmd * const cmds_limit = mt->cmds + mt->cmds_size;
    while (cmds_itt != cmds_limit)
    {
        mcu_term_deallocate(cmds_itt->cmd);
        ++cmds_itt;
    }
    // deallocate command struct array
    mcu_term_deallocate(mt->cmds);
    mcu_term_deallocate(mt->prompt);
    if (mt->argv != 0)
    {
        mcu_term_deallocate(mt->argv);
    }
}

int mcu_term_init(struct mcu_term * const mt, const char* const prompt,
                  char (* const print) (char))
{
    mt->prompt = mcu_term_allocate((strlen(prompt) + 1) * sizeof (*mt->prompt));
    if (mt->prompt == 0)
    {
        return -1;
    }
    strcpy(mt->prompt, prompt);
    mt->line.population = 0;
    mt->argc = 0;
    mt->argv = 0;
    mt->cmds = 0;
    mt->cmds_size = 0;
    mt->print = print;
    mcu_term_print_string(mt, mt->prompt);
    return 0;
}
