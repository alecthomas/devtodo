#include "Terminal.h"
namespace term {

// this is SUCH a dodgy hack
static bool forcecolour = false;

void forceColour(bool state) {
	forcecolour = state; 
}

string title(string const &str) {
	return string("]0;") + str + "";
}

string background(Colour colour) {
	return string("[4") + str::stringify(colour) + "m";
}

string foreground(Colour colour) {
	return string("[3") + str::stringify(colour) + "m";
}

string colour(Colour colour) {
	return string("[3") + str::stringify(colour) + "m";
}

string attribute(Attribute attribute) {
	return string("[") + str::stringify(attribute) + "m";
}

ostream &black(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[30m";
	return os;
}

ostream &red(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[31m";
	return os;
}

ostream &green(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[32m";
	return os;
}

ostream &yellow(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[33m";
	return os;
}

ostream &blue(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[34m";
	return os;
}

ostream &magenta(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[35m";
	return os;
}

ostream &cyan(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[36m";
	return os;
}

ostream &white(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[37m";
	return os;
}

ostream &normal(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[0m";
	return os;
}

ostream &bold(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[1m";
	return os;
}

ostream &halfbright(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[2m";
	return os;
}

ostream &underline(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[4m";
	return os;
}

ostream &blink(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[5m";
	return os;
}

ostream &reverse(ostream &os) {
	if (&cout == &os && (forcecolour || isatty(1))) os << "[7m";
	return os;
}

}
