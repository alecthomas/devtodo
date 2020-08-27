#include <sys/stat.h>
#include <unistd.h>
#include "Loaders.h"
#include "Strings.h"
#include "support.h"
#include "config.h"

using namespace str;

Loader getLoaders() {
Loader out;

	out["xml"] = xmlLoad;
	out["binary"] = binaryLoad;
	return out;
}

Saver getSavers() {
Saver out;

	out["xml"] = xmlSave;
	out["binary"] = binarySave;
	return out;
}

void checkVersion(string const &version) {
vector<string> vtdver = str::split(".", version);
vector<string> vmyver = str::split(".", VERSION);
int tdver[3], myver[3];

	if (options.verbose > 1)
		cout << "todo: loading database version '" << version << "' (binary version is '" << VERSION << "')" << endl;

	if (vtdver.size() != 3) throw load_error("invalid database version number '" + version + "'");

	tdver[0] = destringify<int>(vtdver[0]);
	tdver[1] = destringify<int>(vtdver[1]);
	tdver[2] = destringify<int>(vtdver[2]);

	myver[0] = destringify<int>(vmyver[0]);
	myver[1] = destringify<int>(vmyver[1]);
	myver[2] = destringify<int>(vmyver[2]);

	if (tdver[0] != myver[0] || tdver[1] != myver[1]) {
		cerr << "todo: error, version of database (" << version << ") is different than binary (" << VERSION << ")" << endl;
		exit(1);
	}
	// minor version differences shouldn't be fatal
	if (tdver[2] > myver[2]) cerr << "warning: version of database (" << version << ") is more recent than binary (" << VERSION << ")" << endl;
}

void xmlParse(TodoDB &outdb, vector<XML*>::const_iterator begin, 
	vector<XML*>::const_iterator end, multiset<Todo> &out) {
	for (vector<XML*>::const_iterator i = begin; i != end; i++) {
	XML &x = *(*i);

		switch (x.type()) {
			case XML::Element : {
				if (x.name() == "title") {
					outdb.titleText = trim((*x.child().begin())->body());
					break;
				} else
				if (x.name() == "note") {
				Todo todo;
				// const_cast so I can use attrib[...]
				map<string, string> &attrib = *const_cast<map<string, string>* >(&x.attrib());

					if (attrib.find("priority") == attrib.end() || attrib.find("time") == attrib.end())
						throw load_error("require both 'priority' and 'time' attributes for 'note' element");

					if (attrib.find("done") != attrib.end()) {
							todo.done = true;
							todo.doneTime = destringify<time_t>(attrib["done"]);
						} else
							todo.done = false;

					todo.priority = desymbolisePriority(attrib["priority"]);
					todo.text = trim((*x.child().begin())->body());
					todo.added = destringify<time_t>(attrib["time"]);
					todo.db = &outdb;

					for (vector<XML*>::const_iterator j = x.child().begin(); j != x.child().end(); ++j)
						if ((*j)->name() == "comment") {
							todo.comment = trim((*(*j)->child().begin())->body());
							break;
						}

					xmlParse(outdb, x.child().begin(), x.child().end(), *todo.child);
					out.insert(todo);
				} else if (x.name() == "link") {
				TodoDB *newDb = new TodoDB();
				Todo todo;
				map<string, string> &attrib = *const_cast<map<string, string>* >(&x.attrib());

					if (attrib.find("priority") == attrib.end() || attrib.find("filename") == attrib.end() || attrib.find("time") == attrib.end())
						throw load_error("require 'priority', 'time', and 'filename' attributes for 'link' element");

					try {
						if (outdb.basepath == "" || attrib["filename"][0] == '/')
							newDb->load(attrib["filename"]);
						else
							newDb->load(outdb.basepath + "/" + attrib["filename"]);
						if (newDb->titleText == "") {
							if (newDb->basepath == "")
								todo.text = attrib["filename"];
							else
								todo.text = newDb->basepath.substr(newDb->basepath.rfind("/") + 1);
						}
						else
							todo.text = newDb->titleText;

						delete todo.child;

						
						todo.type = Todo::Link;
						todo.child = &newDb->todo;
						todo.priority = desymbolisePriority(attrib["priority"]);
						todo.todofile = attrib["filename"];
						todo.db = newDb;

						out.insert(todo);
					} catch (...) {
						delete newDb;
					}
				} else
				if (x.name() != "comment")
					throw load_error("expected 'note' element, got '" + x.name() + "'");
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

bool xmlLoad(TodoDB &todo, string const &file) {
ifstream in(file.c_str());

	if (in.bad()||in.fail()||in.eof()) return false;
struct stat s;
	stat(file.c_str(), &s);

char *str;
	str = new char[s.st_size + 1];

	in.read(str, s.st_size);
	str[s.st_size] = 0;

XML x(str);

	if (x.name() != "todo")
		throw load_error("primary element not 'todo'");
map<string, string> const &attrib = x.attrib();

	// do version checking
	if (attrib.find("version") != attrib.end())
		checkVersion(const_cast<map<string, string>&>(attrib)["version"]);

	xmlParse(todo, x.child().begin(), x.child().end(), todo.todo);
	return true;
}

void xmlSave(TodoDB const &db, multiset<Todo> const &todo, ostream &of, int ind) {
	for (multiset<Todo>::const_iterator i = todo.begin(); i != todo.end(); i++) {
		if (i->type == Todo::Link) {
			of  << string(ind * 4, ' ') << "<link"
			    << " filename=\"" << i->todofile << "\""
				<< " priority=\"" << symbolisePriority(i->priority) << "\""
				<< " time=\"" << i->added << "\""
				<< "/>" << endl;

			if (i->db)
				i->db->save(i->todofile);

			/* Restore the TODODB environment variable. */
string envar = "TODODB=" + db.filename;

			putenv(strdup(const_cast<char*>(envar.c_str())));

		} else {
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
				of << string((ind + 1) * 4, ' ');
				of << "<comment>" << endl;
				of << string((ind + 2) * 4, ' ');
				of << htmlify((*i).comment) << endl;
				of << string((ind + 1) * 4, ' ');
				of << "</comment>" << endl;
			}
			xmlSave(db, *i->child, of, ind + 1);
			of << string(ind * 4, ' ');
			of << "</note>" << endl;
		}
	}
}

bool xmlSave(TodoDB const &todo, string const &file) {
	if (todo.isDirty())
	{
	ofstream of(file.c_str());

        if(of.fail( ) || of.bad( )) {
            cout << "ERROR: Failed to create/open file: " << file << "\n";
            exit(1);
        }

		if (options.verbose > 1)
			cout << "todo: saving to database '" << file << "'" << endl;
		of << "<?xml version=\"1.0\"?>" << endl;
		of << "<todo version=\"" << VERSION << "\">" << endl;
		if (todo.titleText != "")
			of	<< "    <title>" << endl 
				<< "        " << todo.titleText << endl
				<< "    </title>" << endl;

		/* Set the TODODB environment variable. */
	string envar = "TODODB=" + todo.filename;

		putenv(strdup(const_cast<char*>(envar.c_str())));

		xmlSave(todo, todo.todo, of, 1);
		of << "</todo>" << endl;
		of.close();
		return true;
	}
	else
	{
	ofstream out("/dev/null");

		xmlSave(todo, todo.todo, out, 1);
		return false;
	}
}

void binaryLoad(ifstream &in, int &n) {
	in.read((char *)&n, sizeof(n));
}

void binaryLoad(ifstream &in, string &str) {
int size;

	binaryLoad(in, size);
char buffer[size + 1];
	in.read(buffer, size);
	buffer[size] = 0;
	str = buffer;
}

void binaryLoad(TodoDB &db, ifstream &in, multiset<Todo> &out) {
int n;

	binaryLoad(in, n);
	for (int i = 0; i < n; i++) {
	Todo t;
	int tmp;

		binaryLoad(in, tmp); t.priority = (Todo::Priority)tmp;
		binaryLoad(in, tmp); t.added = tmp;
		binaryLoad(in, tmp); t.doneTime = tmp;
		if (t.doneTime) t.done = true;
		binaryLoad(in, t.text);
		binaryLoad(db, in, *t.child);

		t.db = &db;
		out.insert(t);
	}

	// number the items
	n = 1;
	for (multiset<Todo>::iterator i = out.begin(); i != out.end(); ++i, ++n) {
	Todo &t = const_cast<Todo&>(*i);
		
		t.index = n;
	}
}

bool binaryLoad(TodoDB &out, string const &file) {
ifstream in(file.c_str());

	if (in.bad()||in.fail()||in.eof()) return false;
char buffer[5];

	in.read(buffer, 4);
	buffer[4] = 0;
	if (string(buffer) != "TODO") return false;
string version;
	binaryLoad(in, version);
	checkVersion(version);
	binaryLoad(in, out.titleText);
	binaryLoad(out, in, out.todo);
	return true;
}

void binarySave(ofstream &of, int n) {
	of.write((char*)&n, sizeof(n));
}

void binarySave(ofstream &of, string const &str) {
	binarySave(of, str.size());
	of.write(str.c_str(), str.size());
}

void binarySave(ofstream &of, multiset<Todo> const &db) {
	binarySave(of, db.size());
	for (multiset<Todo>::const_iterator i = db.begin(); i != db.end(); ++i) {
		binarySave(of, (*i).priority);
		binarySave(of, (*i).added);
		if ((*i).done)
			binarySave(of, (*i).doneTime);
		else
			binarySave(of, 0);
		binarySave(of, (*i).text);
		binarySave(of, *i->child);
	}
}

bool binarySave(TodoDB const &in, string const &file) {
ofstream of(file.c_str());

	if (of.bad()) return false;

string envar = "TODODB=" + in.filename;

	putenv(strdup(const_cast<char*>(envar.c_str())));

	of.write("TODO", 4);
	binarySave(of, VERSION);
	binarySave(of, in.titleText);
	binarySave(of, in.todo);
	return true;
}

