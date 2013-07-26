#ifndef WAC_HANDLER_H_
#define WAC_HANDLER_H_

#include "kvec.h"

typedef kvec_t(unsigned) paramvec_t;

struct handler_struct {
	const char code;
	void (*fn)(paramvec_t *);
};

struct ansi_win_struct {
	unsigned ansi, win;
};

void handle_escape_code(char, paramvec_t *);

#endif // WAC_HANDLER_H_

