#include <algorithm>
#include "support.h"
#include "Todo.h"

Todo::Todo() : priority(None), done(false), added(0), doneTime(0), 
	unfilteredchildren(0), filtered(true), type(Note), child(0), in(0), 
	parent(0), db(0) {
	
	child = new multiset<Todo>;
}

Todo::Todo(const Todo &other) {
	*this = other;
}

Todo::~Todo() {
	if (type == Note && child != NULL)
		delete child;
}

void Todo::incUnFilteredChildren() {
	unfilteredchildren++;
	if (parent) parent->incUnFilteredChildren();
}

void Todo::decUnFilteredChildren() {
	if (unfilteredchildren) unfilteredchildren--;
	if (parent) parent->decUnFilteredChildren();
}

int Todo::operator == (Todo const &other) const {
	return priority == other.priority &&
		text == other.text &&
		comment == other.comment &&
		done == other.done &&
		added == other.added &&
		doneTime == other.doneTime;
}

// Enforces the sort order of Todo database items
int Todo::operator < (Todo const &other) const {
	for (vector<Options::Sort>::iterator i = options.sort.begin(); i != options.sort.end(); ++i) {
		switch ((*i).key) {
			case Options::Sort::Created :
				if (added != other.added) {
					if (added < other.added)
						return (*i).dir;
					else
						return !(*i).dir;
				}
			break;
			case Options::Sort::Completed :
				if (doneTime != other.doneTime) 
					if (doneTime < other.doneTime)
						return (*i).dir;
					else
						return !(*i).dir;
			break;
			case Options::Sort::Text :
				if (text != other.text) 
					if (text < other.text)
						return (*i).dir;
					else
						return !(*i).dir;
			break;
			case Options::Sort::Priority :
				if (priority != other.priority) 
					if (priority < other.priority)
						return (*i).dir;
					else
						return !(*i).dir;
			break;
			case Options::Sort::Duration : {
			time_t d = doneTime - added, od = other.doneTime - other.added;

				if (!done) d = 0;
				if (!other.done) od = 0;

				if (d != od) 
					if (d < od)
						return (*i).dir;
					else
						return !(*i).dir;
			} break;
			case Options::Sort::Done :
				if (done != other.done)
					if (done && !other.done)
						return (*i).dir;
					else
						return !(*i).dir;
			break;
			case Options::Sort::None :
				return 0;
			break;
		}
	}
	return 0;
}

Todo &Todo::operator = (const Todo &other)
{
	if (this != &other)
	{
		priority = other.priority;
		text = other.text;
		todofile = other.todofile;
		comment = other.comment;
		done = other.done;
		added = other.added;
		doneTime = other.doneTime;

		index = other.index;
		unfilteredchildren = other.unfilteredchildren;
		filtered = other.filtered;
		type = other.type;

		parent = other.parent;
		db = other.db;

		in = other.in;

		if (type == Link)
			child = other.child;
		else {
			child = new multiset<Todo>;
			*child = *other.child;
		}
	}

	return *this;
}
