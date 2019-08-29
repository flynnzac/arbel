ifeq ($(OS),Windows_NT)
	SUFFIX=dll
	DOC=xsltproc  --xinclude --output docs/arbel.html --stringparam html.stylesheet arbel.css ~/docbook/docbook-xsl-1.79.1/html/docbook.xsl docs/arbel.dbk
else
	SUFFIX=so
	DOC=xsltproc  --output docs/arbel.html --stringparam html.stylesheet arbel.css /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/docbook.xsl docs/arbel.dbk
endif

arbel: arbel.c operator.c primitive.c utility.c parse.c statement.c save.c
	cc -c -fPIC primitive.c utility.c save.c statement.c parse.c -g -lm -Wall -ldl -lreadline
	cc -o libarbel.$(SUFFIX) -fPIC -shared primitive.o utility.o save.o statement.o parse.o
	cc -o arbel arbel.c primitive.c utility.c save.c parse.c statement.c operator.c -lreadline -g -lm -Wall -ldl 

doc: docs/arbel.dbk docs/arbel.css
	$(DOC)
	cat docs/mysite.mro docs/index.mro.html | mro > docs/index.html

examples: examples/link.c libarbel.$(SUFFIX)
	cc -c -fPIC -I. -L. -larbel -lm examples/link.c
	cc -shared -fPIC -o link.$(SUFFIX) -L. -I. link.o -larbel


install: arbel libarbel.$(SUFFIX)
	cp arbel /usr/local/bin/
	cp libarbel.$(SUFFIX) /usr/local/lib/
