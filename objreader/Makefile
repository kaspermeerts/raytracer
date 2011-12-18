CC = gcc
CFLAGS = -I..
all:

libobjreader.a: objparser.y.o objscanner.l.o mtlparser.y.o mtlscanner.l.o
	@$(AR) -cq libobjreader.a  objscanner.l.o objparser.y.o mtlscanner.l.o mtlparser.y.o
	@echo "	AR libobjreader.a"

.c.o:
	@echo "	CC	$@"
	@$(CC) -c -o $@ $(CFLAGS) $<
	
objscanner.l.c:
	@echo "	FLEX	objscanner.l.c"
	@flex -o objscanner.l.c wavefrontobj.l

objparser.y.c:
	@echo "	BISON	objparser.y.c"
	@bison -d -o objparser.y.c wavefrontobj.y

mtlscanner.l.c:
	@echo "	FLEX	mtlscanner.l.c"
	@flex -o mtlscanner.l.c mtl.l

mtlparser.y.c:
	@echo "	BISON	mtlparser.y.c"
	@bison -d -o mtlparser.y.c mtl.y

