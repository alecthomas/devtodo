#ifndef TODODB_H__
#define TODODB_H__

#include <stdexcept>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Todo.h"
#include "XML.h"
#include "Terminal.h"

using namespace std;

class Options;

/*
	TodoDB is the workhorse of the program. It does all of the loading
	and manipulating of the database.

	01/02/01	Initial creation
*/

class TodoDB {
	public :
		class exception : public runtime_error { public : exception(string const &what) : runtime_error(what.c_str()) {} };
		class quit : public runtime_error { public : quit() : runtime_error("quit") {} };

		enum Mode {
			Add,
			Link,
			Remove,
			View,
			Edit,
			Generate,
			Done,
			NotDone,
			Title,
			Reparent,
			Stats,
			Purge,
		};

		TodoDB();
		TodoDB(string const &file);
		~TodoDB();

		void load(string const &file = ".todo");
		void save(string const &file = ".todo");

		void operator () (Mode mode);

		Todo *find(string const &index) { return find(todo, index); }
		void erase(string &index) { return erase(todo, index); }

		vector<string> getIndexList(string const &indexList);

		void setColour(string const &item, string const &colour);

		void setDirty(bool d) { dirty = d; }
		bool isDirty() const { return dirty; }

		void triggerEvent(string const &event);

		multiset<Todo> todo;
		string titleText;
		string filename, basepath;

	private :
		enum FilterChildren { FILTERED, NOTFILTERED, CHILDRENNOTFILTERED };

		void add();
		void link();
		void remove();
		void view();
		void edit();
		void generate();
		void done();
		void notdone();
		void edittitle();
		void reparent();
		void stats();
		void purge();

		// different loaders

		// Private, so object can't be default copied.
		TodoDB(TodoDB const &copy) {}
		TodoDB &operator = (TodoDB const &copy) { return *this; }

		Todo *find(multiset<Todo> const &todo, string const &index);
		multiset<Todo> &findContainer(multiset<Todo> &todo, string const &index);
		void erase(multiset<Todo> &todo, string const &index);
		void parse(vector<XML*>::const_iterator begin,
		    vector<XML*>::const_iterator end, multiset<Todo> &out);
		void save(multiset<Todo> const &todo, ostream &of, int ind);
		void filterChildren(Todo &todo, FilterChildren filter = FILTERED);
		void filterView();
		void filterView(multiset<Todo> &todo);
		void stats(multiset<Todo> const &todo, int ind);
		void view(multiset<Todo> const &todo, int ind);
		void generate(ostream &out, multiset<Todo> const &todo, int ind);
		unsigned purge(multiset<Todo> &todo, time_t age);

		void fixParents(multiset<Todo> &todo, Todo *parent = 0);

		void initColour();
		void initColourPost();
		Todo::Priority getPriority(string current = "");
		int markDone(Todo &todo);

		void formatItem(ostream &out, int depth, Todo const &item, string const &format);

		string fixPath(string path);
		string fixRelativePath(string base, string path);

		struct stat _stat;
		bool statSuccessful;
		bool dirty;

		struct StreamColour {
			StreamColour() {}
			StreamColour(ostream &(*c)(ostream &), ostream &(*a)(ostream &)) : colour(c), attribute(a) {}
			ostream &(*colour)(ostream &);
			ostream &(*attribute)(ostream &);

			static ostream &veryhigh(ostream &os);
			static ostream &high(ostream &os);
			static ostream &medium(ostream &os);
			static ostream &low(ostream &os);
			static ostream &verylow(ostream &os);
			static ostream &info(ostream &os);
			static ostream &title(ostream &os);
			static ostream &comment(ostream &os);
			static ostream &mono(ostream &os);
			static ostream &normal(ostream &os);
		};
		static map<string, StreamColour> streamColour;

		// Colour strings
		ostream &(*priority[5])(ostream &);
		ostream &(*info)(ostream &);
		ostream &(*title)(ostream &);
		ostream &(*normal)(ostream &);
		ostream &(*comment)(ostream &);
};

#endif
