.PHONY: all html text clean
SGMLFILE := INSTALL.cygwin.sgml
TEXTFILE := $(SGMLFILE:.sgml=.txt)
HTMLFILE := $(SGMLFILE:.sgml=.html)

%.txt: %.sgml
	sgml2txt --charset=latin $^
%.html: %.sgml
	gbsgml2html --split=0 --toc=2 $^

all: html text

html: $(HTMLFILE)
	

text: $(TEXTFILE)
	

clean:
	rm -f $(HTMLFILE) $(TEXTFILE)
