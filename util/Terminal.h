#ifndef CRASH_TERMINAL
#define CRASH_TERMINAL

#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include "Strings.h"

using namespace std;

namespace term {

enum Colour {
	Black,
	Red,
	Green,
	Brown,
	Blue,
	Magenta,
	Cyan,
	White
};

enum Attribute {
	Normal,
	Bold,
	HalfBright,
	Underline = 4,
	Blink,
	Reverse = 7,
};

void forceColour(bool state);

// Used for getting the string representation of terminal attributes
string background(Colour colour);
string foreground(Colour colour);
string colour(Colour colour);
string attribute(Attribute attribute);
string title(string const &str);

// Stream-oriented terminal attributes
ostream &black(ostream &os);
ostream &red(ostream &os);
ostream &green(ostream &os);
ostream &yellow(ostream &os);
ostream &blue(ostream &os);
ostream &magenta(ostream &os);
ostream &cyan(ostream &os);
ostream &white(ostream &os);

ostream &normal(ostream &os);
ostream &bold(ostream &os);
ostream &halfbright(ostream &os);
ostream &underline(ostream &os);
ostream &blink(ostream &os);
ostream &reverse(ostream &os);

}
#endif
