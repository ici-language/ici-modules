NAME=ici-modules
VERSION=0.1.0

SUBDIRS =\
	tools\
	is\
	X\
	Imlib\
	cgi\
	html\
	http\
	net\
	path\
	posix\
	ps\
	serialisation\
	str\
	sys\
	tcl\
	utype\
	wings\
	xml\
	gl\
	glx\
	app\
	env\
	ppp\
	gtk\
	pq\
	shout\
	misc\
	dll\
	bignum\
	uuid\
	is

all\
 clean\
 depend\
 install\
 ici.h strs.h strings.c cfuncs.c\
 uninstall: config
	@set -e;\
	for flg in $(MAKEFLAGS) '';\
	do\
	    case $$flg in\
	    *=*) ;;\
	    *k*) set +e;;\
	    esac;\
	done;\
	for d in $(SUBDIRS); do\
	(\
	 cd "$$d";\
	 if [ -f Makefile ];\
	 then\
	  echo "==== make $@ in $$d";\
	  $(MAKE) $@;\
	  echo;\
	 fi\
	);\
	done

clobber:
	@rm -f .config config.log;\
	for d in $(SUBDIRS); do\
	(\
	  cd "$$d";\
	  if [ -f Makefile ];\
	  then\
	    echo "==== make $@ in $$d";\
	    $(MAKE) $@;\
	    echo;\
	    rm -f Makefile;\
	  fi\
	);\
	done

config: configure .config

.config:
	@ici configure

tar:
	tar -cT MANIFEST -f ../$(NAME)-$(VERSION).tar .
	ls -l ../$(NAME)-$(VERSION).tar

tgz:
	tar -cT MANIFEST -f - . | gzip -9 -c > ../$(NAME)-$(VERSION).tar.gz
	ls -l ../$(NAME)-$(VERSION).tar.gz

MANIFEST: FORCE
	find . -type f|sort|egrep -v -e CVS/ -e '/\.cvsignore' -e '\.o$$' -e '\.so$$' > MANIFEST

FORCE:
