# This comes from xml/Makefile.frag...
CFLAGS	= $(CF_PIC) $(CF_OPTIM) $(CF_CFLAGS) -I. -I$(CF_ICI_H_DIR) -Iexpat/xmlparse -Iexpat/xmltok
OBJS	= xml.o xmlparse.o xmltok.o xmlrole.o

xmlparse.o: expat/xmlparse/xmlparse.c
	$(CC) $(CFLAGS) -c expat/xmlparse/xmlparse.c

xmltok.o: expat/xmltok/xmltok.c
	$(CC) $(CFLAGS) -c expat/xmltok/xmltok.c

xmlrole.o: expat/xmltok/xmlrole.c
	$(CC) $(CFLAGS) -c expat/xmltok/xmlrole.c

# end of xml/Makefile.frag
