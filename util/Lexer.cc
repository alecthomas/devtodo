#include "Lexer.h"

Lexer::Lexer() : initialised(false)
{
}

Lexer::~Lexer()
{
}

Lexer &Lexer::addPattern(unsigned index, char const *in, bool ignore)
{
	// This was to allow statically allocated Lexer's. If regexp's were
	// initialised globally, they may have been constructed before
	// the Regex::cache was initialised...this caused segfaults. Bad.
	if (!initialised)
	{
	Pattern &p = pattern[Character];

		try {
			p.rx = ".";
			p.ignore = false;
			p.enabled = true;
		} catch (Regex::exception &e) {
			throw runtime_error("Lexer::Lexer: index " + str::stringify(index) + ", " + string(e.what()));
		}
		initialised = true;
	}

	if (index < 256) throw runtime_error("pattern indices under 256 are reserved, relevant pattern is '" + string(in) + "'");

Pattern &p = pattern[index];

	try {
		p.rx = in;
		p.ignore = ignore;
		p.enabled = true;
	} catch (Regex::exception &e) {
		throw runtime_error("Lexer::addPattern: index " + str::stringify(index) + ", " + string(e.what()));
	}

	return *this;
}

bool Lexer::get(iterator &it, char const *&in, unsigned &line)
{
	if (*in == 0) 
		return false;

bool ignore;

	do {
		ignore = false;
		for (map<int, Pattern>::iterator i = pattern.begin(); i != pattern.end(); ++i)
			if ((*i).second.enabled)
			{
			Pattern &p = (*i).second;
			int length;

				if ((length = p.rx.matchStart(in)) != -1)
				{
					for (int j = 0; j < length; ++j)
						if (in[j] == '\n') line++;
					if (p.ignore)
					{
						in += length;
						ignore = true;
						if (*in == 0) return false;
						break;
					} else
					{
						it._value = string(in, length);
						it._line = line;
						// Handle individual characters
						if ((*i).first == Character)
							it._type = *in;
						else
							it._type = (*i).first;
						it.in = in;
						in += length;
						return true;
					}
				}
			}
	} while (ignore);
	return false;
}

void Lexer::ignore(unsigned index, bool state)
{
	if (pattern.find(index) == pattern.end())
		throw exception("tried to ignore/unignore an unknown token", 0);
	pattern[index].ignore = state;
}

void Lexer::enable(unsigned index, bool state)
{
	if (pattern.find(index) == pattern.end())
		throw exception("tried to enable/disable an unknown token", 0);
	pattern[index].enabled = state;
}

