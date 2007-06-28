#include <cstdio>
#include "Strings.h"

namespace str {

void wraptext(ostream &out, string const &in, unsigned indent, unsigned initial, unsigned width) {
char const *p = in.c_str();
int ind = indent - initial,
	wid = width - initial;

	if (initial < indent) wid = width - indent;

	while (*p) {
	int count;
	bool newlines = false;

		if (*p && ind > 0) {
			out << string(ind, ' ');
			ind = indent;
		}
			
		for (count = 0; p[count] != 0 && p[count] != '\n' && count < wid; count++) ;
		if (p[count] == '\n') newlines = true;
		// move back to last space seperation
		while (p[count] && !isspace(p[count]) && count) count--;
		if (count == 0)
			for (count = 0; p[count] != 0 && p[count] != '\n' && count < wid; count++) ;
		for (int i = 0; i < count; i++) out << *p++;
		// skip spaces
		while (*p != 0 && isspace(*p)) {
			if (*p == '\n') out << endl;
			p++;
		}
		if (*p && !newlines) out << endl;
		initial = 0;
		ind = indent;
		wid = width - ind;
	}
}

string ucfirst(string const &str) {
string out;	
bool newword = true;
	
	for (string::const_iterator i = str.begin(); i != str.end(); i++)
		if (newword && isalpha(*i)) {
			out += toupper(*i);
			newword = false;
		} else {
			if (isspace(*i) || ispunct(*i)) {
				newword = true;
			}
			out += *i;
		}
	return out;
}

string exec(string const &command) {
FILE *pipe = popen(command.c_str(), "r");
char buffer[1024];
string out;

	if (!pipe) throw runtime_error("exec(): failed, " + string(strerror(errno)));
	while (fgets(buffer, 1024, pipe))
		out += buffer;
	fclose(pipe);
	return out;
}

string join(string const &delim, vector<string> const &components) {
string out;
vector<string>::const_iterator end = components.end();

	end--;
	for (vector<string>::const_iterator i = components.begin(); i != components.end(); i++) {
		out += (*i);
		if (i != end)
		out += delim;
	}
	return out;
}

vector<string> split(string const &delim, string const &text) {
unsigned start = 0;
vector<string> out;

	for (unsigned i = start; i < text.size(); i++)
		if (!strncmp(text.c_str() + i, delim.c_str(), delim.size())) {
			out.push_back(text.substr(start, i - start));
			start = i + delim.size();
			i += delim.size() - 1;
		}
	out.push_back(text.substr(start));
	return out;
}

string replace(string const &match, string const &repl, string const &in) {
string out;
string::size_type found = 0, lastfound = 0;

	while ((found = in.find(match, found)) != string::npos) {
		out += in.substr(lastfound, found - lastfound);
		out += repl;
		found += match.size();
		lastfound = found;
	}
	out += in.substr(lastfound);

	return out;
}

string htmlify(string const &_str) {
string out;
char const *str = _str.c_str();

	for (unsigned i = 0; i < _str.size(); i++)
		switch (str[i]) {
			case '<' : out += "&lt;"; break;
			case '>' : out += "&gt;"; break;
			case '&' : out += "&amp;"; break;
			default : out += str[i]; break;
		}
	return out;
}

string unhtmlify(string const &_str) {
string out;
char const *str = _str.c_str();

	for (unsigned i = 0; i < _str.size(); i++)
		if (str[i] == '&') {
			++i;
			if (!strncmp(str + i, "amp;", 4)) { i += 3; out += '&'; }
			else if (!strncmp(str + i, "gt;", 3)) { i += 3; out += '>'; }
			else if (!strncmp(str + i, "lt;", 3)) { i += 3; out += '<'; }
			else {
				out += '&';
				out += str[i];
			}
		} else
			out += str[i];
	return out;
}

string addcslashes(string const &_str) {
string out;
char const *str = _str.c_str();

	for (unsigned i = 0; i < _str.size(); ++i)
		switch (str[i]) {
			case '\n' : out += "\\n"; break;
			case '\r' : out += "\\r"; break;
			case '\t' : out += "\\t"; break;
			case '"' : out += "\\\""; break;
			case '\\' : out += "\\\\"; break;
			default : out += str[i]; break;
		}
	return out;
}

string stripcslashes(string const &_str) {
string out;
char const *str = _str.c_str();

	for (unsigned i = 0; i < _str.size(); ++i)
		if (str[i] == '\\') {
			++i;
			switch (str[i]) {
				case 'n' : out += '\n'; break;
				case 'r' : out += '\r'; break;
				case 't' : out += '\t'; break;
				case '"' : out += '"'; break;
				case '\\' : out += '\\'; break;
				default : out += str[i]; break;
			}
		} else
			out += str[i];
	return out;
}

}
