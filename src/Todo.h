#ifndef TODO_H__
#define TODO_H__

#include <stdexcept>
#include <string>
#include <set>
#include <sys/types.h>

using namespace std;

/*
	Todo is the basic data structure for each entry in the .todo database.

	01/02/01	Initial creation
*/

class Todo {
	public :
		friend class TodoDB;

		class exception : public runtime_error { public : exception(char const *what) : runtime_error(what) {} };

		enum Type {
			Note = 0,
			Link
		};

		enum Priority {
			Default = -2,
			None = -1,
			VeryLow = 0,
			Low,
			Medium,
			High,
			VeryHigh,
		};

		Todo();
		Todo(const Todo &other);
		~Todo();

		int operator < (Todo const &other) const;
		int operator == (Todo const &other) const;

		Todo &operator = (const Todo &other);

		void incUnFilteredChildren();
		void decUnFilteredChildren();

		// this is saved
		Priority priority;
		string text, todofile, comment;
		bool done;
		time_t added, doneTime;

		// not saved to xml
		int index, unfilteredchildren;
		bool filtered;
		Type type;

		multiset<Todo> *child, *in;
		Todo *parent;
		
		TodoDB *db;
};

#endif
