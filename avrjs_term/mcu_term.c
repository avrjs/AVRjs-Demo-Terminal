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

int mcu_term_add_command(struct mcu_term* const mt, const char* const cmd,
                         void(* const cb)(void*, size_t, char**), void* const cb_arg)
{
    // allocate memory for the string
    char* const cmd_cpy = mcu_term_allocate((strlen(cmd) + 1) * sizeof (*cmd_cpy));
    if (cmd_cpy == 0)
    {
        return -1;
    }
    // copy the string
    strcpy(cmd_cpy, cmd);
    // allocate memory for the struct 
    ++mt->cmds_size;
    struct mcu_term_cmd* const cmds_tmp = mcu_term_reallocate(mt->cmds,
						mt->cmds_size * sizeof (*cmds_tmp));
    if (cmds_tmp == 0)
    {
        --mt->cmds_size;
        return -1;
    }
    mt->cmds = cmds_tmp;
    // initialise the struct
    struct mcu_term_cmd* const tmp = mt->cmds + mt->cmds_size - 1;
    tmp->cmd = cmd_cpy;
    tmp->cb = cb;
    tmp->cb_arg = cb_arg;
    return 0;
}

int mcu_term_remove_command(struct mcu_term* const mt, const char* const cmd)
{
    // find the cmd
    struct mcu_term_cmd* cmds_itt = mt->cmds;
    struct mcu_term_cmd* const cmds_limit = mt->cmds + mt->cmds_size;
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

#include <stdio.h>

int mcu_term_write_char(struct mcu_term* const mt, const char c)
{
    // add the character to the buffer
    if (c == '\n')
    { // process
        mt->line_buffer[mt->line_buffer_population] = 0;
        if (mt->argc > 0)
        {
            // search for command
            struct mcu_term_cmd* cmds_itt = mt->cmds;
            struct mcu_term_cmd* const cmds_limit = mt->cmds + mt->cmds_size;
			
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
        mt->last_char = ' ';
        mt->line_buffer_population = 0;
        return mt->print(mt->prompt);
    }
    else
    { // add char
        if ((mt->last_char == ' ') && (c != ' '))
        { // beginning of word
            ++(mt->argc);
            char ** argv_tmp = mcu_term_reallocate(mt->argv, mt->argc *
                                                   sizeof (*mt->argv));
            if (argv_tmp == 0)
            {
                return -1;
            }
            mt->argv = argv_tmp;
            mt->argv[mt->argc - 1] = mt->line_buffer +
                    mt->line_buffer_population;
        }

        if ((mt->last_char != ' ') && (c == ' '))
        {
            mt->line_buffer[mt->line_buffer_population] = 0;
        }
        else
        {
            mt->line_buffer[mt->line_buffer_population] = c;
        }
        mt->last_char = c;
        ++mt->line_buffer_population;
    }
    return 0;
}

void mcu_term_destroy(struct mcu_term* const mt)
{
    // deallocate command strings
    struct mcu_term_cmd* cmds_itt = mt->cmds;
    struct mcu_term_cmd* const cmds_limit = mt->cmds + mt->cmds_size;
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

int mcu_term_init(struct mcu_term* const mt, const char* const prompt, int(* const print)(char*))
{
    mt->prompt = mcu_term_allocate((strlen(prompt) + 1) * sizeof(*mt->prompt));
    if (mt->prompt == 0)
    {
        return -1;
    }
    strcpy(mt->prompt, prompt);
    mt->last_char = ' ';
    mt->line_buffer_population = 0;
    mt->argc = 0;
    mt->argv = 0;
    mt->cmds = 0;
    mt->cmds_size = 0;
    mt->print = print;
    return 0;
}