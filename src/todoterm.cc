#include "config.h"
#include "todoterm.h"

#ifdef USETERMCAP
#include <iostream>
#include <string>
#include <stdexcept>
#include <curses.h>
#include <term.h>

static char info[2048];
static bool term_initialized;

using namespace std;

int getWidth() {
	if (!term_initialized) {
	char const *termtype = getenv("TERM");
		if (!termtype) {
			cerr << "can't get terminal type, defaulting to vt100." << endl;
			cerr << "please set the TERM env variable." << endl;
			setenv("TERM", "vt100", 0);
		}
	int result = tgetent(info, getenv("TERM"));
		if (result < 0)
			throw runtime_error("could not access termcap database");
		if (result == 0)
			throw runtime_error(string(string("unknown terminal type '") + termtype + "'").c_str());
		term_initialized = true;
	}
	return tgetnum("co");
}
#else

int getWidth() { return 80; }

#endif
