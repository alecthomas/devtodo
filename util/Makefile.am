noinst_LTLIBRARIES=libutil.la
libutil_la_SOURCES=Terminal.cc Terminal.h Lexer.cc Lexer.h \
	Regex.cc Regex.h XML.cc XML.h Strings.cc Strings.h CommandArgs.cc CommandArgs.h

rebuild:
	crash-module -veto --exclude Signal --module XML Strings Terminal CommandArgs --join 50,lib
