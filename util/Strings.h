#ifndef CRASH_STRINGS
#define CRASH_STRINGS

#include <cstdio>
#include <cstring>
#include <string>
#include <cerrno>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace str {

string join(string const &delim, vector<string> const &components);

vector<string> split(string const &delim, string const &text);

string replace(string const &match, string const &repl, string const &in);

inline int count(string const &in, string const &delim = " \t\n\r") {
int count = 1;

	for (unsigned i = 0; i < in.size(); i++)
		if (delim.find(in[i]) != string::npos) {
			count++;
			i += delim.size() - 1;
		}
	return count;
}

inline string dirname(string const &filename) {
int slash = filename.rfind('/');

	if (slash == -1) return ".";
	return filename.substr(0, slash);
}

string exec(string const &command);

string ucfirst(string const &str);

inline string reverse(string const &str) {
string out;

	for (string::const_reverse_iterator i = str.rbegin(); i != str.rend(); i++)
		out += *i;
	return out;
}

inline string uppercase(string const &str) {
string out;

	for (string::const_iterator i = str.begin(); i != str.end(); i++)
		out += toupper(*i);
	return out;
}

inline string lowercase(string const &str) {
string out;

	for (string::const_iterator i = str.begin(); i != str.end(); i++)
		out += tolower(*i);
	return out;
}

inline string invertcase(string const &str) {
string out;

	for (string::const_iterator i = str.begin(); i != str.end(); i++)
		out += islower(*i) ? toupper(*i) : tolower(*i);
	return out;
}

inline string ltrim(string const &str) {
	for (unsigned i = 0; i < str.size(); i++)
		if (!isspace(str[i]))
			return str.substr(i);
	return "";
}

inline string rtrim(string const &str) {
	for (int i = str.size() - 1; i >= 0; i--)
		if (!isspace(str[i]))
			return str.substr(0, i + 1);
	return "";
}

inline string trim(string const &str) {
	return rtrim(ltrim(str));
}

///	Convert all HTML-able characters. ie. >, < and &
string htmlify(string const &str);

///	Convert HTML digraphs to their character forms.
string unhtmlify(string const &str);

///	Convert all C escapable characters to their escaped form.
string addcslashes(string const &str);

///	Convert escaped characters to their original forms.
string stripcslashes(string const &str);

void wraptext(ostream &out, string const &in, unsigned indent = 0, unsigned initialindent = 0, unsigned width = 80);

inline string basename(string const &filename) {
	return filename.substr(filename.rfind('/') + 1);
}

/**
	Convert a type to a string. A simple, but convenient, wrapper around
	ostrstream. If a type has a ostream << operator associated with it,
	this function will work.

	@parameter var Variable to convert.
*/
template <typename T> string stringify(T const &t) {
ostringstream os;
	os << t;
	return os.str();
}

/**
	Convert a string to a type. Requires that the type have an istream >>
	operator assoicated with it.

	eg.
	string s = "10";
	int i;
	i = destringify<int>(s);

	@parameter str Strings to convert to type T.
*/
template <typename T> T destringify(string const &str) {
istringstream os(str);
T t;

	os >> t;
	if (!os.good() && !os.eof()) throw runtime_error("can't destringify '" + str + "'");
	return t;
}

}
#endif
