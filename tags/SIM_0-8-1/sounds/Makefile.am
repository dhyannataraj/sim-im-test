####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = auth.wav message.wav url.wav alert.wav sms.wav file.wav filedone.wav chat.wav startup.wav 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons/
	$(mkinstalldirs) $(kde_datadir)/sim/sounds/
	$(INSTALL_DATA) $(EXTRA_DIST) $(kde_datadir)/sim/sounds/

