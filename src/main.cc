#include "TodoDB.h"
#include "support.h"
#include "config.h"
#include "todorl.h"

void joinArgs(TodoDB &todo, vector<string> const &args, int argc, char const **argv) {
char const *av[args.size() + argc];
int arg = 0;

	av[0] = argv[arg++];
	for (vector<string>::const_iterator i = args.begin(); i != args.end(); i++)
		av[arg++] = (*i).c_str();
	for (int i = 1; i < argc; i++)
		av[arg++] = argv[i];
	parseArgs(todo, arg, av);
}

int main(int argc, char const **argv) {
	// Initialise readline
	rl_readline_name = PACKAGE;
	rl_initialize();

string database;

	try {
	TodoDB todo;

		// Combine arguments from RC and command line
	vector<string> rcargs = parseRC();
		joinArgs(todo, rcargs, argc, argv);

		// Default database to use
		database = options.database;

		// Load todo db
		try {
			// Switch to global database
			if (options.global) {
				database = options.globaldatabase;
				if (options.verbose > 1)
					cout << "todo: switched to global database '" << database << "'" << endl;
			}
			if (options.verbose > 1)
				cout << "todo: attempting load of '" << database << "'" << endl;
			todo.load(database);
		} catch (TodoDB::quit &e) {
			// Quit is thrown if the .todo file can't be accessed
			if (options.verbose > 1)
				cout << "todo: no database found, continuing" << endl;
		} catch (...) {
			throw;
		}
		// perform the relevant action
		todo(options.mode);
		// save if modified (ie. not just viewing)
		if (options.mode != TodoDB::View) {
			todo.save(database);
			if (options.verbose > 1)
				cout << "todo: saved database to '" << database << "'" << endl;
		}
	} catch (exception &e) {
		cerr << "todo: error, " << e.what() << endl;
		return 1;
	}
	return 0;
}
