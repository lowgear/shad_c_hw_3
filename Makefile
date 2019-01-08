LD=/usr/bin/g++
CXX=/usr/bin/g++
CC=/usr/bin/gcc

SRCD=.
RM_ALL=rm -f

INCLUDES=-I$(GTEST) -I$(GTEST)/include -I$(SRCD)
WARN_OPTS=-Wall -Werror -pedantic
LDFLAGS=$(FLAGS) -lm -lpthread

CFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c11

CXXFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c++17

LINK_EXECUTABLE=$(LD) $(LDFLAGS) -o $@ $^

COMPILE_CXX_SRC=$(CXX) $(CXXFLAGS) -c -o $@ $^

COMPILE_C_SRC=$(CC) $(CFLAGS) -c -o $@ $^

all: lisp check;

lisp: main.o builtins.o evaluation.o lisp_io.o models.o iotools.o strtools.o
	$(LINK_EXECUTABLE)

main.o: $(SRCD)/main.c
	$(COMPILE_C_SRC)

builtins.o: $(SRCD)/builtins.c
	$(COMPILE_C_SRC)

evaluation.o: $(SRCD)/evaluation.c
	$(COMPILE_C_SRC)

lisp_io.o: $(SRCD)/lisp_io.c
	$(COMPILE_C_SRC)

models.o: $(SRCD)/models.c
	$(COMPILE_C_SRC)

iotools.o: $(SRCD)/utils/iotools.c
	$(COMPILE_C_SRC)

strtools.o: $(SRCD)/utils/strtools.c
	$(COMPILE_C_SRC)

check: test_main.o gtest-all.o builtins.o evaluation.o lisp_io.o models.o iotools.o strtools.o \
endToEnd_tests.o iotools_tests.o models_tests.o strtools_tests.o vector_tests.o
	$(LINK_EXECUTABLE)

test_main.o: $(SRCD)/test_main.cpp
	$(COMPILE_CXX_SRC)

endToEnd_tests.o: $(SRCD)/tests/endToEnd_tests.cpp
	$(COMPILE_CXX_SRC)

iotools_tests.o: $(SRCD)/tests/iotools_tests.cpp
	$(COMPILE_CXX_SRC)

models_tests.o: $(SRCD)/tests/models_tests.cpp
	$(COMPILE_CXX_SRC)

strtools_tests.o: $(SRCD)/tests/strtools_tests.cpp
	$(COMPILE_CXX_SRC)

vector_tests.o: $(SRCD)/tests/vector_tests.cpp
	$(COMPILE_CXX_SRC)

gtest-all.o: $(GTEST)/src/gtest-all.cc
	$(COMPILE_CXX_SRC)

TO_DELETE=lisp check

clean:
	$(RM_ALL) *.o $(TO_DELETE) $(TO_DELETE:=.exe) $(TO_DELETE:=.bin)
