CFLAGS=-g -O2 -Wall -Wextra -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS=-ldl $(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

#SOURCESMAIN=$(wildcard src/winn_pack.c)
SOURCESMAIN=$(wildcard src/pack.c)
OBJECTSMAIN=$(patsubst %.c,%,$(SOURCESMAIN))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/liblx.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET) tests main 

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@
$(SO_TARGET): $(TARGET) $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS)
build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
tests: $(TESTS) 
	sh ./tests/runtests.sh

$(TESTS):
	$(CC) $(CFLAGS) -o $@ $(findstring $@.c, $(TEST_SRC)) $(TARGET) 

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Main Function
main: $(OBJECTSMAIN)
$(OBJECTSMAIN): 
	$(CC) $(CFLAGS) -o $@ $(SOURCESMAIN) $(TARGET)
	@mv $@ bin

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(OBJECTSMAIN) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true
