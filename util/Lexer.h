#ifndef CRASH_LEXER
#define CRASH_LEXER

#include <stdexcept>
#include <string>
#include <map>
#include <iterator>
#include "Strings.h"
#include "Regex.h"

using namespace std;

/**
	Lexer is a lexical analyser.

	02/01/01	Fixed the bug where ignored tokens caused it to die horribly.
	20/09/00	Restarted after I couldn't find an annoying bug. The interface
				needed to be cleaned up anyway.
	24/08/00	Fixed a bug where the list of tokens wasn't being cleared 
				in between calls to the scanner.
	13/08/00	Created.
*/

class Lexer {
	public :
		class exception : public runtime_error {
			public :
				exception(string const &str, unsigned line) : runtime_error(str), _line(line) {}
				unsigned line() { return _line; }
			private :
				unsigned _line;
		};

		class iterator {
			public :
				iterator() : tokeniser(0), in(0), _line(1) {}

				operator string const & () const { return _value; }
				iterator operator ++ (int) {
				iterator i = *this;

					get();
					return i;
				}

				iterator &operator ++ () {
					get();
					return *this;
				}

				int operator != (iterator const &other) const {
					return in != other.in || tokeniser != other.tokeniser;
				}
				int operator == (iterator const &other) const {
					return  in == other.in && tokeniser == other.tokeniser;
				}

				int operator [] (unsigned index) const { return _value[index]; }
				unsigned type() const { return _type; }
				unsigned line() const { return _line; }
				unsigned size() const { return _value.size(); }
				string const &value() const { return _value; }
				char const *source() const { return in; }
				void skip(unsigned skip) { 
					for (unsigned i = 0; i < skip; i++) 
						if (*in) 
						{
							if (*in == '\n') _line++;
							in++; 
						} else 
							break; 
				}
			private :
				void get() {
					assert(in && tokeniser);
					if (!tokeniser->get(*this, in, _line))
						in = 0;
				}
				iterator(Lexer *t, char const *in) : tokeniser(t), in(in), _line(1) {
					if (in) get();
				}
				friend class Lexer;

				Lexer *tokeniser;
				string _value;
				char const *in;
				unsigned _type;
				unsigned _line;
				struct {
					int start, end;
				} match[50];
				int matches;
		};
		friend class iterator;

		Lexer();
		virtual ~Lexer();

		iterator begin(char const *in) { return iterator(this, in); }
		iterator end() { return iterator(this, 0); }

		Lexer &ignorePattern(unsigned index, char const *pattern) { return addPattern(index, pattern, true); }
		Lexer &addPattern(unsigned index, char const *pattern, bool ignore = false);

		void ignore(unsigned index, bool state = true);
		void enable(unsigned index, bool state = true);
	protected :
		enum { Character = 1000000 };

		struct Pattern {
			Regex rx;
			bool ignore, enabled;
		};
		map<int, Pattern> pattern;
		bool initialised;

		virtual bool get(iterator &it, char const *&in, unsigned &line);
};

inline ostream &operator << (ostream &out, Lexer::iterator const &it) {
	out << it.value();
	return out;
}

#endif
