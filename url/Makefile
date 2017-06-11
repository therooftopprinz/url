#Compiler and Linker
CC                 := g++
AR                 := ar
MAKE               := make
CFLAGS             := -std=gnu++14 -ggdb3 -O0 -Wall -Werror

#Top level
TLD                := .
GTEST              := ../libs/gtest.o

#TARGET
URL_TARGET      := ../bin
URL_LIB_TARGET      := ../bin

#The Directories, Source, Includes, Objects, Binary and Resources
URL_SRCDIR         := src
URL_TESTDIR          := tests/

URL_INCDIR         := .
URL_TESTINCDIR     := $(URL_INCDIR) ../gtest
URL_TEST_LD          := pthread m
URL_LD          := pthread m

TESTFLAG			  :=

ifneq ($(strip $(TEST)),)
TESTFLAG := --gtest_filter="$(TEST)"
endif

## TARGET DEBUG #########################################################################

BUILDDIR              := ../build/normal
BUILDDIR_GCOV         := ../build/gcovs

URL_INCDIR_GCC     := $(addprefix -I, $(URL_INCDIR))
URL_TESTINCDIR_GCC := $(addprefix -I, $(URL_TESTINCDIR))
URL_TEST_LD_GCC 	  := $(addprefix -l, $(URL_TEST_LD))
URL_LD_GCC 	  := $(addprefix -l, $(URL_TEST_LD))
URL_SOURCES        := $(shell find $(URL_SRCDIR) -type f -name *.cpp -not -name main.cpp)
URL_TEST_SOURCES   := $(shell find $(URL_TESTDIR) -type f -name *.cpp)
URL_OBJECTS        := $(addprefix $(BUILDDIR)/, $(URL_SOURCES:.cpp=.cpp.o))
URL_OBJECTS_GCOV   := $(addprefix $(BUILDDIR_GCOV)/, $(URL_SOURCES:.cpp=.cpp.o))
URL_TESTS_OBJECTS  	   := $(addprefix $(BUILDDIR)/, $(URL_TEST_SOURCES:.cpp=.cpp.o))
URL_TESTS_OBJECTS_GCOV  := $(addprefix $(BUILDDIR_GCOV)/, $(URL_TEST_SOURCES:.cpp=.cpp.o))

URL_DEPS = $(URL_OBJECTS:%.o=%.d)
URL_TEST_DEPS = $(URL_TESTS_OBJECTS:%.o=%.d)

-include $(URL_DEPS)
-include $(URL_TEST_DEPS)

## TARGET GTEST #########################################################################

$(GTEST):
	@echo "Building gtest..."
	$(MAKE) -C ../gtest all

## TARGET url #########################################################################

url: $(URL_OBJECTS)
	@mkdir -p $(URL_TARGET)
	@echo Archiving $(URL_TARGET)/url.a
	@$(AR) rcs $(URL_TARGET)/url.a $(URL_OBJECTS)

$(URL_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building SOURCE" $@
	@$(CC) $(CFLAGS) $(URL_INCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

## TARGET url UT #########################################################################

url_test: $(GTEST) $(PTREE_STATIC_LIB) $(URL_OBJECTS) $(URL_TESTS_OBJECTS)
	@mkdir -p $(URL_TARGET)
	@echo Linking $(URL_TARGET)/url_test
	@$(CC) -g $(GTEST) $(URL_TEST_LD_GCC) $(URL_OBJECTS) $(URL_TESTS_OBJECTS) $(PTREE_STATIC_LIB) -o $(URL_TARGET)/url_test

url_test_run: url_test
	$(URL_TARGET)/url_test $(TESTFLAG)

url_test_valgrind_run: url_test
	valgrind --leak-check=full --show-leak-kinds=all -v $(URL_TARGET)/url_test $(TESTFLAG)

url_test_valgrind_run_detailed: url_test
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -v $(URL_TARGET)/url_test $(TESTFLAG)

$(URL_TESTS_OBJECTS): $(BUILDDIR)/%.cpp.o : %.cpp
	@mkdir -p $(@D)
	@echo "Building TEST" $@
	@$(CC) $(CFLAGS) $(URL_TESTINCDIR_GCC) -MMD -g -c $(patsubst $(BUILDDIR)/%.cpp.o,%.cpp,$@) -o $@

clean_url_test:
	echo cleaning ut
	find $(BUILDDIR)/$(URL_TESTDIR) -type f -name *.cpp.o -exec rm {} \;

clean_url: clean_url_test
	echo cleaning url
	find $(BUILDDIR)/$(URL_SRCDIR) -type f -name *.cpp.o -exec rm {} \;

clean_gcov:
	echo cleaning GCOV
	find $(BUILDDIR_GCOV) -type f -name *.cpp.o -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcda -exec rm {} \;
	find $(BUILDDIR_GCOV) -type f -name *.gcno -exec rm {} \;

clean: clean_client_test clean_client clean_url_test clean_url clean_gcov
	echo All cleaned
