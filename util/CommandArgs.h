#ifndef CRASH_COMMANDARGS
#define CRASH_COMMANDARGS

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include "Strings.h"

using namespace std;

/*
	CommandArgs is a class to parse command line arguments.

	04/02/01	Initial creation
*/

class CommandArgs {
	public :
		class exception : public runtime_error { public : exception(string const &what) : runtime_error(what.c_str()) {} };

		enum Parameter {
			None,
			Required,
			Optional
		};

		CommandArgs();
		~CommandArgs();

		void addArgument(int shortarg, string const &longarg = "", Parameter parameter = None, string const &help = "");
		void setHelp(int shortarg, string const &help);
		void displayHelp(ostream &out, int termwidth = 80);

		class iterator {
			public :
				enum Type {
					Argument,
					Unknown
				};

				iterator() :
					cmdarg(0), inshort(0), _type(Unknown), value(0),
					arg(0), argc(0), _argument(-1), argv(0) {}

				int operator == (iterator const &other) const {
					return argv == other.argv && argc == other.argc;
				}

				iterator operator ++ () {
					if (!get()) {
						argc = -1;
						argv = 0;
					}
					return *this;
				}

				iterator operator ++ (int) {
				iterator j = *this;
					if (!get()) {
						argc = -1;
						argv = 0;
					}
					return j;
				}

				Type type() { return _type; }
				int option() const { return _argument; }
				string const &longOption() const;
				char const *parameter() const { return value; }
			private :
				iterator(CommandArgs *cmdarg, int argc, char const **argv) :
					cmdarg(cmdarg), inshort(0), _type(Unknown), value(0),
					arg(0), argc(argc), _argument(-1), argv(argv) { get(); }

				bool get();

				CommandArgs *cmdarg;
				int inshort;
				Type _type;
				char const *value;
				int arg, argc, _argument;
				char const **argv;

				friend class CommandArgs;
		};

		iterator begin(int argc, char const **argv) {
			return iterator(this, argc - 1, argv + 1);
		}
		iterator end() {
			return iterator(this, -1, 0);
		}
	private :
		friend class iterator;

		// Private, so object can't be default copied.
		CommandArgs(CommandArgs const &copy) {}
		CommandArgs &operator = (CommandArgs const &copy) { return *this; }

		struct Arg {
			int shortarg;
			string longarg, help;
			int argument;
		};
		vector<Arg> args;
};
#endif
