#include "CommandArgs.h"
CommandArgs::CommandArgs() {
}

CommandArgs::~CommandArgs() {
}

void CommandArgs::addArgument(int shortarg, string const &longarg, Parameter argument, string const &help) {
Arg arg;

	arg.shortarg = shortarg;
	arg.argument = argument;
	arg.longarg = longarg;
	arg.help = help;

	args.push_back(arg);
}

void CommandArgs::setHelp(int shortarg, string const &help) {
	for (vector<CommandArgs::Arg>::iterator i = args.begin(); i != args.end(); i++)
		if ((*i).shortarg == shortarg) {
			(*i).help = help;
			return;
		}
	throw exception(string("couldn't set help for non-existent option '") + (char)shortarg + "'");
}

bool CommandArgs::iterator::get() {
	if (!argv || arg >= argc) return false;

	_type = Unknown;
	_argument = 0;
	value = 0;
	// continuing existing short parameter list
	if (inshort) {
	char c = argv[arg][inshort];

			for (vector<CommandArgs::Arg>::iterator i = cmdarg->args.begin(); i != cmdarg->args.end(); i++) {
			CommandArgs::Arg const &a = (*i);

				if (a.shortarg == c) {
					_type = Argument;

					// option with a parameter, but still more short args to go
					if (a.argument == Required && argv[arg][inshort + 1] != 0) {
						value = argv[arg] + inshort + 1;
						inshort = 0;
						_argument = c;
						arg++;
						return true;
					} else {
						inshort++;
						// next argument?
						if (argv[arg][inshort] == 0) {
							inshort = 0;
							arg++;
							// optional argument present?
							if (arg < argc && a.argument == Optional && argv[arg][0] != '-') {
								value = argv[arg];
								arg++;
							}
						}
						_argument = c;
						if (a.argument == Required) {
							if (arg >= argc)
								throw exception(string("expected parameter to argument '-") + c + "'");
							value = argv[arg++];
						}
						_type = Argument;
						return true;
					}
				}
			}
		value = argv[arg++];
		return true;
	}

	// check options
	if (argv[arg][0] == '-') {
		for (vector<CommandArgs::Arg>::iterator i = cmdarg->args.begin(); i != cmdarg->args.end(); i++) {
		CommandArgs::Arg a = (*i);

			// long arg?
			if (argv[arg][1] == '-') {
				if (!a.longarg.compare(argv[arg] + 2)) {
					_argument = a.shortarg;
					arg++;
					if (a.argument == Required) {
						if (arg >= argc)
							throw exception(string("expected parameter to argument '--") + a.longarg + "'");
						value = argv[arg++];
					} else // optional argument present?
					if (a.argument == Optional && arg < argc && argv[arg][0] != '-') {
						value = argv[arg];
						arg++;
					}
					_type = Argument;
					return true;
				}
			// short arg?
			} else {
				if (a.shortarg == argv[arg][1]) {
				char c = argv[arg][1];

					if (a.argument == Required && argv[arg][2] != 0) {
						_argument = c;
						_type = Argument;
						value = argv[arg] + 2;
						inshort = 0;
						arg++;
						return true;
					} else {
						inshort = 2;
						// next argument?
						if (argv[arg][inshort] == 0) {
							inshort = 0;
							arg++;
							// optional argument present?
							if (arg < argc && a.argument == Optional && argv[arg][0] != '-') {
								value = argv[arg];
								arg++;
							}
						}
						_argument = c;
						if (a.argument == Required) {
							if (arg >= argc)
								throw exception(string("expected parameter to argument '") + c + "'");
							value = argv[arg++];
						}
						_type = Argument;
						return true;
					}
				}
			}
		}
		value = argv[arg++];
		return true;
	} else {
		_type = Unknown;
		value = argv[arg++];
		_argument = 0;
	}
	return true;
}

void CommandArgs::displayHelp(ostream &out, int termwidth) {
unsigned max = 0;

	// calculate widest argument
	for (vector<CommandArgs::Arg>::iterator i = args.begin(); i != args.end(); i++) {
	Arg &arg = *i;
	string tmp;

		if (arg.help == "") continue;

		if (arg.shortarg > 0) { 
			tmp += '-'; 
			tmp += (char)arg.shortarg; 
		}
		if (arg.longarg != "") {
			if (tmp != "") tmp += ", ";
			tmp += "--" + arg.longarg;
		}
		if (arg.argument == Required) {
			tmp += " ARG";
		} else
		if (arg.argument == Optional) {
			tmp += " [ARG]";
		}
		if (max < tmp.size()) max = tmp.size();
	}

	for (vector<CommandArgs::Arg>::iterator i = args.begin(); i != args.end(); i++) {
	Arg &arg = *i;
	string tmp;

		if (arg.help == "") continue;

		if (arg.shortarg > 0) { 
			tmp += '-'; 
			tmp += (char)arg.shortarg; 
		}
		if (arg.longarg != "") {
			if (tmp != "") tmp += ", ";
			tmp += "--" + arg.longarg;
		}
		if (arg.argument == Required) {
			tmp += " ARG";
		} else
		if (arg.argument == Optional) {
			tmp += " [ARG]";
		}
		out << tmp;
		str::wraptext(out, arg.help, max + 2, tmp.size(), termwidth);
		out << endl;
	}
}
string const &CommandArgs::iterator::longOption() const {
	assert(cmdarg);
	for (vector<CommandArgs::Arg>::iterator i = cmdarg->args.begin(); !(i == cmdarg->args.end()); ++i)
		if (_argument == (*i).shortarg)
			return (*i).longarg;
	throw exception("unknown option index '" + str::stringify(_argument) + "'");
}

