#ifndef ENCODE_H
#define ENCODE_H

#include <stddef.h>

/*
 * Copyright 2010-2011 Brian Uechi <buasst@gmail.com>
 *
 * This file is part of mochad.
 *
 * mochad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * mochad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mochad.  If not, see <http://www.gnu.org/licenses/>.
 */

int processcommandline(int fd, char *aLine);

typedef struct cm15a_encode_state {
    char remainder[80];
    size_t remlen;
    int discarding;
} cm15a_encode_state_t;

void cm15a_encode_state_init(cm15a_encode_state_t *state);

void cm15a_encode_with_state(int fd, cm15a_encode_state_t *state,
        unsigned char *buf, size_t buflen);

void cm15a_encode(int fd, unsigned char * buf, size_t buflen);

#endif
