#ifndef LOADERS_H__
#define LOADERS_H__

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include "TodoDB.h"

using namespace std;

/*
	21/04/01	Initial creation
*/

/*
	These functions load and save the database in a variety of user-selectable
	formats. The default is "xml", but "binary" is *much* faster.
*/

struct load_error : public runtime_error { load_error(string const &what) : runtime_error(what.c_str()) {} };
struct save_error : public runtime_error { save_error(string const &what) : runtime_error(what.c_str()) {} };

typedef map<string, bool (*)(TodoDB &, string const &)> Loader;
typedef map<string, bool (*)(TodoDB const &, string const &)> Saver;

Loader getLoaders();
Saver getSavers();

bool xmlLoad(TodoDB &out, string const &file);
bool xmlSave(TodoDB const &in, string const &file);

bool binaryLoad(TodoDB &out, string const &file);
bool binarySave(TodoDB const &in, string const &file);
#endif
