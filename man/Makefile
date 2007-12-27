MANPAGES := hardware-connected.1

all : $(MANPAGES)

.SUFFIXES :
.SUFFIXES : .1 .1.xml .1.txt

%.1.xml : %.1.txt
	asciidoc -d manpage -b docbook -o "$@" "$<"

%.1 : %.1.xml
	xmlto man "$<"

.PHONY : clean
clean ::
	$(RM) $(MANPAGES) $(MANPAGES:%=%.xml)
