#include "XML.h"

bool XML::initialised = false;
Lexer XML::xmlScan, XML::tagScan, XML::commentScan, XML::dataScan, XML::processScan;

#ifdef CRASH_SIGNAL
Signal2<string const &, map<string, string> const &> XML::onElementBegin;
Signal1<string const &> XML::onElementEnd;
Signal2<XML const &, string const &> XML::onBody;
Signal2<XML const &, string const &> XML::onData;
#endif

XML::XML(Type type, XML *parent, Lexer::iterator &token) : _parent(parent), _type(type) {
	switch (type) {
		case Element : parseElement(token); break;
		case Body : parseBody(token); break;
		case Data : parseData(token); break;
	}
}

XML::XML() : _parent(0), _type(Element) {
	init();
}

XML::XML(char const *str) : _parent(0), _type(Element) {
	init();
	parse(str);
}

XML::~XML() {
	for (vector<XML*>::iterator i = _child.begin(); i != _child.end(); ++i)
		delete *i;
}

void XML::parse(char const *str) {
	try {
	Lexer::iterator i = xmlScan.begin(str);

		parseElement(i);
	} catch (Lexer::exception &e) {
		throw exception(e.what(), e.line());
	}
}

void XML::init() {
	// Only initialise scanners once
	if (!initialised) {
		xmlScan.addPattern(XmlCommentBegin, "<!--");
		xmlScan.addPattern(XmlBegin, "<[a-zA-Z0-9_-]+"
			"([[:space:]]+[a-zA-Z_0-9-]+=(([/a-zA-Z_0-9,.]+)|(\"[^\"]*\")|('[^']*')))"
			"*[[:space:]]*(/?)>");
		xmlScan.addPattern(XmlEnd, "</[a-zA-Z0-9_-]+>");
		xmlScan.addPattern(XmlDataBegin, "<!DATA[[:space:]]*\\[\\[");
		xmlScan.addPattern(XmlContent, "([^<]|[\n\r])+");                                     

		commentScan.addPattern(CommentEnd, "-->[[:space:]]*");
		commentScan.addPattern(CommentBody, "[\n\r]|.");

		tagScan.addPattern(ElementWS, "[[:space:]]+", true);
		tagScan.addPattern(ElementValue, "('(\\.|[^'])*')|(\"(\\.|[^\"])*\")");
		tagScan.addPattern(ElementKey, "([a-zA-Z_][a-zA-Z0-9-]*)");
		tagScan.addPattern(ElementAssignment, "=");                                          
		tagScan.addPattern(ElementTerminator, "/");

		dataScan.addPattern(DataEnd, "]]>");
		dataScan.addPattern(DataBody, "[\n\r]|.");

		processScan.addPattern(ProcessBegin, "<\\?xml");
		processScan.addPattern(ProcessBody, "\\?>|[^?][^>]");
		processScan.addPattern(ProcessEnd, "\\?>");

		initialised = true;
	}
}

// Skip comments
void XML::skip(Lexer::iterator &token) {
	while (token.type() == XmlCommentBegin)
	{
	int skip = 0;

		try {
			for (Lexer::iterator i = commentScan.begin(token.source()); i != commentScan.end(); ++i) {
				skip += i.size();
				if (i.type() == CommentEnd) 
					break;
			}
		} catch (Lexer::exception &e) {
			throw exception(e.what(),  token.line() + e.line() - 1);
		}
		token.skip(skip);
		++token;
	}
}

// Get next token, skipping any comments
void XML::next(Lexer::iterator &token) {
	++token;
	skip(token);
}

void XML::parseElement(Lexer::iterator &token) {
	skip(token);

	if (token.type() != XmlBegin)
		throw exception("expected element, got '" + token.value() + "'", token.line());

char str[token.size()];
	strncpy(str, token.value().c_str() + 1, token.size() - 2);
	str[token.size() - 2] = 0;
	
	try {
	Lexer::iterator i = tagScan.begin(str);

		if (i.type() != ElementKey)
			throw exception("invalid key", token.line());
		_data = i.value();

		// Extract attributes
		for (++i; i != tagScan.end(); ++i) {

			if (i.type() == ElementTerminator) {
				next(token);
				return;
			}
			if (i.type() != ElementKey)
				throw exception("expected key for attribute, got '" + i.value() + "'", token.line());
		string k = i.value();
			++i;
			if (i.type() != ElementAssignment)
				throw exception("expected assignment operator after attribute key, got '" + i.value() + "'", token.line());
			++i;
			if (i.type() != ElementValue)
				throw exception("expected value for key '" + k + "', got '" + i.value() + "'", token.line());
			_attrib[k] = str::stripcslashes(i.value().substr(1, i.size() - 2));
		}
	} catch (Lexer::exception &e) {
		throw exception(e.what(), token.line() + e.line() - 1);
	}

#ifdef CRASH_SIGNAL
	XML::onElementBegin(_data, _attrib);
#endif

	next(token);

	// Scan children
	while (token != xmlScan.end())
		switch (token.type())
		{
			case XmlBegin :
				_child.push_back(new XML(Element, this, token));
			break;
			case XmlDataBegin :
				_child.push_back(new XML(Data, this, token));
			break;
			case XmlEnd :
				if (token.value().substr(2, token.size() - 3) != _data)
					throw exception("expected tag closure for '" + _data + "', got '" + token.value() + "'", token.line());
#ifdef CRASH_SIGNAL
				XML::onElementEnd(_data);
#endif
				next(token);
				return;
			break;
			case XmlContent :
				_child.push_back(new XML(Body, this, token));
			break;
		}
}

void XML::parseBody(Lexer::iterator &token)
{
	skip(token);

	if (token.type() != XmlContent)
		throw exception("expected body, got '" + token.value() + "'", token.line());

	// text is buffered into the buffer, then appended to _data as it fills
char const *s = token.value().c_str();
unsigned size = token.value().size();

	for (unsigned i = 0; i < size; ++i) {
		if (s[i] == '&') {
			if (!strncmp(s + i, "&lt;", 4)) {
				_data += '<';
				i += 3;
			} else
			if (!strncmp(s + i, "&gt;", 4)) {
				_data += '>';
				i += 3;
			} else
			if (!strncmp(s + i, "&amp;", 5)) {
				_data += '&';
				i += 4;
			}
		} else
			_data += s[i];
	}
#ifdef CRASH_SIGNAL
	XML::onBody(*_parent, _data);
#endif

	next(token);
}

void XML::parseData(Lexer::iterator &token)
{
	skip(token);

int skip = 0;

	for (Lexer::iterator i = dataScan.begin(token.source()); i != dataScan.end(); ++i)
	{
		skip += i.size();
		if (i.type() == DataEnd) break;
		_data += i.value();
	}

	token.skip(skip);

#ifdef CRASH_SIGNAL
	XML::onData(*_parent, _data);
#endif
}
