.PHONY: install
install:mod_fastrw.c ../mod_fastrw.h
	apxs -i -I .. -a -c mod_fastrw.c 
../mod_fastrw.h: ../../include/bbsconfig.h
	$(MAKE) -C .. mod_fastrw.h
