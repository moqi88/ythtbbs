BASEPATH = ../..
include $(BASEPATH)/Makefile.Base

CLASSPATH = ./jcommon-0.9.0.jar:./jfreechart-0.9.15.jar:../JPlot/:.

all: JPlot.jar

JPlot.class: JPlot.java
	javac -classpath $(CLASSPATH) JPlot.java

JPlot.jar: JPlot.class
	jar cf JPlot.jar JPlot.class

install: JPlot.jar
	cp jcommon-0.9.0.jar jfreechart-0.9.15.jar JPlot.jar $(HTMPATH)/

clean:
	rm -rf *.class
