#ifndef CRASH_REGEX
#define CRASH_REGEX

#include <cassert>
#include <cstring>
#include <string>
#include <map>
#include <utility>
#include <stdexcept>
#include <cassert>
#include <sys/types.h>
#include <regex.h>

#ifndef CRASH_REGEX_CACHE_THRESHOLD
#define CRASH_REGEX_CACHE_THRESHOLD 128
#endif

using namespace std;


/**
	Regex is a C++ wrapper around the POSIX Regex library.

	13/00/01	Added cache. This speeds up general rx construction considerably.
	14/08/00	Created.
*/

class Regex {
	public :
		struct exception : public runtime_error { exception(string const &what) : runtime_error(what) {} };
		struct out_of_range : public exception { out_of_range(string const &what) : exception(what) {} };
		struct no_match : public exception { no_match(string const &what) : exception(what) {} };

		Regex();
		Regex(char const *regex);
		Regex(Regex const &copy);
		~Regex();

		Regex &operator = (Regex const &copy);
		Regex &operator = (char const *regex);

		string const &source() const { return inrx; }

		/*
			Regex regex("'([^']*)'");
			string out;

				out = regex.transform("'alec thomas'", "(\\1)");
				// outputs: (alec thomas)
		*/
		string transform(string const &in, string const &mask);

		int match(char const *str);
		int operator == (char const *str) { return match(str); }
		int matchStart(char const *str);
		int operator <= (char const *str) { return matchStart(str); }

		int substrings() {
			for (int i = 0; i < 50; i++)
				if (matches[i].rm_so == -1) return i;
			return 50;
		}

		int subStart(unsigned index) {
			assert(index < 50 && matches[index].rm_so != -1);
			return matches[index].rm_so;
		}
		int subEnd(unsigned index) {
			assert(index < 50 && matches[index].rm_so != -1);
			return matches[index].rm_so;
		}
	private :
		string inrx;
		regex_t regex;
		regmatch_t matches[50];

		struct Cache {
			Cache() : hits(0), instances(0) {}

			regex_t rx;
			int hits, instances;
		};
		friend struct Cache;

//		static map<string, Cache> cache;
};
#endif
