#ifndef CRASH_XML
#define CRASH_XML

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include "Strings.h"
#include "Lexer.h"

using namespace std;


class XML {
	public :
		enum Type {
			Element = 256,
			Body,
			Data
		};

#ifdef CRASH_SIGNAL
		static Signal2<string const &, map<string, string> const &> onElementBegin;
		static Signal1<string const &> onElementEnd;
		static Signal2<XML const &, string const &> onBody;
		static Signal2<XML const &, string const &> onData;
#endif

		class exception : public runtime_error {
			public :
				exception(string const &what, int line) : 
					runtime_error(what.c_str()), _line(line) {}
				int line() const { return _line; }
			private :
				int _line;
		};

		XML();
		XML(char const *input);
		virtual ~XML();

		void parse(char const *input);

		Type type() const { return _type; }

		string const &name() const { return _data; }
		string const &body() const { return _data; }
		string const &data() const { return _data; }

		vector<XML*> const &child() const { return _child; }
		map<string, string> const &attrib() const { return _attrib; }
	protected :
		XML(Type type,  XML *parent, Lexer::iterator &token);

		void init();

		void skip(Lexer::iterator &token);
		void next(Lexer::iterator &token);

		void parseElement(Lexer::iterator &token);
		void parseBody(Lexer::iterator &token);
		void parseData(Lexer::iterator &token);

		// Lexer constants
		enum { XmlDecl = 256, XmlCommentBegin, XmlBegin, XmlEnd, XmlDataBegin, XmlContent };
		enum { ElementWS = 256, ElementValue, ElementKey, ElementAssignment, ElementTerminator};
		enum { CommentEnd = 256,  CommentBody };
		enum { DataEnd = 256, DataBody };
		enum { ProcessBegin = 256, ProcessBody, ProcessEnd };

		static bool initialised;
		static Lexer xmlScan, tagScan, commentScan, dataScan, processScan;

		XML *_parent;
		Type _type;
		string _data;

		map<string, string> _attrib;
		vector<XML*> _child;
};

#endif
