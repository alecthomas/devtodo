man_MANS=devtodo.1
manlinks=tda tdr tdd tde tdl todo

install-data-local:
	mkdir -p $(DESTDIR)$(mandir)/man1
	cd $(DESTDIR)$(mandir)/man1; for i in $(manlinks); do $(LN_S) -f devtodo.1 $$i.1; done
	test -f $(DESTDIR)$(sysconfdir)/todorc || ( mkdir -p $(DESTDIR)$(sysconfdir) ; $(INSTALL_DATA) todorc.example $(DESTDIR)$(sysconfdir)/todorc )

uninstall:
	rm -f $(addprefix $(DESTDIR)$(mandir)/man1/, $(addsuffix .1, $(manlinks) devtodo))
	rm -f $(DESTDIR)$(sysconfdir)/todorc
