bin_PROGRAMS=devtodo
devtodo_SOURCES=Todo.cc Todo.h TodoDB.cc TodoDB.h support.cc support.h \
	main.cc todoterm.cc todoterm.h Loaders.cc Loaders.h todorl.h
binlinks=tda tdr tdd tdl tde todo
	
devtodo_LDADD=../util/libutil.la
INCLUDES=-I../util
LIBS=@READLINE_LIBS@

if DEBUG
AM_CPPFLAGS=-g
else
AM_CPPFLAGS=-DNDEBUG
endif

install-exec-hook:
	cd $(DESTDIR)$(bindir); for i in $(binlinks); do $(LN_S) -f devtodo $$i; done

uninstall:
	rm $(addprefix $(DESTDIR)$(bindir)/, $(binlinks))
