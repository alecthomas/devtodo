
// We don't want this freed by the delete function.
#include <map>
#include "Strings.h"
#include "TodoDB.h"
#include "Loaders.h"
#include "support.h"
#include "config.h"

using namespace term;
using namespace str;

static bool coloursInitialised = false;

map<string, TodoDB::StreamColour> TodoDB::streamColour;

TodoDB::TodoDB() {
	if (!coloursInitialised) {
		initColour();
		coloursInitialised = true;
	}
}

TodoDB::TodoDB(string const &file) {
	initColour();
	load(file);
}

TodoDB::~TodoDB() {
}

void TodoDB::initColour() {
	streamColour["veryhigh"] = StreamColour(red, bold);
	streamColour["high"] = StreamColour(yellow, bold);
	streamColour["medium"] = StreamColour(::normal, StreamColour::mono);
	streamColour["low"] = StreamColour(cyan, StreamColour::mono);
	streamColour["verylow"] = StreamColour(blue, bold);
	streamColour["info"] = StreamColour(green, StreamColour::mono);
	streamColour["title"] = StreamColour(green, bold);
	streamColour["comment"] = StreamColour(white, bold);
}

void TodoDB::initColourPost() {
	if (options.mono) {
		streamColour["veryhigh"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["high"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["medium"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["low"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["verylow"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["info"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["title"] = StreamColour(StreamColour::mono, StreamColour::mono);
		streamColour["comment"] = StreamColour(StreamColour::mono, StreamColour::mono);
		priority[0] = StreamColour::mono;
		priority[1] = StreamColour::mono;
		priority[2] = StreamColour::mono;
		priority[3] = StreamColour::mono;
		priority[4] = StreamColour::mono;
		info = StreamColour::mono;
		title = StreamColour::mono;
		normal = StreamColour::mono;
		comment = StreamColour::mono;
	} else {
		priority[0] = StreamColour::veryhigh;
		priority[1] = StreamColour::high;
		priority[2] = StreamColour::medium;
		priority[3] = StreamColour::low;
		priority[4] = StreamColour::verylow;
		info = StreamColour::info;
		title = StreamColour::title;
		normal = StreamColour::normal;
		comment = StreamColour::comment;
	}

}

void TodoDB::operator () (Mode mode) {
	initColourPost();
	switch (mode) {
		case Add : triggerEvent("add"); add(); break;
		case Link : triggerEvent("link"); link(); break;
		case Remove : triggerEvent("remove"); remove(); break;
		case View : triggerEvent("view"); view(); break;
		case Edit : triggerEvent("edit"); edit(); break;
		case Generate : triggerEvent("generate"); generate(); break;
		case Done : triggerEvent("done"); done(); break;
		case NotDone : triggerEvent("notdone"); notdone(); break;
		case Title : triggerEvent("title"); edittitle(); break;
		case Reparent : triggerEvent("reparent"); reparent(); break;
		case Purge : triggerEvent("purge"); purge(); break;
		default :
			throw exception("unknown action?");
		break;
	}
}

/*	Find an item.

	Items are specified by their number. Sub-items are specified by a .
	followed by their number, and so on. The kleene start can be used to
	match any item at a level although wildcard matching does not actually
	occur here, but in the getIndexList method.
*/
Todo *TodoDB::find(multiset<Todo> const &todo, string const &index) {
int looking = destringify<int>(index);

 	for (multiset<Todo>::const_iterator i = todo.begin(); i != todo.end(); i++)
		if ((*i).index == looking) {
			// Recurse into child
			if (index.find(".") != string::npos) {
				try {
				string ns = index.substr(index.find(".") + 1);

					return find(*i->child, ns);
				} catch (exception &e) {
					throw exception("couldn't find index '" + index + "'");
				}
			}
			return const_cast<Todo*>(&(*i));
		}
	throw exception("couldn't find index '" + index + "'");
}

multiset<Todo> &TodoDB::findContainer(multiset<Todo> &todo, string const &index) {
int looking = destringify<int>(index);

	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); i++)
		if ((*i).index == looking) {
			// Recurse into child
			if (index.find(".") != string::npos) {
				try {
				string ns = index.substr(index.find(".") + 1);

					return findContainer(const_cast<multiset<Todo>& >(*i->child), ns);
				} catch (exception &e) {
					throw exception("couldn't find index '" + index + "'");
				}
			}
			return todo;
		}
	throw exception("couldn't find index '" + index + "'");
}

//	Erase the item with the specified index.
void TodoDB::erase(multiset<Todo> &todo, string const &index) {
int looking;

	looking = destringify<int>(index);
	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); i++)
		if ((*i).index == looking) {
			// Recurse into child
			if (index.find(".") != string::npos) {
				try {
				string ss = index.substr(index.find(".") + 1);
					erase(*const_cast<Todo&>(*i).child, ss);
				} catch (exception &e) {
					throw exception("couldn't find index '" + index + "'");
				}
			} else {
				todo.erase(i);
				setDirty(true);
			}
			return;
		}
	throw exception("couldn't find index '" + index + "'");
}

void TodoDB::edittitle() {
string text;

	if (options.text != "") text = options.text;
	else {
		if (isatty(0)) {
			if (options.verbose)
				cout << info << "Enter text for the title of this todo list." << normal << endl;
			text = readText("text> ", titleText);
		} else {
		string line;

			while (getline(cin, line))
				text += line + '\n';
		}
	}
	titleText = text;

	setDirty(true);
}

void TodoDB::parse(vector<XML*>::const_iterator begin, 
	vector<XML*>::const_iterator end, multiset<Todo> &out) {
	for (vector<XML*>::const_iterator i = begin; i != end; i++) {
	XML &x = *(*i);

		switch (x.type()) {
			case XML::Element : {
				if (x.name() == "title") {
					titleText = trim((*x.child().begin())->body());
					break;
				} else
				if (x.name() == "note") {
				Todo todo;
				// const_cast so I can use attrib[...]
				map<string, string> &attrib = *const_cast<map<string, string>* >(&x.attrib());

					if (attrib.find("priority") == attrib.end() || attrib.find("time") == attrib.end())
						throw exception("require both 'priority' and 'time' attributes for 'note' element");

					if (attrib.find("done") != attrib.end()) {
							todo.done = true;
							todo.doneTime = destringify<time_t>(attrib["done"]);
						} else
							todo.done = false;

					todo.priority = desymbolisePriority(attrib["priority"]);
					todo.text = trim((*x.child().begin())->body());
					todo.added = destringify<time_t>(attrib["time"]);

					parse(x.child().begin(), x.child().end(), *todo.child);

					out.insert(todo);
				} else if (x.name() == "link") {
				TodoDB newDb;
				Todo todo;
				map<string, string> &attrib = *const_cast<map<string, string>* >(&x.attrib());

					if (attrib.find("filename") == attrib.end())
						throw exception("require 'filename' attribute for 'link' element");

					newDb.load(attrib["filename"]);
					
					if (newDb.titleText == "")
						todo.text = attrib["filename"];
					else
						todo.text = newDb.titleText;

					todo.child = &newDb.todo;

					out.insert(todo);
				} else
					throw exception("expected 'note' element, got '" + x.name() + "'");
			}
			break;
			default :
			break;
		}
	}

	// number the items
int n = 1;
	for (multiset<Todo>::iterator i = out.begin(); i != out.end(); ++i, ++n) {
	Todo &t = const_cast<Todo&>(*i);
		
		t.index = n;
	}
}

void TodoDB::fixParents(multiset<Todo> &todo, Todo *parent) {
	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); ++i) {
	Todo &todo = (Todo&)*i;

		if (parent)
			todo.parent = parent;
		fixParents(*todo.child, &todo);
	}
}

void TodoDB::load(string const &file) {
Loader loader;
string lastError;

	setDirty(false);

	stat(file.c_str(), &_stat);
	if (options.timeout && options.mode == View && (time(0) - _stat.st_atime < options.timeoutseconds)) {
		if (options.verbose)
			cout << "todo: database not displayed due to timeout" << endl;
		return;
	}

	options.timeout = false;

	filename = file;

	/* Get the base name */
string::size_type pos = filename.rfind("/");
	
	if (pos > 0 && pos != string::npos)
		basepath = filename.substr(0, pos);

ifstream in(file.c_str());

	statSuccessful = false;
	if (in.bad()||in.fail()||in.eof()) throw quit();
	statSuccessful = true;

	in.close();

	loader = getLoaders();

	for (vector<string>::iterator i = options.loaders.begin(); i != options.loaders.end(); ++i) {
		if (options.verbose > 1)
			cout << "todo: trying '" << (*i) << "' loader" << endl;
		if (loader.find(*i) == loader.end())
			throw exception("couldn't find loader for '" + *i + "'");
		try {
			if (loader[*i](*this, file)) {
				if (options.verbose > 1)
					cout << "todo: loaded database successfully with '" << (*i) << "' loader" << endl;
				triggerEvent("load");
				return;
			}
		} catch (std::exception &e) {
			lastError = e.what();
		}
	}
	throw exception("no database loaders for database format or database corrupt (last error was '" + lastError + "'");
}

void TodoDB::save(multiset<Todo> const &todo, ostream &of, int ind) {

	for (multiset<Todo>::const_iterator i = todo.begin(); i != todo.end(); i++)
	{
		cerr << "saving: " << i->text << endl;
		if (i->type == Todo::Link) {
			of  << string(ind * 4, ' ') << "<link"
			    << " filename=\"" << i->todofile << "\""
				<< " priority=\"" << symbolisePriority(i->priority) << "\""
				<< " time=\"" << i->added << "\""
				<< "/>" << endl;

			if (i->db)
				i->db->save(i->todofile);

			/* Restore the TODODB environment variable. */
string envar = "TODODB=" + filename;

			//setenv("TODODB", options.database.c_str(), 1);
			putenv(strdup(const_cast<char*>(envar.c_str())));

		}
		else {
			of	<< string(ind * 4, ' ') << "<note"
				<< " priority=\"" << symbolisePriority((*i).priority) << "\""
				<< " time=\"" << (*i).added << "\"";
			if ((*i).done) {
				of	<< " done=\"" << (*i).doneTime<< "\"";
			}
			of << ">" << endl;
			of << string((ind + 1) * 4, ' ');
			of << htmlify((*i).text) << endl;
			if ((*i).comment != "") {
				of << "<comment>" << endl;
				of << string((ind + 2) * 4, ' ');
				of << htmlify((*i).comment) << endl;
				of << "</comment>" << endl;
			}
			save(*i->child, of, ind + 1);
			of << string(ind * 4, ' ');
			of << "</note>" << endl;
		}
	}
}

void TodoDB::save(string const &file) {
	// Do backups
	if (dirty && options.backups) {
	string newname;
		for (int i = options.backups - 1; i > 0; i--) {
			newname = file + "." + stringify(i + 1);
			if (options.verbose > 1)
				cout << "todo: renaming " << file << "." << i  << " to " << file << "." << i + 1 << endl;
			chmod(newname.c_str(), 0600);
			::unlink(newname.c_str());
			rename((file + "." + stringify(i)).c_str(), newname.c_str());
			chmod(newname.c_str(), 0400);
		}
		if (options.verbose > 1)
			cout << "todo: renaming " << file << " to " << file << ".1" << endl;
		newname = file + ".1";
		chmod(newname.c_str(), 0600);
		::unlink(newname.c_str());
		rename(file.c_str(), newname.c_str());
		chmod(newname.c_str(), 0400);
	}
	if (todo.size() || titleText != "") {
	Saver saver = getSavers();
	string lastError;

		if (dirty && options.verbose > 1)
			cout << "todo: saving to database '" << file << "'" << endl;

		for (vector<string>::iterator i = options.loaders.begin(); i != options.loaders.end(); ++i) {
			if (dirty && options.verbose > 1)
				cout << "todo: trying '" << (*i) << "' saver" << endl;
			if (saver.find(*i) == saver.end())
				throw exception("couldn't find saver for '" + *i + "'");
			try {
				if (saver[*i](*this, file)) {
					// Preserve ownership and mode
					if (statSuccessful) {
						if (options.verbose > 1)
							cout << "todo: preserving attributes" << endl;
						chmod(file.c_str(), _stat.st_mode);
						chown(file.c_str(), _stat.st_uid, _stat.st_gid);
					} else {
						triggerEvent("create");
						if (options.paranoid) {
							if (options.verbose > 1)
								cout << "todo: paranoia check" << endl;
							stat(file.c_str(), &_stat);
							if (_stat.st_mode & 0077)
								cerr << "todo: warning, created database (" << file << ") has group or world permissions" << endl;
						}
					}
					triggerEvent("save");
					return;
				}
				else if (!dirty)
					return;
			} catch (std::exception &e) {
				lastError = e.what();
			}
		}
		throw exception(lastError);
	throw exception(lastError);
	} else {
		if (options.verbose > 1)
			cout << "todo: empty database '" << file << "', unlinking" << endl;
		::unlink(file.c_str());
	}
}

vector<string> TodoDB::getIndexList(string const &str) {
vector<string> tmp = split(",", str), out;

	for (vector<string>::iterator i = tmp.begin(); i != tmp.end(); i++) {
		// wildcard?
		if ((*i)[(*i).size() - 1] == '*') {
		string base = (*i).substr(0, (*i).rfind("."));
		Todo *t = find(todo, base);


			if (!t) throw exception("can't expand non-existant wildcard note '" + (*i) + "'");
			for (multiset<Todo>::iterator j = t->child->begin(); j != t->child->end(); j++)
				out.push_back(base + stringify((*j).index));
		} else
		// is it a range?
		if ((*i).find('-') != string::npos) {
		string start = (*i).substr(0, (*i).find('-')), 
			end = (*i).substr(start.size() + 1);

			if (end.find('.') != string::npos)
				throw exception("ranges are in the form 'x.y.z1-z2' not '" + (*i) + "'");

		int a = destringify<int>(start.substr(start.rfind('.') + 1)),
			b = destringify<int>(end);

			if (b < a) {
			int swap = a;

				a = b;
				b = swap;
			}

		string base = start.substr(0, (*i).rfind('.') + 1);

			for (int i = a; i <= b; ++i)
				out.push_back(base + stringify(i));
		} else
			out.push_back((*i));
	}
	return out;
}

/*	Get a priority level from the user. If the user has passed a priority
	level on the command line it will be used instead of interactively
	requesting a priority level.

	This function allows for tab completion of priority levels as well
	as the levels being in the command history.
*/
Todo::Priority TodoDB::getPriority(string current)
{
char *pri[] = {
	"veryhigh",
	"high",
	"medium",
	"low",
	"verylow",
};

string priority;

	if (isatty(0)) {
		// Display list of priorities if at a tty
		for (int i = 0; i < 5; i++) {
			cout << info << i + 1 << ". " << TodoDB::priority[i] << pri[i] << normal << "   ";
			addHistory(pri[i]);
		}
		cout << endl;
	}
	while (true) {
		if (isatty(0)) {
			if (options.verbose)
				cout << info << "Enter a priority from those listed above." << normal << endl;
			priority = readText("priority> ", current, true);
		} else
			priority = current;
		priority = trim(priority);
		// Default to medium
		if (priority == "") priority = "medium";
		try {
			if (priority.size() == 1 && isdigit(priority[0])) {
			int index = destringify<int>(priority);

				if (index < 1 || index > 5) throw "invalid priority";
				return desymbolisePriority(pri[index - 1]);
			}
			return desymbolisePriority(symbolisePriority(priority));
		} catch (...) {
			cout << "error: invalid priority" << endl;
		}
	}
}

void TodoDB::add() {
string text;
Todo *parent = 0;

	// grafting, get parents priority and use it as default for children
	if (options.index.size()) {
		parent = find(todo, options.index[0]);

		if (!parent) throw exception("couldn't find '" + options.index[0] + "' to graft to"); 
	}

	if (options.text != "")
		text = options.text;
	else
	{
		if (isatty(0)) {
			if (options.verbose)
				cout << info << "Enter text for the item you are adding." << normal << endl;
			text = readText("text> ", text);
		} else {
		string line;

			while (getline(cin, line))
				text += line + '\n';
		}
	}

Todo t;

	t.text = text;

	if (options.priority == Todo::Default)
		if (parent)
			options.priority = parent->priority;
		else
			options.priority = Todo::Medium;

	if (options.priority != Todo::None)
		t.priority = options.priority;
	else {
	string defpri;

		if (parent)
			defpri = symbolisePriority(parent->priority);
		else
			defpri = "medium";
		t.priority = getPriority(defpri);
	}
	t.added = getCurrentDate();
	// grafting
	if (parent) {
		if (options.verbose > 1)
			cout << "todo: grafting new item to item " << options.index[0] << endl;
		parent->child->insert(t);
		parent->db->setDirty(true);

		if (options.verbose) {
		unsigned findindex = 1;

			for (multiset<Todo>::iterator i = parent->child->begin(); i != parent->child->end(); ++i, findindex++)
				if (t == *i)
					cout << info << "Index of new item is " << options.index[0] << "." << findindex << normal << endl;
		}
	} else {
		if (options.verbose > 1)
			cout << "todo: adding new item" << endl;
		todo.insert(t);
		setDirty(true);

		if (options.verbose) {
		unsigned findindex = 1;
			for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); ++i, findindex++)
				if (t == *i)
					cout << info << "Index of new item is " << findindex << normal << endl;
		}
	}

}

void TodoDB::link() {
string filename;
Todo *parent = 0;

	// grafting, get parents priority and use it as default for children
	if (options.index.size()) {
		parent = find(todo, options.index[0]);

		if (!parent) throw exception("couldn't find '" + options.index[0] + "' to graft to");
	}

	if (options.filename != "")
		filename = options.filename;
	else
	{
		if (isatty(0)) {
			if (options.verbose)
				cout << info << "Enter the filename of the todo database to link." << normal << endl;

			filename = readText("filename> ", filename);
		} else
		{
		string line;

			// XXX
			while (getline(cin, line))
				filename += line + '\n';
		}
	}

Todo t;

	t.type = Todo::Link;

	if (parent)
		t.todofile = fixRelativePath(parent->db->basepath, filename);
	else
		t.todofile = filename;

ifstream in(t.todofile.c_str());
	if (!in) throw exception("could not link to '" + t.todofile + "', unreadable/not found");

	if (options.priority == Todo::Default)
		options.priority = Todo::Medium;

	if (options.priority != Todo::None)
		t.priority = options.priority;
	else {
		string defpri;

		if (parent)
			defpri = symbolisePriority(parent->priority);
		else
			defpri = "medium";

		t.priority = getPriority(defpri);
	}

	t.added = getCurrentDate();

	// grafting
	if (parent) {
		if (options.verbose > 1)
			cout << "todo: grafting new item to item " << options.index[0] << endl;

		parent->child->insert(t);
		parent->db->setDirty(true);

		if (options.verbose) {
			unsigned findindex = 1;

				for (multiset<Todo>::iterator i = parent->child->begin(); i != parent->child->end(); ++i, findindex++)
					if (t == *i)
						cout << info << "Index of new item is " << options.index[0] << "." << findindex << normal << endl;
		}
	} else {
		if (options.verbose > 1)
			cout << "todo: adding new item" << endl;

		t.todofile = filename;

		todo.insert(t);

		setDirty(true);

		if (options.verbose) {
		unsigned findindex = 1;

			for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); ++i, findindex++)
				if (t == *i)
					cout << info << "Index of new item is " << findindex << normal << endl;
		}
	}
}

void TodoDB::edit() {
	if (options.index.size() == 0) throw exception("no notes specified to edit");
string const &index = options.index[0];
Todo t = *find(todo, index);

	if (t.type == Todo::Link)
		throw exception("can't edit the body of a link");
	erase(todo, options.index[0]);
	if (t.done) throw exception("you can't edit an item that is done");
	if (options.verbose)
		cout << info << "Modify the text of the item you are editing." << normal << endl;
	t.text = readText("text> ", t.text);

	if (options.priority != Todo::None && options.priority != Todo::Default)
		t.priority = options.priority;

	if (options.priority != Todo::Default)
		t.priority = getPriority(symbolisePriority(t.priority));

	if (options.comment || t.comment != "") {
		if (options.verbose)
			cout << info << "Enter comment for this item." << normal << endl;
		t.comment = readText("comment> ", t.comment);
	}

	if (index.find(".") != string::npos) {
string parent = index.substr(0, index.rfind("."));

	Todo *p = find(todo, parent);

		p->child->insert(t);
		p->db->setDirty(true);
	} else {
		t.db = this;
		todo.insert(t);
		setDirty(true);
	}
}

void TodoDB::remove() {
vector<string> remove = options.index;
vector<string> notfound;
int erased = 0;

	for (vector<string>::iterator j = remove.begin(); j != remove.end(); j++) {
	Todo *t = find(todo, *j);

		if (t) {
			if (t->type == Todo::Link && options.verbose)
				cout << info << "todo: removing link to database '" << t->todofile << "'" << normal << endl;
			erased++;
			if (options.verbose > 1)
				cout << info << "todo: permanently removing item '" << (*j) << "'" << normal << endl;
			erase(todo, *j);
			t->db->setDirty(true);
		} else
			notfound.push_back(*j);
	}
	if (notfound.size())
		throw exception("couldn't erase records '" + join(",", notfound) + "'");
	if (options.verbose)
		cout << info << "todo: erased " << erased << " records" << normal << endl;
}

void TodoDB::filterChildren(Todo &todo, FilterChildren filter) {
	todo.filtered = filter == FILTERED ? true : false;
	if (!todo.filtered && todo.parent) 
		todo.incUnFilteredChildren();
	if (filter == NOTFILTERED)
		for (multiset<Todo>::iterator i = todo.child->begin(); i != todo.child->end(); i++)
			filterChildren((Todo&)*i, NOTFILTERED);
	else
	if (filter == CHILDRENNOTFILTERED)
		for (multiset<Todo>::iterator i = todo.child->begin(); i != todo.child->end(); i++) {
			if ((
				// ugh
				options.filter.done && (
					(options.filter.donedir == Options::Positive) ||
					(options.filter.donedir == Options::Equal && i->done) ||
					(options.filter.donedir == Options::Negative && !i->done)
					)
				) || 
				(!options.filter.done && !i->done)
				) {
				// doesn't change sort order
				const_cast<Todo*>(&*i)->filtered = false;
				const_cast<Todo*>(&*i)->incUnFilteredChildren();
			}
		}
}

void TodoDB::filterView(multiset<Todo> &todo) {
int n = 1;

	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); i++, n++) {
	Todo &todo = const_cast<Todo&>(*i);
	Options::Filter &filter = options.filter;
	bool skipchildren = false;

		if (filter.show == Options::Negative) 
			skipchildren = true;
		else
		if (filter.show == Options::Positive)
			filterChildren(todo, NOTFILTERED);
		else {
			// Filter on done
			if (filter.done) {
				// Don't show done notes
				if (filter.donedir == Options::Negative) {
					if (!todo.done) todo.filtered = false;
				} else
				// Only show done notes
				if (filter.donedir == Options::Equal) {
					if (todo.done) todo.filtered = false;
				} else
					todo.filtered = false;
			}

			// Filter on priority
			if (!todo.filtered && filter.priority != Todo::None) {
				todo.filtered = true;
				if (filter.prioritydir == Options::Negative) {
					if (todo.priority <= filter.priority) 
						todo.filtered = false;
				} else
				if (filter.prioritydir == Options::Positive) {
					if (todo.priority >= filter.priority) 
						todo.filtered = false;
				} else
					if (todo.priority == filter.priority) 
						todo.filtered = false;
			}

			if (filter.children) {
				// Don't show children
				if (filter.childrendir == Options::Negative) {
				bool state = todo.filtered;

					filterChildren(todo, FILTERED);
					todo.filtered = state;
					skipchildren = true;
				}
			}

			// Search filter - after all other filters
			if (!todo.filtered && filter.search.source().size() != 0 && filter.search.match(todo.text.c_str()) == -1)
				todo.filtered = true;
		}

		if (!todo.filtered)
			todo.incUnFilteredChildren();

		//if (!todo.filterchildren)
		if (!skipchildren) filterView(*todo.child);
	}
}

void TodoDB::filterView() {

	fixParents(todo);

	filterView(todo);

	// filter items based on explicit numeric
	for (map<string,Options::Dir>::const_iterator i = options.filter.item.begin(); i != options.filter.item.end(); ++i) {
	Todo *t = find(todo, (*i).first);

		if ((*i).second == Options::Negative)
			filterChildren(*t, FILTERED);
		else
		if ((*i).second == Options::Positive)
			filterChildren(*t, NOTFILTERED);
		else
			filterChildren(*t, CHILDRENNOTFILTERED);
	}
}

void TodoDB::view(multiset<Todo> const &todo, int ind) {

	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); i++) {
	Todo const &todo = (*i);

		if (!todo.filtered) {
			if (options.verbose)
				formatItem(cout, ind, todo, options.format["verbose-display"]);
			else
				formatItem(cout, ind, todo, options.format["display"]);
			if (options.comment && todo.comment != "") {
			int indent = 4 * (ind + 1);

				cout << comment << string(indent, ' ');
				if (options.summary) {
				const string s = todo.comment;

					if (s.find('\n') != string::npos) {
						if ((int)s.find('\n') < options.columns - 1 - indent)
							cout << s.substr(0, s.find('\n')) << info << "+" << normal;
						else
							cout << s.substr(0, options.columns - 1 - indent) << info << "+" << normal;
					} else
					if ((int)s.size() > options.columns - 3 - indent)
						cout << "(" << s.substr(0, options.columns - 3 - indent) << ")" << info << "+";
					else
						cout << "(" << s << ")";
				} else
					wraptext(cout, "(" + todo.comment + ")", indent, indent, options.columns);
				cout << normal << endl;
			}
		}

		//if (todo.filterchildren && !todo.unfilteredchildren) continue;
		if (todo.filtered && todo.unfilteredchildren)
			cout << string(4 * ind, ' ') << info << todo.index << "..." << normal << endl;
		view(*todo.child, ind + 1);
	}
}

void TodoDB::view() {
	if (titleText != "") {
		cout << title;
		wraptext(cout, titleText, 4, 0, options.columns - 8);
		cout << endl << normal;
	}
	filterView();
	if (options.verbose > 1)
		cout << "todo: displaying using format '" << options.format["verbose-display"] << "'" << endl;
	view(todo, 0);
}

void TodoDB::generate(ostream &out, multiset<Todo> const &todo, int ind) {

	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); i++) {
	Todo const &todo = (*i);

		if (!todo.filtered)
			if (options.verbose) {
				formatItem(out, ind, todo, options.format["verbose-generated"]);
			} else
				formatItem(out, ind, todo, options.format["generated"]);
		//if (todo.filterchildren) continue;
		if (todo.filtered && todo.unfilteredchildren)
			out << string(4 * ind, ' ') << info << todo.index << "..." << normal << endl;
		generate(out, *todo.child, ind + 1);
	}
}

void TodoDB::generate()
{
ofstream out("TODO");

	if (out.bad()) 
		throw exception("couldn't open TODO for generation");

	if (titleText != "") wraptext(out, titleText, 0, options.columns);
	if (options.verbose > 1)
		cout << "todo: generating using format '" << options.format["verbose-generated"] << "'" << endl;
	filterView();
	generate(out, todo, 0);
	if (options.verbose)
		cout << info << "todo: generated TODO from current database" << normal << endl;
}

void TodoDB::purge() {
time_t now = getCurrentDate();

unsigned purged = purge(todo, now - options.purgeage * 86400);
	if (options.verbose)
		cout << info << "todo: purged " << purged << " completed items older than " << options.purgeage << " days" << normal << endl;
}

unsigned TodoDB::purge(multiset<Todo> &todo, time_t age) {
unsigned purged = 0;

	for (multiset<Todo>::iterator i = todo.begin(); i != todo.end(); ++i) {
		if (i->done && i->doneTime < age) {
		multiset<Todo>::iterator last = i++;

			cout << i->doneTime << " < " << age << endl;
			last->db->setDirty(true);
			todo.erase(last);
			++purged;
		} else {
			if (i->child) {
				purged += purge(*i->child, age);
			}
		}
	}
	return purged;
}

void TodoDB::stats(multiset<Todo> const &todo, int ind) {
}

void TodoDB::stats() {
	filterView();
	if (options.verbose > 1)
		cout << "todo: displaying using format '" << options.format["verbose-display"] << "'" << endl;
	view(todo, 0);
}

int TodoDB::markDone(Todo &todo) {
int count = 1;

	todo.done = true;
	for (multiset<Todo>::iterator i = todo.child->begin(); i != todo.child->end(); i++)
		count += markDone(const_cast<Todo&>(*i));
	return count;
}

void TodoDB::done() {
vector<string> done = options.index, notfound;
int marked = 0;

	for (vector<string>::iterator j = done.begin(); j != done.end(); j++) {
	Todo *t = find(todo, *j);

		if (t) {
			marked += markDone(*t);
			if (options.verbose > 1)
				cout << "todo: marked '" << *j << "' as done" << endl;
			t->doneTime = getCurrentDate();
			if (options.verbose)
				cout << info << "Enter comment for this item." << normal << endl;
			t->comment = readText("comment> ", t->comment);
			t->db->setDirty(true);
		} else
			notfound.push_back(*j);
	}
	if (notfound.size())
		throw exception("couldn't mark records as done '" + join(",", notfound) + "'");
	if (options.verbose)
		cout << info << "todo: marked " << marked << " records as done" << normal << endl;
}

void TodoDB::notdone() {
vector<string> done = options.index, notfound;
int marked = 0;

	for (vector<string>::iterator j = done.begin(); j != done.end(); j++) {
	Todo *t = find(todo, *j);

		if (t) {
			t->done = false;
			marked++;
			if (options.verbose > 1)
				cout << "todo: marked '" << *j << "' as not done" << endl;

			t->db->setDirty(true);
		} else
			notfound.push_back(*j);
	}
	if (notfound.size())
		throw exception("couldn't mark records as not done '" + join(",", notfound) + "'");
	if (options.verbose)
		cout << info << "todo: marked " << marked << " records as not done" << normal << endl;
}

void TodoDB::setColour(string const &item, string const &colour) {
	if (streamColour.find(item.c_str()) == streamColour.end()) throw exception("unknown item '" + item + "' can't be coloured");

ostream &(*c)(ostream &);
ostream &(*a)(ostream &) = StreamColour::mono;
string clr = colour;

	if (clr[0] == '+') {
		a = ::bold;
		clr = colour.substr(1);
	}
	if (clr == "black") c = ::black;
	else
	if (clr == "red") c = ::red;
	else
	if (clr == "green") c = ::green;
	else
	if (clr == "yellow") c = ::yellow;
	else
	if (clr == "blue") c = ::blue;
	else
	if (clr == "magenta") c = ::magenta;
	else
	if (clr == "cyan") c = ::cyan;
	else
	if (clr == "white") c = ::white;
	else
	if (clr == "default") c = StreamColour::mono;
	else
		throw exception("unknown colour '" + clr + "'");
	streamColour[item].attribute = a;
	streamColour[item].colour = c;
}

void TodoDB::reparent() {
string aps = options.index[0];
Todo *a = find(aps);
multiset<Todo> *ap = 0;

int delim = aps.rfind('.');
	if (delim != -1) {
		aps = aps.substr(0, delim);
		ap = &(*find(aps)->child);
	} else
		ap = &todo;

Todo tmp = *a;
	for (multiset<Todo>::iterator i = ap->begin(); i != ap->end(); ++i)
		if (&(*i) == a) {
			i->db->setDirty(true);
			ap->erase(i);
			break;
		}

	if (options.index.size() > 1) {
		Todo *t = find(options.index[1]);
		tmp.parent = t;
		tmp.db = t->db;
		t->child->insert(tmp);
		t->db->setDirty(true);
	} else {
		tmp.db = this;
		todo.insert(tmp);
		setDirty(true);
	}
	

//	tmp.child = NULL;
}

ostream &TodoDB::StreamColour::veryhigh(ostream &os) { return os << normal << streamColour["veryhigh"].attribute << streamColour["veryhigh"].colour; }
ostream &TodoDB::StreamColour::high(ostream &os) { return os << normal << streamColour["high"].attribute << streamColour["high"].colour; }
ostream &TodoDB::StreamColour::medium(ostream &os) { return os << normal << streamColour["medium"].attribute << streamColour["medium"].colour; }
ostream &TodoDB::StreamColour::low(ostream &os) { return os << normal << streamColour["low"].attribute << streamColour["low"].colour; }
ostream &TodoDB::StreamColour::verylow(ostream &os) { return os << normal << streamColour["verylow"].attribute << streamColour["verylow"].colour; }
ostream &TodoDB::StreamColour::info(ostream &os) { return os << normal << streamColour["info"].attribute << streamColour["info"].colour; }
ostream &TodoDB::StreamColour::title(ostream &os) { return os << normal << streamColour["title"].attribute << streamColour["title"].colour; }
ostream &TodoDB::StreamColour::comment(ostream &os) { return os << normal << streamColour["comment"].attribute << streamColour["comment"].colour; }
ostream &TodoDB::StreamColour::mono(ostream &os) { return os; }
ostream &TodoDB::StreamColour::normal(ostream &os) { return os << ::normal; }

/*
	Template string for formatting output. For example, 
		%i%[info]%f%2n.%[priority]%T\n
	would generate the default display.

	- %<n>i: indent to current depth; <n> is the number of spaces per 
	  indent level and defaults to 4
	- %T is the item text, which wraps and indents to the depth the item
	  started at
	- %t is unwrapped text
	- %p is the priority
	- %c is the creation date (formatted according to --format-date)
	- %d is the completion (done) date
	- %D is the time it took to complete the item
	- %<n>n is the index number of the item; <n> is the amount of characters the
	  number should take up - padded with spaces
	- %f is the state flag (+ means children, - means done, * means 
	  children and done)
	- %[<colour>] to specify a colour (an additional colour is 'priority' 
	  which defaults to the current items priority colour). 
	  eg. %[priority]
	- %s is a summary (one line) of the text body

*/
void TodoDB::formatItem(ostream &out, int depth, Todo const &item, string const &format) {
int indent = 0;
int defaultindent = 4;
Options::Dir dir = Options::Equal;

	for (unsigned i = 0; i < format.size(); ++i) {
	int multiplier = -1;

		switch (format[i]) {
			case '%' :
				++i;
				if (isdigit(format[i]) || format[i] == '-' || format[i] == '+') {
					if (format[i] == '-') {
						dir = Options::Negative;
						++i;
					} else
					if (format[i] == '+') {
						dir = Options::Positive;
						++i;
					} else
						dir = Options::Equal;
					multiplier = destringify<int>(format.c_str() + i);
					while (format[i] && (isdigit(format[i]) || 
						format[i] == '-' || format[i] == '+')) ++i;
				}
				switch (format[i]) {
					case '%' : out << '%'; break;
					case '>' :
						if (multiplier == -1)
							throw exception("'>' formatting flag requires numeric prefix");
						defaultindent = multiplier;
					break;
					// indent
					case 'i' : {
					int i = 0;

						i = depth;
						if (dir != 0)
							i += dir * multiplier;
						i *= defaultindent;
						for (; i > indent; indent++)
							out << ' ';
					}
					break;
					case 'T' : {
					int i = 0;

						i = depth;
						if (dir != 0)
							i += dir * multiplier;
						i *= defaultindent;
						if (options.summary) {
						const string s = item.text;

							if (s.find('\n') != string::npos) {
								if ((int)s.find('\n') < options.columns - 1 - indent)
									out << s.substr(0, s.find('\n')) << info << "+" << normal;
								else
									out << s.substr(0, options.columns - 1 - indent) << info << "+" << normal;
							} else
							if ((int)s.size() > options.columns - 1 - indent)
								out << s.substr(0, options.columns - 1 - indent) << info << "+" << normal;
							else
								out << s;
						} else
							wraptext(out, item.text, i, indent, options.columns);
					}
					break;
					case 't' :
						if (options.summary) {
/*							if ((int)item.text.size() > options.columns - 1 - indent)
								out << item.text.substr(0, options.columns - 1 - indent) << info << "+" << normal << endl;
							else
								out << item.text << endl;
							indent = 0;*/
						const string s = item.text;

							if (s.find('\n') != string::npos) {
								if ((int)s.find('\n') < options.columns - 1 - indent)
									out << s.substr(0, s.find('\n')) << info << "+" << normal << endl;
								else
									out << s.substr(0, options.columns - 1 - indent) << info << "+" << normal << endl;
							} else
							if ((int)s.size() > options.columns - 1 - indent)
								out << s.substr(0, options.columns - 1 - indent) << info << "+" << normal << endl;
							else
								out << s << endl;
							indent = 0;
						} else {
							out << item.text;
							indent += item.text.size();
						}
					break;
					case 's' :
						if ((int)item.text.size() > options.columns - 1 - indent)
							out << item.text.substr(0, options.columns - 1 - indent) << info << "+" << normal << endl;
						else
							out << item.text << endl;
						indent = 0;
					break;
					case 'p' : {
					string s = symbolisePriority(item.priority);
						out << s;
						indent += s.size();
					}
					break;
					case 'c' : {
					string s = dateToHuman(item.added);
						out << s;
						indent += s.size();
					}
					break;
					case 'd' : {
					string s;
					
						if (item.done)
							s = "completed on " + dateToHuman(item.doneTime);
						else
							s = "incomplete";
						out << s;
						indent += s.size();
					}
					break;
					case 'D' :
						if (item.done) {
						string s = elapsedToHuman(item.added, item.doneTime) + " elapsed";
							out << s;
							indent += s.size();
						} else {
							out << "incomplete";
							indent += 12;
						}
					break;
					case 'n' : {
					string s = stringify(item.index);
						if (multiplier != -1 && (int)s.size() < multiplier) {
							out << string(multiplier - s.size(), ' ');
							indent += multiplier - s.size();
						}
						out << s;
						indent += s.size();
					}
					break;
					case 'F' : {
					string s;
					bool filteredchildren = false;

						for (multiset<Todo>::const_iterator i = item.child->begin(); i != item.child->end(); i++)
							if ((*i).filtered) {
								filteredchildren = true;
								break;
							}

						if (item.done) {
							if (filteredchildren)
								s = "done, children";
							else
								s = "done";
						} else
						if (filteredchildren)
							s = "children";
						else
							s = "open";
						out << s;
						indent += s.size();
					}
					break;
					case 'f' : {
					bool filteredchildren = false;

						for (multiset<Todo>::const_iterator i = item.child->begin(); i != item.child->end(); i++)
							if ((*i).filtered) {
								filteredchildren = true;
								break;
							}

						if (item.done) {
							if (filteredchildren)
								out << '*';
							else
								out << "-";
						} else
						if (filteredchildren)
							out << "+";
						else
							out << " ";
						indent++;
					} break;
					case '[' : {
						i++;
						if (format.find(']', i) == string::npos)
							throw exception("no matching ']' found for format string");
					string colour = format.substr(i, format.find(']', i) - i);
						i = format.find(']', i);
						if (colour == "priority")
							out << priority[4 - item.priority];
						else
						if (colour == "info")
							out << info;
						else
						if (colour == "normal")
							out << normal;
						else
						if (colour == "comment")
							out << comment;
						else
							out << priority[4 - desymbolisePriority(colour)];
					}
					break;
					default :
						throw exception(stringify("unhandled formatting flag '") + format[i] + "'");
					break;
				}
			break;
			// Handle some C-style escape characters
			case '\\' :
				++i;
				switch (format[i]) {
					case 'n' : out << '\n'; indent = 0; break;
					case 't' :
						out << ' ';
						for (indent++; indent % 8 != 0; indent++) out << ' ';
					break;
				}
			break;
			default :
				out << format[i];
				if (format[i] == '\n') indent = 0; else indent++;
			break;
		}
	}
	out << normal;
}

void TodoDB::triggerEvent(string const &event) {
	if (options.event.find(event) == options.event.end()) return;
vector<string> &v = options.event[event];
char const *argv[v.size() + 1];

	if (options.verbose > 1)
		cout << "todo: event '" << event << "' triggered ('" << str::join(" ", options.event[event]) << "')" << endl;
	argv[0] = "todo";
	for (unsigned i = 0; i < v.size(); i++) {
		string s = expandEnvars(v[i]);
		argv[i + 1] = strdup(s.c_str());
	}
	parseArgs(*this, v.size() + 1, argv);
}

string TodoDB::fixPath(string path)
{
	string newPath;
	string::iterator iter;
	
	for (iter = path.begin(); iter != path.end(); iter++)
	{
		if (*iter == '.')
		{
			if (*(iter + 1) == '.' && *(iter + 2) == '/')
			{
				if (!newPath.empty())
				{
					newPath.erase(newPath.rfind("/", newPath.size() - 2) + 1);
				}

				iter += 2;

				continue;
			}
			else if (*(iter + 1) == '/')
			{
				iter++;

				continue;
			}
			else if (iter + 1 == path.end())
				continue;
		}
		else if (*iter == '/')
		{
			while (*(iter + 1) == '/')
				iter++;
		}

		newPath.push_back(*iter);
	}

	return newPath;
}

string TodoDB::fixRelativePath(string base, string path)
{
	char cwd[BUFSIZ];
	string relBase, relDestPath;
	string prefix;
	string result;
	size_t s;

	if (path[0] == '/')
		return path;

	/* Chop off any trailing '/''s */
	if ((s = base.find_last_not_of('/')) != string::npos)
		base.erase(s + 1);

	if ((s = path.find_last_not_of('/')) != string::npos)
		path.erase(s + 1);

	/* Get the working directory. */
	getcwd(cwd, BUFSIZ);

	/* Build the paths appropriately. */
	if (base[0] != '/')
	{
		relBase = cwd;
		relBase += "/" + base;
	}
	else
		relBase = base;

	relDestPath = cwd;
	relDestPath += "/" + path;

	/* Fix up the paths to process stuff like '..' */
	relBase = fixPath(relBase);
	relDestPath = fixPath(relDestPath);

	/* See how much of the prefix we can chop off. */
	s = 0;

	do
	{
		size_t s1 = relBase.find("/", s);

		if (s1 == 0 && relBase[0] != '/')
			break;

		if (relBase.substr(0, s1) == relDestPath.substr(0, s1))
			s = s1 + 1;
		else
			prefix = relBase.substr(0, s);

	} while (prefix.empty());

	relBase.erase(0, prefix.size());
	relDestPath.erase(0, prefix.size());
	
	if (relBase.empty())
		result = relDestPath;
	else
	{
		/* Look at the number of '/''s in relBase */
		size_t count = 0;

		for (s = 0; (s = relBase.find("/", s + 1)) != string::npos; count++)
			;

		for (size_t i = 0; i <= count; i++)
			result += "../";

		result += relDestPath;
	}

	return result;
}
