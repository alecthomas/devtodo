#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <readline/readline.h>
#include <readline/history.h>
#include "support.h"
#include "TodoDB.h"
#include "Strings.h"
#include "CommandArgs.h"
#include "todoterm.h"
#include "config.h"

using namespace str;

Options options;

Options::Options() :
	verbose(0), purgeage(0), mono(false), paranoid(false), global(false),
	summary(false), timeout(false), comment(false),
	database(".todo"),
	priority(Todo::None),
	mode(TodoDB::View),
	backups(0), timeoutseconds(0) {

	// default formatting strings
	format["date"] = "%c";
	format["display"] = "%4>%i%f%[info]%2n.%[priority]%+1T\n";
	format["comment-display"] = "%4>%i%f%[info]%2n.%[priority]%+1T\n%+1i%[comment](Comment: %C)\n";
	format["verbose-display"] = "%i%[info]%f%2n.%[priority]%+1T\n%+1i%[info]Added: %[normal]%c  %[info]Completed: %[normal]%d\n%+1i%[info]Duration: %[normal]%D  %[info]Priority: %[normal]%p\n\n";
	format["generated"] = "%2>%i-%+1T\n%+1i(added %c, %d, priority %p)\n\n";
	format["verbose-generated"] = "%2>%i- %+1T\n\n%+1iAdded: %c  Completed: %d\n%+1iDuration: %D  Priority: %p\n\n\n";

	loaders.push_back("xml");
	loaders.push_back("binary");

	if (getenv("HOME"))
		globaldatabase = string(getenv("HOME")) + "/.todo_global";
	else
		globaldatabase = "";

	filename = "";

	columns = getWidth();
}

Options::Filter::Filter() {
	// defaults for filter
	childrendir = Equal;
	children = false;
	prioritydir = Equal;
	priority = Todo::None;
	donedir = Negative;
	done = true;
	show = Equal;
}

void parseSort(string const &arg) {
vector<string> keys = str::split(",", arg);
struct {
	const char *str;
	int key;
} table[] = {
	{ "created", Options::Sort::Created },
	{ "completed", Options::Sort::Completed },
	{ "text", Options::Sort::Text },
	{ "priority", Options::Sort::Priority },
	{ "duration", Options::Sort::Duration },
	{ "done", Options::Sort::Done },
	{ "none", Options::Sort::None },
	{ 0, 0 }
};
	options.sort.clear();

	for (vector<string>::iterator i = keys.begin(); i != keys.end(); ++i) {
	string p = *i;
	Options::Sort sort;

		if (p[0] == '-') {
			sort.dir = Options::Negative;
			p = p.substr(1);
		} else {
			sort.dir = Options::Equal;
			if (p[0] == '+') p = p.substr(1);
		}
	int j;
		for (j = 0; table[j].str != 0; ++j)
			if (p == table[j].str) {
				sort.key = (Options::Sort::Key)table[j].key;
				break;
			}
		if (table[j].str == 0)
			throw runtime_error("unknown sort key '" + p + "'");
		options.sort.push_back(sort);
	}
}

void parseFilter(TodoDB &todo, string const &arg) {
vector<string> out;

	out = split(",", arg);
	for (vector<string>::iterator i = out.begin(); i != out.end(); i++) {
	Options::Dir dir = Options::Equal;
	string str = *i;

		// Search
		if (str[0] == '/') {
			options.filter.search = str.c_str() + 1;
		} else {
			if (str[0] == '-' || str[0] == '+' || str[0] == '=') {
				if (str[0] == '-')
					dir = Options::Negative;
				else
				if (str[0] == '+')
					dir = Options::Positive;
				else
					dir = Options::Equal;
				// singular '-' or '+' sets default display mode
				if (str.size() == 1) {
					options.filter.show = dir;
					continue;
				}
				str = str.substr(1);
			}
			// must be children?
			if (str == "children") {
				options.filter.children = true;
				options.filter.childrendir = dir;
			} else
			// ...or done?
			if (str == "done") {
				options.filter.done = true;
				options.filter.donedir = dir;
			} else
			// catch-all, show everything
			if (str == "all") {
				options.filter.done = true;
				options.filter.donedir = Options::Positive;
				options.filter.children = true;
				options.filter.childrendir = Options::Positive;
			} else 
			if (isdigit(str[0])) {
			vector<string> indices = todo.getIndexList(str);

				for (vector<string>::iterator i = indices.begin(); i != indices.end(); ++i) {
					options.filter.item[*i] = dir;
					if (dir != Options::Negative)
						options.filter.show = Options::Negative;
				}
			} else
				// is it a priority?
				try {
				Todo::Priority n = desymbolisePriority(symbolisePriority(str));

					options.filter.priority = n;
					options.filter.prioritydir = dir;
				} catch (logic_error&) {
					throw runtime_error("unexpected symbol '" + str + "' in filter expression");
				}
		}
	}
}

static void mergeVectors(vector<string> &out, vector<string> const &in) {
	for (vector<string>::const_iterator i = in.begin(); i != in.end(); ++i)
		out.push_back(*i);
}

/**
	Parse command line arguments into an associative array of key/value
	pairs.

	@param	argc	argc from main()
	@param	argv	argv from main()
	@return			key/value pairs
*/
void parseArgs(TodoDB &todo, int argc, char const **argv) {
CommandArgs args;
string myname = argv[0];

	// detect default mode from command name
	if (myname.rfind("tda") == myname.size() - 3)
		options.mode = TodoDB::Add;

	if (myname.rfind("tde") == myname.size() - 3)
		options.mode = TodoDB::Edit;

	if (myname.rfind("tdr") == myname.size() - 3)
		options.mode = TodoDB::Remove;

	if (myname.rfind("tdd") == myname.size() - 3)
		options.mode = TodoDB::Done;

	if (myname.rfind("tdl") == myname.size() - 3)
		options.mode = TodoDB::Link;

enum { Help = -100, Remove, Version, Title, Colour, Mono, ForceColour,
	Database, GlobalDatabase, DateFormat, DisplayFormat,
	VerboseDisplayFormat, VerboseGeneratedFormat,
	GeneratedFormat, Sort, Paranoid, DatabaseLoaders, Backup,
	Timeout, Format, UseFormat, DumpConfig, Exec, Echo, Stats, Purge };

	args.addArgument('a', "add", CommandArgs::Optional);
	args.setHelp('a', "Add a note (will prompt for a note if one is not supplied).");
	args.addArgument('l', "link", CommandArgs::Required);
	args.setHelp('l', "Links another todo file to this one.");
	args.addArgument('g', "graft", CommandArgs::Required);
	args.setHelp('g', "In conjunction with --add or --link, graft the new item to the specified item.");
	args.addArgument('R', "reparent", CommandArgs::Required);
	args.setHelp('R', "Change the parent of the first item index to the second item index (separated by a comma). If no second index is given the item is reparented to the root of the tree. eg. todo -R 1,2");
	args.addArgument('p', "priority", CommandArgs::Required);
	args.setHelp('p', "In conjunction with --add, set the default priority (default|veryhigh|high|medium|low|verylow).");
	args.addArgument('e', "edit", CommandArgs::Required);
	args.setHelp('e', "Edit the note that is indexed by the given number.");
	args.addArgument(Remove, "remove", CommandArgs::Required);
	args.setHelp(Remove, "Remove the note that is indexed by the given number.");
	args.addArgument('d', "done", CommandArgs::Required);
	args.setHelp('d', "Mark the specified notes (and all children) as done.");
	args.addArgument('c', "comment");
	args.setHelp('c', "Also manipulate finishing comment for item when viewing or marking as done.");
	args.addArgument('D', "not-done", CommandArgs::Required);
	args.setHelp('D', "Mark the specified notes (and all children) as not done.");
	args.addArgument(GlobalDatabase, "global-database", CommandArgs::Required);
	args.setHelp(GlobalDatabase, "Specify the database to use if the -G (--global) parameter is used.");
	args.addArgument('G', "global");
	args.setHelp('G', "Use the database specified by the --global-database option. Defaults to ~/.todo_global.");
	args.addArgument(Database, "database", CommandArgs::Required);
	args.setHelp(Database, "Change the database from the default (.todo) to the filename specified.");
	args.addArgument('T', "TODO");
	args.setHelp('T', "Generate a typical TODO output text file from a Todo DB.");
	args.addArgument('A', "all");
	args.setHelp('A', "Shortcut for the filter '+done,+children' to show all notes.");
	args.addArgument('f', "filter", CommandArgs::Required);
	args.setHelp('f', "Display only those notes that pass the filter. A filter is composed of the minimum or maximum priority level, the keywords 'done', 'all' or 'children'. Alternatively, a regular expression search filter can be specified with '/<expression>'. eg. '-low,=done,-children' would show any notes with a priority less than or equal to 'low' that are marked as done and would not show child notes.");
	args.addArgument('v', "verbose");
	args.setHelp('v', "Display verbosely.");
	args.addArgument(Colour, "colour", CommandArgs::Required);
	args.setHelp(Colour, "Override default colours of todo items. Multiple colours can be specified, separated by commas. eg. high=red,medium=white. Items are bolded by prefixing them with a '+'.");
	args.addArgument(Mono, "mono");
	args.setHelp(Mono, "Disable all use of colour.");
	args.addArgument(ForceColour, "force-colour");
	args.setHelp(ForceColour, "Force use of colour.");
	args.addArgument(Help, "help");
	args.setHelp(Help, "Display this help.");
	args.addArgument(Version, "version");
	args.setHelp(Version, "Display version of todo.");
	args.addArgument(Title, "title", CommandArgs::Optional);
	args.setHelp(Title, "Set the title of this directory's todo notes.");
	args.addArgument(DateFormat, "date-format", CommandArgs::Required);
	args.setHelp(DateFormat, "Format the display of time values. The format is that used by strftime(3).");
	args.addArgument(Format, "format", CommandArgs::Required);
	args.setHelp(Format, "Specify a display format to use. ARG is in the form <key>=<format-string>.");
	args.addArgument(UseFormat, "use-format", CommandArgs::Required);
	args.setHelp(UseFormat, "Map one format string to another (eg. --use-format display=my-own-format).");
	args.addArgument(DumpConfig, "dump-config");
	//	Individual formatting strings are deprecated
	args.addArgument(DisplayFormat, "display-format", CommandArgs::Required);
	//args.setHelp(DisplayFormat, "Specify the format of items when displaying them to the console.");
	args.addArgument(VerboseDisplayFormat, "verbose-display-format", CommandArgs::Required);
	//args.setHelp(VerboseDisplayFormat, "As with --display-format but used when --verbose (-v) is specified.");
	args.addArgument(GeneratedFormat, "generated-format", CommandArgs::Required);
	//args.setHelp(GeneratedFormat, "Specify the format of items when generating a TODO file from the database.");
	args.addArgument(VerboseGeneratedFormat, "verbose-generated-format", CommandArgs::Required);
	//args.setHelp(VerboseGeneratedFormat, "As with --generated-format but used when --verbose (-v) is specified.");
	args.addArgument(Sort, "sort", CommandArgs::Required);
	args.setHelp(Sort, "Sort the database with the specified expression.");
	args.addArgument(Paranoid, "paranoid");
	args.setHelp(Paranoid, "Be paranoid about some settings, including permissions.");
	args.addArgument(DatabaseLoaders, "database-loaders", CommandArgs::Required);
	args.setHelp(DatabaseLoaders, "Specify order of database format loaders to use when loading and saving (defaults to 'xml').");
	args.addArgument(Backup, "backup", CommandArgs::Optional);
	args.setHelp(Backup, "Enable backups of the database. The optional argument is the number of backups to keep (defaults to 1).");
	args.addArgument('s', "summary");
	args.setHelp('s', "Toggle summary mode.");
	args.addArgument(Timeout, "timeout", CommandArgs::Optional);
	args.setHelp(Timeout, "Set the number of seconds between database displays and/or conform to this setting (see man page for more information).");
	args.addArgument(Purge, "purge", CommandArgs::Optional);
	args.setHelp(Purge, "Purge items marked as done. Optionally only purge completed items older than ARG days.");
	args.addArgument(Exec, "exec", CommandArgs::Required);
	args.addArgument(Echo, "echo", CommandArgs::Required);
	args.addArgument(Stats, "stats");

	for (CommandArgs::iterator i = args.begin(argc, argv); !(i == args.end()); i++)
		switch (i.option()) {
			case Help :
				cout << "usage: " << myname << " [<arguments>]" << endl
					<< "Where <arguments> can be any of the following:" << endl;
				args.displayHelp(cout, options.columns);
				cout << endl
					<< "In addition, there are five convenience symlinks. These are 'tda', 'tdr'," << endl
					<< "'tdd', 'tde', and 'tdl'. For 'tde', 'tdd' and 'tdr' supply an index to edit," << endl
					<< "mark done and remove respectively. For 'tda' supply the text of the todo item" << endl
					<< "item and optionally the priority. For 'tdl' supply the path to another todo" << endl
					<< "file to link in to the current todo file." << endl
					<< "eg. tde 1" << endl;
				exit(0);
			break;
			case Version :
				cout << VERSION << endl;
				exit(0);
			break;
			case Title :
				if (i.parameter()) options.text = i.parameter();
				options.mode = TodoDB::Title;
			break;
			case DateFormat :
				options.format["date"] = i.parameter();
			break;
			case Mono :
				options.mono = true;
			break;
			case Database :
				options.database = i.parameter();
			break;
			case Sort : 
				parseSort(i.parameter());
			break;
			case GlobalDatabase :
				options.globaldatabase = i.parameter();
			break;
			case Backup :
				if (i.parameter()) {
					options.backups = destringify<int>(i.parameter());
					if (options.backups <= 0)
						throw runtime_error("can't specify backup revisions <= 0");
				} else
					options.backups = 1;
			break;
			case Timeout :
				if (i.parameter()) {
					options.timeoutseconds = destringify<int>(i.parameter());
				} else {
					if (options.timeoutseconds == 0) options.timeoutseconds = 10;
					options.timeout = true;
				}
			break;
			case Purge :
				if (i.parameter())
					options.purgeage = destringify<int>(i.parameter());
				options.mode = TodoDB::Purge;
			break;
			case Exec :
				if (options.verbose > 1)
					cout << "todo: executing '" << i.parameter() << "'" << endl;
				system(i.parameter());
			break;
			case Echo :
				cout << i.parameter() << endl;
			break;
			case 's' :
				options.summary = !options.summary;
			break;
			case 'G' :
				options.global = true;
			break;
			case Paranoid :
				options.paranoid = true;
			break;
			case DatabaseLoaders :
				options.loaders = str::split(",", i.parameter());
			break;
			case Colour : {
			vector<string> colours = str::split(",", i.parameter());

				for (vector<string>::iterator j = colours.begin(); j != colours.end(); ++j) {
				vector<string> component = str::split("=", *j);
					if (component.size() != 2) 
						throw runtime_error(string("colour values should be in the format <item>=<colour>, not '") + i.parameter() + "'");
					todo.setColour(component[0], component[1]);
				}
			}
			break;
			case ForceColour :
				term::forceColour(true);
			break;
			case Format : {
			vector<string> comp;
			
				if (!strchr(i.parameter(), '='))
					throw runtime_error("invalid usage of --format");
			string tmp = i.parameter();
				comp.push_back(tmp.substr(0, strchr(i.parameter(), '=') - i.parameter()));
				comp.push_back(tmp.substr(strchr(i.parameter(), '=') - i.parameter() + 1));
				options.format[comp[0]] = comp[1];
			}
			break;
			case UseFormat : {
			vector<string> mapping = str::split("=", i.parameter());
				if (mapping.size() != 2)
					throw runtime_error("invalid usage of --use-format");
				if (options.format.find(mapping[1]) == options.format.end() ||
					options.format.find(mapping[1]) == options.format.end())
					throw runtime_error("no such format '" + mapping[1] + "'");
				options.format[mapping[0]] = options.format[mapping[1]];
			}
			break;
			case DumpConfig : {
				cout << "# todo version " << VERSION << " config dump" << endl;
				cout << "# terminal columns at time of dump: " << options.columns << endl;
				{
				vector<string> temp;

					cout << "filter ";
					if (options.filter.priority != Todo::None) {
					string s;

						if (options.filter.prioritydir == Options::Negative) s = "-";
						else if (options.filter.prioritydir == Options::Positive) s = "+";
						switch (options.filter.priority) {
							case Todo::VeryLow : s += "verylow"; break;
							case Todo::Low : s += "low"; break;
							case Todo::Medium : s += "medium"; break;
							case Todo::High : s += "high"; break;
							case Todo::VeryHigh : s += "veryhigh"; break;
							default : break;
						}
						temp.push_back(s);
					}
					if (options.filter.children)
						if (options.filter.childrendir == Options::Negative)
							temp.push_back("-children");
						else
						if (options.filter.childrendir == Options::Positive)
							temp.push_back("+children");
						else
							temp.push_back("children");
					if (options.filter.done)
						if (options.filter.donedir == Options::Negative)
							temp.push_back("-done");
						else
						if (options.filter.donedir == Options::Positive)
							temp.push_back("+done");
						else
							temp.push_back("done");
					if (options.filter.show == Options::Negative)
						temp.push_back("-");
					else
					if (options.filter.show == Options::Positive)
						temp.push_back("+");
					if (options.filter.search.source() != "")
						temp.push_back("/" + options.filter.search.source());
					cout << str::join(",", temp) << endl;
				}
				cout << "sort ";
				for (vector<Options::Sort>::iterator i = options.sort.begin(); i != options.sort.end();) {
					if ((*i).dir == Options::Negative) cout << '-';
					if ((*i).dir == Options::Positive) cout << '+';
					switch ((*i).key) {
						case Options::Sort::None : cout << "none"; break;
						case Options::Sort::Created : cout << "created"; break;
						case Options::Sort::Completed : cout << "completed"; break;
						case Options::Sort::Text : cout << "text"; break;
						case Options::Sort::Priority : cout << "priority"; break;
						case Options::Sort::Duration : cout << "duration"; break;
						case Options::Sort::Done : cout << "done"; break;
					}
					++i;
					if (i != options.sort.end()) cout << ",";
				}
				cout << endl;
				for (map<string, string>::iterator i = options.format.begin(); i != options.format.end(); ++i)
					cout << "format " << (*i).first << "=" << str::addcslashes((*i).second) << endl;
				if (options.priority != Todo::None)
					cout << "priority " << symbolisePriority(options.priority) << endl;
				if (options.verbose)
					cout << "verbose" << endl;
				if (options.mono)
					cout << "mono" << endl;
				if (options.paranoid)
					cout << "paranoid" << endl;
				if (options.summary)
					cout << "summary" << endl;
				if (options.timeoutseconds)
					cout << "timeout " << options.timeoutseconds << endl;
				if (options.text != "")
					cout << "# user supplied text is '" << options.text << "'" << endl;
				cout << "database " << options.database << endl;
				if (options.global)
					cout << "# using global database..." << endl;
				cout << "global-database " << options.globaldatabase << endl;
				if (options.backups)
					cout << "backup " << options.backups << endl;
				if (options.index.size())
					cout << "# user supplied indices " << str::join(",", options.index) << endl;
				cout << "database-loaders " << str::join(",", options.loaders) << endl;
				for (map<string, vector<string> >::iterator i = options.event.begin(); i != options.event.end(); ++i) {
					cout << "on " << (*i).first << " ";
					cout << str::join(" ", (*i).second).substr(2) << endl;
				}
				exit(0);
			}
			break;
			case VerboseDisplayFormat :
				options.format["verbose-display"] = i.parameter();
			break;
			case VerboseGeneratedFormat :
				options.format["verbose-generated"] = i.parameter();
			break;
			case DisplayFormat :
				options.format["display"] = i.parameter();
			break;
			case GeneratedFormat :
				options.format["generated"] = i.parameter();
			break;
			case Stats :
				options.mode = TodoDB::Stats;
			break;
			case 'v' : 
				options.verbose++;
			break;
			case 'a' : 
				// optional text to add
				if (i.parameter()) options.text = i.parameter();
				options.mode = TodoDB::Add;
			break;
			case 'l' :
				if (i.parameter()) options.filename = i.parameter();
				options.mode = TodoDB::Link;
			break;
			case 'g' : 
				options.index = todo.getIndexList(i.parameter());
				if (options.index.size() > 1)
					cerr << "warning: more than one note index given in graft, ignoring tail" << endl;
			break;
			case 'R' :
				options.index = todo.getIndexList(i.parameter());
				if (options.index.size() > 2)
					throw runtime_error("--reparent accepts a maximum of two comma separated indices");
				options.mode = TodoDB::Reparent;
			break;
			case 'p' : 
				options.priority = desymbolisePriority(symbolisePriority(i.parameter()));
			break;
			case 'e' : 
				options.index = todo.getIndexList(i.parameter());
				if (options.index.size() > 1)
					cerr << "warning: more than one note index given in edit, ignoring tail" << endl;
				options.mode = TodoDB::Edit;
			break;
			case Remove : 
				options.index = todo.getIndexList(i.parameter());
				options.mode = TodoDB::Remove;
			break;
			case 'c' :
				options.comment = true;
			break;
			case 'd' : 
				options.index = todo.getIndexList(i.parameter());
				options.mode = TodoDB::Done;
			break;
			case 'D' : 
				options.index = todo.getIndexList(i.parameter());
				options.mode = TodoDB::NotDone;
			break;
			case 'T' : 
				options.mode = TodoDB::Generate;
			break;
			case 'A' : 
				options.filter.children = true;
				options.filter.childrendir = Options::Positive;
				options.filter.done = true;
				options.filter.donedir = Options::Positive;
			break;
			case 'f' : 
				parseFilter(todo, i.parameter());
			break;
			case 0 :
				// default parameter for tda is the todo text body
				if (options.mode == TodoDB::Add) {
					if (options.text != "")
						options.text += " " + string(i.parameter());
					else
						options.text = i.parameter();
					continue;
				}

				// default paramter for tdr is the index
				if (options.mode == TodoDB::Remove) {
					mergeVectors(options.index, todo.getIndexList(i.parameter()));
					continue;
				}

				// default paramter for tdd is the index
				if (options.mode == TodoDB::Done) {
					mergeVectors(options.index, todo.getIndexList(i.parameter()));
					continue;
				}

				// default parameter for tde is the index
				if (options.mode == TodoDB::Edit) {
					mergeVectors(options.index, todo.getIndexList(i.parameter()));
					continue;
				}

				// default parameter for tdl is the path to a todo file
				if (options.mode == TodoDB::Link) {
					options.filename = i.parameter();
					continue;
				}

				// default parameters for viewing are filters
				if (options.mode == TodoDB::View) {
					try {
						parseFilter(todo, i.parameter());
						continue;
					} catch (...) {
					}
				}

				throw invalid_argument(string("unknown argument '") + i.parameter() + "', try --help");
			break;
		}
}

string symbolisePriority(string sym) {
static struct {
	char const *s, *n;
} s[] = {
	{ "default", "-2" },
	{ "verylow", "5" },
	{ "low", "4" },
	{ "medium", "3" },
	{ "high", "2" },
	{ "veryhigh", "1" },
	{ 0, 0 },
};

	for (int i = 0; s[i].s; i++)
		if (sym == s[i].s || sym == s[i].n) return s[i].s;
	throw logic_error("unknown priority level '" + sym + "'");
}

string symbolisePriority(Todo::Priority sym) {
static struct {
	char const *s;
	Todo::Priority p;
} s[] = {
	{ "default", Todo::Default },
	{ "verylow", Todo::VeryLow },
	{ "low", Todo::Low },
	{ "medium", Todo::Medium },
	{ "high", Todo::High },
	{ "veryhigh", Todo::VeryHigh },
	{ 0, Todo::None },
};

	for (int i = 0; s[i].s; i++)
		if (sym == s[i].p)
			return s[i].s;
	throw logic_error("unknown priority level '" + stringify<int>(sym) + "'");
}

Todo::Priority desymbolisePriority(string sym) {
static struct {
	char const *s, *n;
	Todo::Priority p;
} s[] = {
	{ "default", "-2", Todo::Default },
	{ "verylow", "5", Todo::VeryLow },
	{ "low", "4", Todo::Low },
	{ "medium", "3", Todo::Medium },
	{ "high", "2", Todo::High },
	{ "veryhigh", "1", Todo::VeryHigh },
	{ 0, 0, Todo::None },
};

	sym = trim(sym);
	for (int i = 0; s[i].s; i++)
		if (sym == s[i].s || sym == s[i].n)
			return s[i].p;
	throw logic_error("unknown symbolic priority level '" + sym + "'");
}

time_t getCurrentDate() {
	return time(0);
}

string dateToHuman(time_t date) {
// If we have strftime, use it
#ifdef HAVE_STRFTIME
char buffer[64];
	strftime(buffer, 64, options.format["date"].c_str(), localtime(&date));
	return buffer;
#else
string out = ctime(&date);

	return out.substr(0, out.size() - 1);
#endif
}

/* Readline support */
string const *rl_buffer;

static char const *list[] = {
	"veryhigh",
	"high",
	"medium",
	"low",
	"verylow",
	0
};

static char *getMatches(const char *text, int state) {
static int i, len;
char const *t;

	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while ((t = list[i]) != 0) {
		++i;
		if (!strncmp(t, text, len))
			return strdup(t);
	}
	return 0;
}

static char **completion(const char *text, int start, int end) {
	if (start == 0)
		return rl_completion_matches(text, (char*(*)(const char*, int))getMatches);
	return 0;
}

static int init_rl() {
	rl_insert_text(const_cast<char*>(rl_buffer->c_str()));
	rl_attempted_completion_function = (char **(*)(const char*, int, int))completion;
	return 0;
}

string readText(string const &prompt, string existing) {
string out;

	rl_startup_hook = (int(*)())init_rl;
	rl_buffer = &existing;
char const *tmp = readline(const_cast<char*>(prompt.c_str()));
	if (tmp) out = tmp;

	return out;
}

void addHistory(string text) {
	add_history(const_cast<char*>(text.c_str()));
}

static bool rcvalid(string const &str) {
	return str == "verbose" || str == "filter" || str == "priority" ||
		str == "TODO" || str == "colour" || str == "mono" ||
		str == "global-database" || str == "global" ||
		str == "database" || str == "date-format" || 
		str == "sort" || str == "display-format" || 
		str == "generated-format" || str == "verbose-display-format" || 
		str == "verbose-generated-format" ||
		str == "database-loaders" || str == "backup" || 
		str == "summary" || str == "timeoutseconds" ||
		str == "timeout" || str == "format" ||
		str == "use-format" || str == "paranoid" || str == "exec" || 
		str == "echo" || str == "comment" || str == "force-colour";
}

/*
	Order of RC file parsing:
		$TODORC, falling back to SYSCONFDIR/todorc
		$HOME/.todorc
*/
vector<string> parseRC() {
vector<string> rcfile, out;

	// $TODORC
	if (getenv("TODORC")) {
	ifstream test(getenv("TODORC"));

		if (test.bad())
			cerr << "warning: TODORC envar points to invalid file '" << getenv("TODORC") << "'" << endl;
		else
			rcfile.push_back(getenv("TODORC"));
	} else {
	// SYSCONFDIR/todorc (typically /etc/todorc)
	ifstream test(SYSCONFDIR "/todorc");
		
		if (test.good()) rcfile.push_back(SYSCONFDIR "/todorc");
	}

	// $HOME/.todorc
	if (getenv("HOME")) {
	ifstream test((string(getenv("HOME")) + "/.todorc").c_str());

		if (test.good()) 
			rcfile.push_back((string(getenv("HOME")) + "/.todorc").c_str());
	}

	if (!rcfile.size()) return out;

	// Load config information from all RC files
	for (vector<string>::iterator rc = rcfile.begin(); rc != rcfile.end(); ++rc) {
	ifstream in((*rc).c_str());
	char inputline[1024], *line = inputline;

		while (in.getline(line, 1024)) {
		string str;
		istringstream is(line);

			if (is >> str) {
				if (str[0] == '#') continue;

				// Parse events
				if (str == "on") {
				string event;
				vector<string> data;

					is >> event;

					if (event != "add" && event != "remove" && event != "view" &&
						event != "edit" && event != "generate" &&
						event != "done" && event != "done" && event != "notdone" &&
						event != "title" && event != "reparent" && event != "create" &&
						event != "save" && event != "load" && event != "link")
						throw runtime_error(string("unknown event '") + event + "'");

					is >> str;
					// multi-line event ?
					if (str == "{") {
						while (in.getline(line, 1024)) {
						istringstream is(line);

							if (is >> str) {
								if (str[0] == '#') continue;

								if (str[0] == '}') break;

								if (rcvalid(str)) {
									data.push_back("--" + str);
									if (getline(is, str)) {
									string remainder = str::trim(str);
										if (remainder != "") data.push_back(remainder);
									}
								}
							}
						}
					} else
						if (rcvalid(str)) {
							data.push_back("--" + str);
							if (getline(is, str)) {
							string remainder = str::trim(str);
								if (remainder != "") data.push_back(remainder);
							}
						} else
							throw runtime_error(("unrecognised resource '" + str + "' in " + *rc).c_str());
					options.event[event] = data;
				} else
				if (rcvalid(str)) {
				// these are the only valid options in the rc file
					out.push_back("--" + str);
				string remainder = expandEnvars(str::trim(line + str.size()));
					if (remainder != "") out.push_back(remainder);
				} else
					throw runtime_error(("unrecognised resource '" + str + "' in " + *rc).c_str());
			}
		}
	}
	return out;
}

string expandEnvars(string const &str) {
string out;

	for (unsigned i = 0; i < str.size(); ++i)
		if (str[i] == '$') {
			i++;
		unsigned mark = i;
			for (; str[i] != 0 && (isalnum(str[i]) || str[i] == '_'); ++i) ;

			if (i != mark) {
			string var = str.substr(mark, i - mark);

				if (!getenv(var.c_str())) {
					if (options.verbose)
						cout << "notice: no such environment variable '" + var + "'." << endl;
				} else
					out += getenv(var.c_str());
			} else
				out += '$';

			i--;
		} else
			out += str[i];
	return out;
}

string elapsedToHuman(time_t start, time_t end) {
time_t t = end - start;
string ret;

	if (t < 0) return "?? (-ve)";
	if (t == 0) return "No time";

	if (t / (60 * 60 * 24 * 7))
		ret += stringify(t / (60 * 60 * 24 * 7)) + "w ";
	if (t / (60 * 60 * 24))
		ret += stringify((t / (60 * 60 * 24)) % 7) + "d ";
	if (t / (60 * 60))
		ret += stringify((t / (60 * 60)) % 24) + "h ";
	if (t / 60)
		ret += stringify((t / 60) % 60) + "m ";
	//ret += stringify(t % 60) + "s";
	return ret;
}
