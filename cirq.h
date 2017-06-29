/*
The MIT License (MIT)
Copyright (c) 2013 Julian Ingram
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

#ifndef CIRQ_H
#define CIRQ_H

#include <stdlib.h>
#include <string.h>

struct cirq
{
    volatile unsigned char* buffer;
    volatile unsigned char* buffer_limit;
    volatile unsigned char* buffer_max;
    volatile unsigned char* volatile head;
    volatile unsigned char* volatile tail;
};

static inline struct cirq cirq_init(const size_t width,
    volatile unsigned char* buffer)
{
    volatile unsigned char* limit = buffer + width;
    struct cirq cirq = {
        .buffer = buffer,
        .buffer_limit = limit,
        .buffer_max = limit - 1,
        .head = (unsigned char *volatile)cirq.buffer,
        .tail = (unsigned char *volatile)cirq.buffer
    };
    return cirq;
}

static inline char cirq_empty(const struct cirq* const c)
{
    return (c->head == c->tail) ? 1 : 0;
}

// this space function returns one less than the actual space available, because
// when the buffer is (actually) full, head == tail and it is indistinguishable
// from an empty buffer.
static inline size_t cirq_space(const struct cirq* const c)
{
    return (c->tail >= c->head) ? ((c->buffer_limit - c->tail) +
            (c->head - c->buffer)) - 1 : (c->head - c->tail) - 1;
}

static inline size_t cirq_population(const struct cirq* const c)
{
    return (c->tail >= c->head) ? c->tail - c->head :
    (c->buffer_limit - c->head) + (c->tail - c->buffer);
}

static inline void cirq_flush(struct cirq* const c)
{
    c->tail = c->head;
}

// tail always points to free space, head points to a given value, except when
// the queue is empty.

// increments tail
static inline void cirq_push_back(struct cirq* const c,
    const unsigned char item)
{
    volatile unsigned char* const p = c->tail;
    *p = item;
    c->tail = (p == c->buffer_max) ? c->buffer : p + 1;
}

// decrements head
static inline void cirq_push_front(struct cirq* const c,
    const unsigned char item)
{
    volatile unsigned char* const head = c->head;
    volatile unsigned char* const p = (head == c->buffer) ? c->buffer_max :
        head - 1; // this has const issues?
    *p = item;
    c->head = p;
}

// decremenets tail
static inline unsigned char cirq_pop_back(struct cirq* const c)
{
    volatile unsigned char* const tail = c->tail;
    volatile unsigned char* const p = (tail == c->buffer) ?
        c->buffer_max : tail - 1;
    c->tail = p;
    return *p;
}

// increments head
static inline unsigned char cirq_pop_front(struct cirq* const c)
{
    volatile unsigned char* const p = c->head;
    c->head = (p == c->buffer_max) ? c->buffer : p + 1;
    return *p;
}

static inline unsigned char cirq_peek_back(const struct cirq* const c,
    const size_t index)
{
    volatile unsigned char* const tail = c->tail;
    volatile unsigned char* p;
    if (tail == c->buffer)
    {
        p = c->buffer_max - index;
    }
    else
    {
        volatile unsigned char* const f = tail - 1;
        size_t offset = f - c->buffer;
        p = (offset >= index) ? f - index : c->buffer_limit - (index - offset);
    }
    return *p;
}

static inline unsigned char cirq_peek_front(const struct cirq* const c,
    const size_t index)
{
    volatile unsigned char* const head = c->head;
    size_t offset = c->buffer_limit - head;
    volatile unsigned char* const p = (offset > index) ? head + index :
        c->buffer + (index - offset);
    return *p;
}

static inline void cirq_place_back(struct cirq* const c,
    const unsigned char item, const size_t index)
{
    volatile unsigned char* const tail = c->tail;
    volatile unsigned char* p;
    if (tail == c->buffer)
    {
        p = c->buffer_max - index;
    }
    else
    {
        volatile unsigned char* const f = tail - 1;
        size_t offset = f - c->buffer;
        p = (offset >= index) ? f - index : c->buffer_limit - (index - offset);
    }
    *p = item;
}

static inline void cirq_place_front(struct cirq* const c,
    const unsigned char item, const size_t index)
{
    volatile unsigned char* const head = c->head;
    size_t offset = c->buffer_limit - head;
    volatile unsigned char* const p = (offset > index) ? head + index :
        c->buffer + (index - offset);
    *p = item;
}

#endif
