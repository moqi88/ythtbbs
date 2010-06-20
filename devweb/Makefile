.PHONY:install
TARGET:=/home/bbs/ftphome/root/boards/BBSDev/html
install:
	[ ! -d  $(TARGET) ] || mkdir -p $(TARGET)
	cp *.html *.gif $(TARGET)
	cp index.html $(TARGET)/index.htm
	$(MAKE) -C ../doc html;cp ../doc/*.html $(TARGET)
