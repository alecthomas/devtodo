#ifndef SUPPORT_H__
#define SUPPORT_H__

#include <fstream>
#include <set>
#include <ctime>
#include <map>
#include "TodoDB.h"

using namespace std;

/* Options that can be set per run. */
struct Options {
	Options();

	enum Dir { Negative = -1, Equal = 0, Positive = +1 };

	int verbose, purgeage;
	bool mono, paranoid, global, summary, timeout, comment;
	string text, database, globaldatabase, filename,
		dateformat;
	map<string, string> format;

	struct Filter {
		Filter();

		Dir prioritydir;
		Todo::Priority priority;
		Dir childrendir;
		bool children;
		Dir donedir;
		bool done;
		map<string, Dir> item;
		Dir show;
		Regex search;
	} filter;

	struct Sort {
		Dir dir;
		enum Key { None, Created, Completed, Text, Priority, Duration, Done } key;
	};
	vector<Sort> sort;

	vector<string> index, loaders;
	map<string, vector<string> > event;
	Todo::Priority priority;
	TodoDB::Mode mode;
	typedef map<string, bool (*)(multiset<Todo> &out, string const &title)> Loader;
	int backups, timeoutseconds, columns;
};

extern Options options;

// Turn a numeric priority into a symbolic one
string symbolisePriority(string sym);
string symbolisePriority(Todo::Priority sym);
Todo::Priority desymbolisePriority(string sym);

// text input
string readText(string const &prompt, string existing = "", bool nuke = false);
void addHistory(string text);

// Parse command line arguments
void parseArgs(TodoDB &todo, int argc, char const **argv);
vector<string> parseRC();

// Date stuff
time_t getCurrentDate();
string dateToHuman(time_t time);
string elapsedToHuman(time_t start, time_t end);

// Misc
/// Expand any $<identifier> type environment variables found in a string
string expandEnvars(string const &str);
#endif
