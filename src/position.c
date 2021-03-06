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

#include "position.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitboard.h"
#include "util.h"
#include "attack_gen.h"
#include "hash_table.h"
#include "move.h"
#include "move_gen.h"

void print_position(struct position *pos) {
	int t;
#ifdef UNICODE
	char *u = " \u2659\u2658\u2657\u2656\u2655\u2654\u265F\u265E\u265D\u265C\u265B\u265A";
#else
	char *u = " PNBRQKpnbrqk";
#endif

	printf("\n       a   b   c   d   e   f   g   h\n");
	for (int i = 0; i < 8; i++) {
		printf("     +---+---+---+---+---+---+---+---+\n   %i |", 8 - i);
		for (int j = 0; j < 8; j++) {
			t = 8 * (7 - i) + j;
			printf(" %c |", u[pos->mailbox[t]]);
		}
		printf(" %i\n", 8 - i);
	}
	printf("     +---+---+---+---+---+---+---+---+\n");
	printf("       a   b   c   d   e   f   g   h\n\n");
}

uint64_t generate_checkers_white(struct position *pos) {
	uint64_t checkers = 0;
	int square;

	square = ctz(pos->white_pieces[king]);
	checkers |= (shift_north_west(pos->white_pieces[king]) | shift_north_east(pos->white_pieces[king])) & pos->black_pieces[pawn];
	checkers |= rook_attacks(square, pos->pieces) & (pos->black_pieces[rook] | pos->black_pieces[queen]);
	checkers |= bishop_attacks(square, pos->pieces) & (pos->black_pieces[bishop] | pos->black_pieces[queen]);
	checkers |= knight_attacks(square) & pos->black_pieces[knight];

	return checkers;
}

uint64_t generate_checkers_black(struct position *pos) {
	uint64_t checkers = 0;
	int square;

	square = ctz(pos->black_pieces[king]);
	checkers |= (shift_south_west(pos->black_pieces[king]) | shift_south_east(pos->black_pieces[king])) & pos->white_pieces[pawn];
	checkers |= rook_attacks(square, pos->pieces) & (pos->white_pieces[rook] | pos->white_pieces[queen]);
	checkers |= bishop_attacks(square, pos->pieces) & (pos->white_pieces[bishop] | pos->white_pieces[queen]);
	checkers |= knight_attacks(square) & pos->white_pieces[knight];

	return checkers;
}

uint64_t generate_attacked(struct position *pos) {
	return pos->turn ? generate_attacked_white(pos) : generate_attacked_black(pos);
}

uint64_t generate_attacked_white(struct position *pos) {
	uint64_t attacked = 0;
	uint64_t piece;
	int square;

	square = ctz(pos->black_pieces[king]);

	attacked = king_attacks(square);
	attacked |= shift_south_west(pos->black_pieces[pawn]);
	attacked |= shift_south_east(pos->black_pieces[pawn]);

	piece = pos->black_pieces[knight];
	while (piece) {
		square = ctz(piece);
		attacked |= knight_attacks(square);
		piece = clear_ls1b(piece);
	}

	piece = pos->black_pieces[bishop] | pos->black_pieces[queen];
	while (piece) {
		square = ctz(piece);
		attacked |= bishop_attacks(square, pos->pieces ^ pos->white_pieces[king]);
		piece = clear_ls1b(piece);
	}

	piece = pos->black_pieces[rook] | pos->black_pieces[queen];
	while (piece) {
		square = ctz(piece);
		attacked |= rook_attacks(square, pos->pieces ^ pos->white_pieces[king]);
		piece = clear_ls1b(piece);
	}

	return attacked;
}

uint64_t generate_attacked_black(struct position *pos) {
	uint64_t attacked = 0;
	uint64_t piece;
	int square;

	square = ctz(pos->white_pieces[king]);

	attacked = king_attacks(square);
	attacked |= shift_north_west(pos->white_pieces[pawn]);
	attacked |= shift_north_east(pos->white_pieces[pawn]);

	piece = pos->white_pieces[knight];
	while (piece) {
		square = ctz(piece);
		attacked |= knight_attacks(square);
		piece = clear_ls1b(piece);
	}

	piece = pos->white_pieces[bishop] | pos->white_pieces[queen];
	while (piece) {
		square = ctz(piece);
		attacked |= bishop_attacks(square, pos->pieces ^ pos->black_pieces[king]);
		piece = clear_ls1b(piece);
	}

	piece = pos->white_pieces[rook] | pos->white_pieces[queen];
	while (piece) {
		square = ctz(piece);
		attacked |= rook_attacks(square, pos->pieces ^ pos->black_pieces[king]);
		piece = clear_ls1b(piece);
	}

	return attacked;
}

uint64_t generate_pinned_white(struct position *pos) {
	uint64_t pinned_all = 0;
	int king_square;
	int square;
	uint64_t rook_pinners;
	uint64_t bishop_pinners;
	uint64_t pinned;

	king_square = ctz(pos->white_pieces[king]);
	rook_pinners = rook_attacks(king_square, pos->black_pieces[all]) & (pos->black_pieces[rook] | pos->black_pieces[queen]);
	bishop_pinners = bishop_attacks(king_square, pos->black_pieces[all]) & (pos->black_pieces[bishop] | pos->black_pieces[queen]);

	while (rook_pinners) {
		square = ctz(rook_pinners);

		pinned = between_lookup[square + 64 * king_square] & pos->white_pieces[all];
		if (single(pinned)) {
			pinned_all |= pinned;
		}

		rook_pinners = clear_ls1b(rook_pinners);
	}

	while (bishop_pinners) {
		square = ctz(bishop_pinners);
		pinned = between_lookup[square + 64 * king_square] & pos->white_pieces[all];

		if (single(pinned)) {
			pinned_all |= pinned;
		}

		bishop_pinners = clear_ls1b(bishop_pinners);
	}

	return pinned_all;
}

uint64_t generate_pinned_black(struct position *pos) {
	uint64_t pinned_all = 0;
	int king_square;
	int square;
	uint64_t rook_pinners;
	uint64_t bishop_pinners;
	uint64_t pinned;

	king_square = ctz(pos->black_pieces[king]);
	rook_pinners = rook_attacks(king_square, pos->white_pieces[all]) & (pos->white_pieces[rook] | pos->white_pieces[queen]);
	bishop_pinners = bishop_attacks(king_square, pos->white_pieces[all]) & (pos->white_pieces[bishop] | pos->white_pieces[queen]);

	while (rook_pinners) {
		square = ctz(rook_pinners);

		pinned = between_lookup[square + 64 * king_square] & pos->black_pieces[all];
		if (single(pinned)) {
			pinned_all |= pinned;
		}

		rook_pinners = clear_ls1b(rook_pinners);
	}

	while (bishop_pinners) {
		square = ctz(bishop_pinners);

		pinned = between_lookup[square + 64 * king_square] & pos->black_pieces[all];
		if (single(pinned)) {
			pinned_all |= pinned;
		}

		bishop_pinners = clear_ls1b(bishop_pinners);
	}
	
	return pinned_all;
}
int square(char *algebraic) {
	if (strlen(algebraic) != 2) {
		return -1;
	}
	if (find_char("abcdefgh", algebraic[0]) == -1 || find_char("12345678", algebraic[1]) == -1)
		return -1;
	return find_char("abcdefgh", algebraic[0]) + 8 * find_char("12345678", algebraic[1]);
}

char *algebraic(char *str, int square) {
	if (square < 0 || 63 < square) {
		str[0] = '-';
		str[1] = '\0';
	}
	else {
		str[0] = "abcdefgh"[square % 8];
		str[1] = "12345678"[square / 8];
	}
	str[2] = '\0';
	return str;
}

char *castle_string(char *str, int castle) {
	int counter = 0;
	for (int i = 0; i < 4; i++)
		if (get_bit(castle, i))
			str[counter++] = "KQkq"[i];
	if (!counter)
		str[counter++] = '-';
	str[counter] = '\0';
	return str;
}

/* assumes that fen is ok */
void pos_from_fen(struct position *pos, int argc, char **argv) {
	int t = 0;
	unsigned long i;

	pos->castle = 0;
	for (i = all; i <= king; i++) {
		pos->white_pieces[i] = 0;
		pos->black_pieces[i] = 0;
	}
	for (i = 0; i < 64; i++)
		pos->mailbox[i] = 0;

	int counter = 56;
	for (i = 0; i < strlen(argv[0]); i++) {
		switch (argv[0][i]) {
		case 'P':
		case 'N':
		case 'B':
		case 'R':
		case 'Q':
		case 'K':
		case 'p':
		case 'n':
		case 'b':
		case 'r':
		case 'q':
		case 'k':
			t = find_char(" PNBRQKpnbrqk", argv[0][i]);
			if (t < 7) {
				pos->white_pieces[t] = set_bit(pos->white_pieces[t], counter);
				pos->mailbox[counter] = t;
			}
			else {
				pos->black_pieces[t - 6] = set_bit(pos->black_pieces[t - 6], counter);
				pos->mailbox[counter] = t;
			}
			counter++;
			break;
		case '/':
			counter -= 16;
			break;
		default:
			counter += find_char(" 12345678", argv[0][i]);
		}
	}
	for (i = all; i <= king; i++) {
		pos->white_pieces[all] |= pos->white_pieces[i];
		pos->black_pieces[all] |= pos->black_pieces[i];
	}
	pos->pieces = pos->white_pieces[all] | pos->black_pieces[all];

	pos->turn = (argv[1][0] == 'w');

	if (-1 < find_char(argv[2], 'K') && find_char(argv[2], 'K') < 4)
		pos->castle = set_bit(pos->castle, 0);
	if (-1 < find_char(argv[2], 'Q') && find_char(argv[2], 'Q') < 4)
		pos->castle = set_bit(pos->castle, 1);
	if (-1 < find_char(argv[2], 'k') && find_char(argv[2], 'k') < 4)
		pos->castle = set_bit(pos->castle, 2);
	if (-1 < find_char(argv[2], 'q') && find_char(argv[2], 'q') < 4)
		pos->castle = set_bit(pos->castle, 3);

	pos->en_passant = square(argv[3]) == -1 ? 0 : square(argv[3]);

	pos->halfmove = 0;
	pos->fullmove = 1;
	if (argc >= 5)
		pos->halfmove = atoi(argv[4]);
	if (argc >= 6)
		pos->fullmove = atoi(argv[5]);

	pos->zobrist_key = 0;
	for (i = 0; i < 64; i++)
		if (pos->mailbox[i])
			pos->zobrist_key ^= zobrist_piece_key(pos->mailbox[i] - 1, i);
	if (pos->turn)
		pos->zobrist_key ^= zobrist_turn_key();
	pos->zobrist_key ^= zobrist_castle_key(pos->castle);
	if (pos->en_passant)
		pos->zobrist_key ^= zobrist_en_passant_key(pos->en_passant);
}

/* half and full move have to be specified */
int fen_is_ok(int argc, char **argv) {
	struct position *pos = NULL;
	int ret = 1;
	int t = 0;
	unsigned long i;

	if (argc < 4)
		goto failure;

	/* string_is_int returns 0 if integer is negative */
	if (argc >= 5)
		if (!string_is_int(argv[4]) || atoi(argv[4]) > 100)
			goto failure;
	if (argc >= 6)
		if (!string_is_int(argv[5]) || atoi(argv[5]) > 6000)
			goto failure;

	int counter = 56;
	int counter_mem = counter + 16;
	int white_king_counter = 0;
	int black_king_counter = 0;
	int current_line = 0;

	int mailbox[64];
	for (i = 0; i < 64; i++)
		mailbox[i] = 0;
	for (i = 0; i < strlen(argv[0]); i++) {
		if (counter < 0)
			goto failure;
		switch (argv[0][i]) {
		case 'K':
			white_king_counter++;
			/* fallthrough */
		case 'k':
			black_king_counter++;
			/* fallthrough */
		case 'P':
		case 'N':
		case 'B':
		case 'R':
		case 'Q':
		case 'p':
		case 'n':
		case 'b':
		case 'r':
		case 'q':
			if (counter > 63 || current_line >= 8)
				goto failure;
			t = find_char(" PNBRQKpnbrqk", argv[0][i]);
			if (t < 7) {
				mailbox[counter] = t;
			}
			else {
				mailbox[counter] = t;
			}
			counter++;
			current_line++;
			break;
		case '/':
			if (counter % 8 || counter + 8 != counter_mem)
				goto failure;
			counter_mem = counter;
			counter -= 16;
			current_line = 0;
			break;
		default:
			if (find_char("12345678", argv[0][i]) == -1 || current_line >= 8)
				goto failure;
			counter += find_char(" 12345678", argv[0][i]);
			current_line += find_char(" 12345678", argv[0][i]);
		}
	}

	/* black_king != 2 because fallthrough */
	if (white_king_counter != 1 || black_king_counter != 2)
		goto failure;

	if (strlen(argv[1]) != 1 || (argv[1][0] != 'w' && argv[1][0] != 'b'))
		goto failure;

	for (i = 0; i < strlen(argv[2]); i++) {
		switch(argv[2][i]) {
		case 'K':
		case 'Q':
		case 'k':
		case 'q':
			if (i > 3)
				goto failure;
			break;
		case '-':
			if (strlen(argv[2]) != 1)
				goto failure;
			break;
		default:
			goto failure;
		}
	}

	if (find_char(argv[2], 'K') != -1)
		if (mailbox[e1] != white_king || mailbox[h1] != white_rook)
			goto failure;
	if (find_char(argv[2], 'Q') != -1)
		if (mailbox[e1] != white_king || mailbox[a1] != white_rook)
			goto failure;
	if (find_char(argv[2], 'k') != -1)
		if (mailbox[e8] != black_king || mailbox[h8] != black_rook)
			goto failure;
	if (find_char(argv[2], 'q') != -1)
		if (mailbox[e8] != black_king || mailbox[a8] != black_rook)
			goto failure;

	for (i = 0; i < strlen(argv[3]); i++) {
		switch(argv[3][i]) {
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
			if (i != 0 || strlen(argv[3]) != 2)
				goto failure;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			if (i != 1 || strlen(argv[3]) != 2)
				goto failure;
			break;
		case '-':
			if (strlen(argv[3]) != 1)
				goto failure;
			break;
		default:
			goto failure;
		}
	}

	t = square(argv[3]);
	if (t != -1) {
		if (t / 8 == 2) {
			if (mailbox[t + 8] != white_pawn ||
				mailbox[t] != empty ||
				mailbox[t - 8] != empty ||
				argv[2][0] != 'w')
				goto failure;
		}
		else if (t / 8 == 5) {
			if (mailbox[t - 8] != black_pawn ||
				mailbox[t] != empty ||
				mailbox[t + 8] != empty ||
				argv[2][0] != 'b')
				goto failure;
		}
		else {
			goto failure;
		}

	}

	for (i = 0; i < 8; i++)
		if (mailbox[i] % 6 == pawn || mailbox[i + 56] % 6 == pawn)
			goto failure;

	/* now ok to generate preliminary position */
	pos = malloc(sizeof(struct position));
	pos_from_fen(pos, argc, argv);
	swap_turn(pos);
	uint64_t attacked = generate_attacked(pos);
	swap_turn(pos);
	if (pos->turn) {
		if (attacked & pos->black_pieces[king])
			goto failure;
	}
	else {
		if (attacked & pos->white_pieces[king])
			goto failure;
	}

	/* check full and half move? */
	goto no_failure;
failure:;
	ret = 0;
no_failure:;
	free(pos);
	return ret;
}

void random_pos(struct position *pos, int n) {
	char *fen[] = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", "w", "KQkq", "-", "0", "1", };
	pos_from_fen(pos, SIZE(fen), fen);
	move m[256];

	for (int i = 0; i < n; i++) {
		generate_all(pos, m);
		if (!*m)
			return;
		do_move_zobrist(pos, m + rand_int(move_count(m)));
	}
}

char *pos_to_fen(char *fen, struct position *pos) {
	int k = 0;
	char tmp[128];

	unsigned long i = 56, j = 0;
	while (1) {
		if (pos->mailbox[i]) {
			if (j)
				fen[k++] = " 12345678"[j];
			j = 0;
			fen[k++] = " PNBRQKpnbrqk"[pos->mailbox[i]];
		}
		else {
			j++;
		}
		i++;
		if (i == 8) {
			if (j)
				fen[k++] = " 12345678"[j];
			break;
		}
		if (i % 8 == 0)  {
			if (j)
				fen[k++] = " 12345678"[j];
			j = 0;
			fen[k++] = '/';
			i -= 16;
		}
	}
	fen[k++] = ' ';
	fen[k++] = "bw"[pos->turn];
	fen[k++] = ' ';
	castle_string(tmp, pos->castle);
	for (i = 0; i < strlen(tmp); i++)
		fen[k++] = tmp[i];
	fen[k++] = ' ';
	algebraic(tmp, pos->en_passant ? pos->en_passant : -1);
	for (i = 0; i < strlen(tmp); i++)
		fen[k++] = tmp[i];
	fen[k++] = ' ';
	sprintf(tmp, "%i", pos->halfmove);
	for (i = 0; i < strlen(tmp); i++)
		fen[k++] = tmp[i];
	fen[k++] = ' ';
	sprintf(tmp, "%i", pos->fullmove);
	for (i = 0; i < strlen(tmp); i++)
		fen[k++] = tmp[i];
	fen[k++] = '\0';
	return fen;
}
