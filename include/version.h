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

char compiler[] =
#if __clang__
"clang "
MACRO_VALUE(__clang_major__)"."
MACRO_VALUE(__clang_minor__)"."
MACRO_VALUE(__clang_patchlevel__)
#elif _MSC_VER
"MSVC "
MACRO_VALUE(_MSC_FULL_VER)"."
MACRO_VALUE(_MSC_BUILD)
#elif __GNUC__
"gcc "
MACRO_VALUE(__GNUC__)"."
MACRO_VALUE(__GNUC_MINOR__)"."
MACRO_VALUE(__GNUC_PATCHLEVEL__)
#else
"unknown"
#endif
#if __MINGW64__
" (MinGW-w64)"
#elif __MINGW32__
" (MinGW-w32)"
#endif
;

char environment[] =
#if __APPLE__
"Apple"
#elif __CYGWIN__
"Cygwin"
#elif _WIN64
"Microsoft Windows 64-bit"
#elif _WIN32
"Microsoft Windows 32-bit"
#elif __linux__
"Linux"
#elif __unix__
"Unix"
#else
"unknown"
#endif
;

/* MSVC does not allow a macro based transformation */
char *date(char *str) {
#ifdef __DATE__
	/* year */
	str[0] = __DATE__[9];
	str[1] = __DATE__[10];
	/* first digit of month */
	/* oct nov dev */
	str[2] = (__DATE__[0] == 'o' || __DATE__[0] == 'n' || __DATE__[0] == 'd') ? '1' : '0';
	/* second digit of month */
	str[3] =
	(__DATE__[2] == 'l') ? '7' : /* jul */
	(__DATE__[1] == 'u') ? '6' : /* jun */
	(__DATE__[2] == 'n') ? '1' : /* jan */
	(__DATE__[2] == 'y') ? '5' : /* may */
	(__DATE__[1] == 'a') ? '3' : /* mar */
	(__DATE__[2] == 'b') ? '2' : /* feb */
	(__DATE__[2] == 'r') ? '4' : /* apr */
	(__DATE__[2] == 'g') ? '8' : /* aug */
	(__DATE__[2] == 'p') ? '9' : /* sep */
	(__DATE__[2] == 't') ? '0' : /* oct */
	(__DATE__[2] == 'v') ? '1' : /* nov */
	'2'; /* dec */
	/* first digit of day */
	str[4] = (__DATE__[4] == ' ') ? '0' : __DATE__[4];
	/* second digit of day */
	str[5] = __DATE__[5];
	str[6] = '\0';
	return str;
#else
	sprintf(str, "unknown");
	return str;
#endif
}

char hash_table_size_str[] = MACRO_VALUE(HASH_TABLE_SIZE)"MiB";
