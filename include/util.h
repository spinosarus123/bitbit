/* bitbit, a bitboard based chess engine written in c.
 * Copyright (C) 2022 Isak Ellmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "move.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

#define SIZE(x) (sizeof(x) / sizeof (*(x)))

#define UNUSED(x) (void)(x)

/* exploit of how macro expansions work */
#define MACRO_NAME(x) #x
#define MACRO_VALUE(x) MACRO_NAME(x)

uint64_t rand_uint64();

int rand_int(int i);

int find_char(char *s, char c);

int string_is_int(char *s);

void merge_sort(move *arr, int16_t *val, unsigned int start, unsigned int end, int increasing);

void merge(move *arr, int16_t *val, unsigned int first, unsigned int last, int increasing);

void util_init();

#endif
