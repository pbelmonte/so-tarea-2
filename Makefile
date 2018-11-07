# Para usar este Makefile es necesario definir la variable
# de ambiente NSYSTEM con el directorio en donde se encuentra
# la raiz de nSystem.  En csh esto se hace con:
#
#   setenv NSYSTEM ~cc41b/nSystem2008
#
# Elegir una entre los siguientes ejemplos
#
# msgprodcons iotest test term-serv
#

LIBNSYS= $(NSYSTEM)/lib/libnSys.a

CFLAGS= -ggdb -Wall -pedantic -I$(NSYSTEM)/include -I$(NSYSTEM)/src
LFLAGS= -ggdb

all: testmon

.SUFFIXES:
.SUFFIXES: .o .c .s

.c.o .s.o:
	gcc -c $(CFLAGS) $<

testmon: testmon.o $(LIBNSYS)
	gcc $(LFLAGS) testmon.o -o $@ $(LIBNSYS)

clean:
	rm -f *.o *~

cleanall:
	rm -f *.o *~ testmon
