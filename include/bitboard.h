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

#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>

void bitboard_init();

#if __GNUC__
static inline uint64_t ctz(uint64_t b) {
	return __builtin_ctzll(b);
}

static inline uint64_t popcount(uint64_t b) {
	return __builtin_popcountll(b);
}
#elif _MSC_VER
#include <nmmintrin.h>
#if _WIN64
static inline uint64_t ctz(uint64_t b) {
	uint64_t ret;
	_BitScanForward64(&ret, b);
	return ret;
}
#else
static inline uint64_t ctz(uint64_t b) {
	uint64_t ret;
	if (b && 0xFFFFFFFF) {
		_BitScanForward(&ret, b);
		return ret;
	}
	else {
		_BitScanForward(&ret, b >> 32));
		return ret + 32;
	}
}
#endif
static inline uint64_t popcount(uint64_t b) {
	return _mm_popcnt_u64(b);
}
#endif

static inline uint64_t bitboard(int i) {
	return (uint64_t)1 << i;
}

static inline uint64_t get_bit(uint64_t b, int i) {
	return b & bitboard(i);
}

static inline uint64_t set_bit(uint64_t b, int i) {
	return b | bitboard(i);
}

static inline uint64_t clear_bit(uint64_t b, int i) {
	return b & ~bitboard(i);
}

static inline uint64_t clear_ls1b(uint64_t b) {
	return b & (b - 1);
}

void print_bitboard(uint64_t b);

void print_binary(uint64_t b);

static inline uint64_t single(uint64_t b) {
	return !(b & (b - 1));
}

static inline uint64_t insert_zero(uint64_t b, int i) {
	return ((b << 1) & ~(bitboard(i + 1) - 1)) | (b & (bitboard(i) - 1));
}

extern uint64_t between_lookup[64 * 64];
extern uint64_t line_lookup[64 * 64];
extern int castle_lookup[64 * 64 * 16];

static inline int castle(int source_square, int target_square, int castle) {
	return castle_lookup[source_square + 64 * target_square + 64 * 64 * castle];
}

extern const uint64_t FILE_H;
extern const uint64_t FILE_G;
extern const uint64_t FILE_F;
extern const uint64_t FILE_E;
extern const uint64_t FILE_D;
extern const uint64_t FILE_C;
extern const uint64_t FILE_B;
extern const uint64_t FILE_A;
extern const uint64_t FILE_AB;
extern const uint64_t FILE_GH;
extern const uint64_t RANK_8;
extern const uint64_t RANK_7;
extern const uint64_t RANK_6;
extern const uint64_t RANK_5;
extern const uint64_t RANK_4;
extern const uint64_t RANK_3;
extern const uint64_t RANK_2;
extern const uint64_t RANK_1;

static inline uint64_t shift_north(uint64_t b) {
	return b << 8;
}
static inline uint64_t shift_south(uint64_t b) {
	return b >> 8;
}
static inline uint64_t shift_west(uint64_t b) {
	return (b >> 1) & ~FILE_H;
}
static inline uint64_t shift_east(uint64_t b) {
	return (b << 1) & ~FILE_A;
}
static inline uint64_t shift_north_east(uint64_t b) {
	return (b << 9) & ~FILE_A;
}
static inline uint64_t shift_north_west(uint64_t b) {
	return (b << 7) & ~FILE_H;
}
static inline uint64_t shift_south_west(uint64_t b) {
	return (b >> 9) & ~FILE_H;
}
static inline uint64_t shift_south_east(uint64_t b) {
	return (b >> 7) & ~FILE_A;
}
static inline uint64_t shift_north_north(uint64_t b) {
	return b << 16;
}
static inline uint64_t shift_south_south(uint64_t b) {
	return b >> 16;
}

#endif
