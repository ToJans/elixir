#include <stdio.h>
#include <windows.h>

#include "handler.h"
#include "util.h"

const short FOREGROUND_COLORS = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
const short BACKGROUND_COLORS = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;

extern short get_console_attribs();
extern short initial_console_attribs;
static void handle_ich(paramvec_t *);
static void handle_ed(paramvec_t *);
static void handle_sgr(paramvec_t *);

void handle_escape_code(char escape_code, paramvec_t *params)
{
	static struct handler_struct handlers[] = {
		{ '@', handle_ich }, // Insert Character (CH)
		{ 'J', handle_ed }, // Erase in Display (ED)
		{ 'm', handle_sgr }, // Character Attributes (SGR)
	};

	static size_t handler_count = ARRAY_SIZE(handlers);
	for (size_t i = 0; i < handler_count; ++i) {
		struct handler_struct *p = handlers+i;
		if (p->code == escape_code) {
			p->fn(params);
			break;
		}
	}
}

static void handle_ich(paramvec_t *params)
{
	unsigned length;

	size_t params_length = kv_size(*params);
	if (params_length == 0) {
		length = 1;
	} else {
		length = kv_A(*params, 0);
	}

	for (; length; --length) {
		putc(' ', stdout);
	}
}

static void handle_ed(paramvec_t *params)
{
	printf("(ED)");
}

static int ansi2win_code(unsigned ansi_code)
{
    int win_code = -1;

    static struct ansi_win_struct ansi_win_map[] = {
		// Foreground codes
		{ 31, FOREGROUND_RED },
		{ 32, FOREGROUND_GREEN },
		{ 33, FOREGROUND_RED | FOREGROUND_GREEN },
		{ 34, FOREGROUND_BLUE },
		{ 35, FOREGROUND_RED | FOREGROUND_BLUE },
		{ 36, FOREGROUND_GREEN | FOREGROUND_BLUE },
		{ 37, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
		// Background codes
		{ 41, BACKGROUND_RED },
		{ 42, BACKGROUND_GREEN },
		{ 43, BACKGROUND_RED | BACKGROUND_GREEN },
		{ 44, BACKGROUND_BLUE },
		{ 45, BACKGROUND_RED | BACKGROUND_BLUE },
		{ 46, BACKGROUND_GREEN | BACKGROUND_BLUE },
		{ 47, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE },
	};

	static size_t table_size = ARRAY_SIZE(ansi_win_map);
	for (size_t i = 0; i < table_size; ++i) {
		struct ansi_win_struct *p = ansi_win_map + i;
		if (p->ansi == ansi_code) {
			win_code = p->win;
			break;
		}
	}

	return win_code;
}

static void handle_sgr(paramvec_t *params)
{
	short previous_attribs;
	unsigned short ansi_code;
	short win_code;

	size_t params_length = kv_size(*params);
	for (size_t i = 0; i < params_length; ++i) {
		previous_attribs = get_console_attribs();
		ansi_code = kv_A(*params, i);

		if (ansi_code == 0) {
			win_code = initial_console_attribs;
		} else if(ansi_code == 1) {
			win_code = previous_attribs | FOREGROUND_INTENSITY;
		} else if(ansi_code == 2) {
			win_code = previous_attribs & ~FOREGROUND_INTENSITY;
		} else {
			win_code = ansi2win_code(ansi_code);
			if(win_code & FOREGROUND_COLORS) {
				win_code |= previous_attribs & ~FOREGROUND_COLORS;
			} else if(win_code & BACKGROUND_COLORS) {
				win_code |= previous_attribs & ~BACKGROUND_COLORS;
			}
		}

		// Invalid or not supported code
		if (win_code == -1)
			continue;

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), win_code);
	}
}

