#include <ctype.h>
#include <stdio.h>
#include <windows.h>

#include "kvec.h"

#include "handler.h"

short initial_console_attribs;
static void parse_sequence();
short get_console_attribs();

int main()
{
	int c;
	enum { OUTSIDE, ESC, CSI } state = OUTSIDE;
	initial_console_attribs = get_console_attribs();

	while ((c = getc(stdin)) != EOF) {
		// ESC character - \e, often transcribed as ^[
		if (c == '\033') {
			state = ESC;
		// Control Sequence Introducer (CSI) is be triggered either by the sequence
		// ESC + [ or directly by \233
		} else if ((state == ESC && c == '[') || c == '\233') {
			state = CSI;
		} else if (state == CSI) {
			ungetc(c, stdin);
			parse_sequence();
			state = OUTSIDE;
		} else {
			putc(c, stdout);
			if(state != OUTSIDE)
				state = OUTSIDE;
		}
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			initial_console_attribs);
	fflush(stdout);
	fclose(stdout);

	return 0;
}

short get_console_attribs()
{
	struct _CONSOLE_SCREEN_BUFFER_INFO buffer_info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer_info);
	return buffer_info.wAttributes;
}

// Parses the parameters and escape code after control character and calls
// the dispatcher
static void parse_sequence()
{
	unsigned temp_param;
	paramvec_t params;
	kv_init(params);

	// Load (unsigned) integer parameters delimited with semicolon
	while (scanf("%u;", &temp_param)) {
		kv_push(unsigned, params, temp_param);
	}

	int escape_code = getchar();
	if (escape_code == EOF)
		return;

	handle_escape_code(escape_code, &params);

	kv_destroy(params);
}

