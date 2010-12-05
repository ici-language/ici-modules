#
# This osx specific makefile was automatically created by
# processing Makefile.generic to get archive/Makefile.
#
# See the comments in configure for details.
#
PREFIX	= /usr/local
MOD	= archive
CFLAGS	= -fno-common -g -O0 -DNDEBUG -DICI_MODULE_NAME=$(MOD) -I. -I/usr/local/include
LDFLAGS = -flat_namespace -bundle -undefined suppress
OBJS	= module.o restore.o save.o 
C_SRC	= module.c restore.c save.c 
ALL_SRC	= $(C_SRC) 
ICIMOD	= ici4archive.bundle
LIBS	= 
LIBINSTALLS = $(ICIMOD) 

default: $(ICIMOD)

$(ICIMOD): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) 

doc: ici$(MOD).1 ici$(MOD).html $(MOD).man.frag $(MOD).synopsis

ici$(MOD).1 $(MOD).man.frag $(MOD).synopsis: $(ALL_SRC)
	ici ../tools/extract-doc-from-src.ici man $(ALL_SRC)

ici$(MOD).html: $(ALL_SRC)
	ici ../tools/extract-doc-from-src.ici html $(ALL_SRC)

clean:
	-rm -f $(OBJS) $(MOD).man.frag $(MOD).html.frag $(MOD).synopsis

clobber: clean
	-rm -f $(ICIMOD) ici$(MOD).1 ici$(MOD).html .depend

depend:
	mkdep $(CFLAGS) $(C_SRC)

test tst:
	ici tst-$(MOD).ici

install: $(LIBINSTALLS) ici$(MOD).1 ici$(MOD).html
	install -c -m 444 $(LIBINSTALLS) $(PREFIX)/lib/ici4
	install -c -m 444 ici$(MOD).1 $(PREFIX)/man/man1
