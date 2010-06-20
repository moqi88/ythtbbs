SUBDIRS = . ythtlib libythtbbs src nju09 local_utl
.PHONY: install-exec-local
install-exec-local:
	./Installbbs.sh
update:
	$(MAKE) -C src install
	$(MAKE) -C local_utl install
	$(MAKE) -C nju09 install
