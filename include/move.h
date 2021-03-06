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

#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>

#include "position.h"

/* 0-5 source square.
 * 6-11 target square.
 * 12-13 flag, 0: none, 1: en passant, 2: promotion, 3: castle.
 * 14-15 promotion piece, 0: knight, 1: bishop, 2: rook, 3: queen.
 * 16-18 capture, 0: no piece, 1: pawn, 2: knight, 3: bishop, 4: rook, 5: queen.
 * 19-23 available castles before move. First bit, 0: K, 1: Q, 2: k, 3: q.
 * 24-29 en passant square before move.
 * 30-35 halfmove.
 * 36-63 fullmove.
 */
typedef uint64_t move;

/* uint32_t fastest? */
static inline uint8_t move_from(move *m) { return *m & 0x3F; }
static inline uint8_t move_to(move *m) { return (*m >> 0x6) & 0x3F; }
static inline uint8_t move_flag(move *m) { return (*m >> 0xC) & 0x3; }
static inline uint8_t move_promote(move *m) { return (*m >> 0xE) & 0x3; }
static inline uint8_t move_capture(move *m) { return (*m >> 0x10) & 0x7; }
static inline uint8_t move_castle(move *m) { return (*m >> 0x13) & 0xF; }
static inline uint8_t move_en_passant(move *m) { return (*m >> 0x18) & 0x3F; }
static inline uint8_t move_halfmove(move *m) { return (*m >> 0x1E) & 0x3F; }
static inline uint32_t move_fullmove(move *m) { return (*m >> 0x24); }
static inline void move_set_captured(move *m, uint8_t i) { *m |= (i << 0x10); }
static inline void move_set_castle(move *m, uint8_t i) { *m |= (i << 0x13); }
static inline void move_set_en_passant(move *m, uint8_t i) { *m |= (i << 0x18); }
static inline void move_set_halfmove(move *m, uint8_t i) { *m |= (i << 0x1E); }
static inline void move_set_fullmove(move *m, uint64_t i) { *m |= (i << 0x24); }

void do_move(struct position *pos, move *m);

void undo_move(struct position *pos, move *m);

void do_move_zobrist(struct position *pos, move *m);

void undo_move_zobrist(struct position *pos, move *m);

static inline move new_move(uint8_t source_square, uint8_t target_square, uint8_t flag, uint8_t promotion) {
	return source_square | (target_square << 0x6) | (flag << 0xC) | (promotion << 0xE);
}

void print_move(move *m);

move string_to_move(struct position *pos, char *str);

#endif
