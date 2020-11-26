LINKER ?= g++
CC ?= gcc
CXX ?= g++
CFLAGS_BASE = -Wall -Wextra 
CFLAGS_CILK = -DCILK -fcilkplus
CFLAGS_GTEST = -lgtest_main -lgtest -lpthread

ifeq ($(MODE),RELEASE)
	CFLAGS_OPT += -O3
else ifeq ($(MODE),DEBUG)
	CFLAGS_OPT += -O0 -g3
endif

LDFLAGS := -lcilkrts
CFLAGS := $(CFLAGS_BASE) $(CFLAGS_CILK) $(CFLAGS_OPT)
CXXFLAGS := $(CFLAGS) -std=c++11

########### Change the test cases here ###########
ITEM_SRC = scan.cpp filter.cpp listRanking.C listRanking3.C
##################################################
ITEM_TEST := $(ITEM_SRC:%.C=%)
ITEM_TEST := $(ITEM_TEST:%.cpp=%)
ITEM_TEST := $(ITEM_TEST:%=test_%)
ITERM_EXEC := $(ITEM_TEST:test_%=bin/%.test)


.PHONY : test list build clean $(ITEM_TEST)

test : $(ITEM_TEST)

build : $(ITERM_EXEC)

list :
	@echo 'Available Tests:'
	@for name in $(ITEM_TEST);\
	do\
		echo $$name;\
	done 

clean :
	@-rm -r depend/*
	@-rm -r bin/*

$(ITEM_TEST) : build
	@echo '====== Running Test $(@:test_%=%) ======'
	-$(@:test_%=bin/%.test) `cat $(@:test_%=test/%.arg)`
	@echo '----------------------------------------'

%.test : %.o ; $(LINKER) $< $(LDFLAGS) -o $@
bin/%.o : %.C ; $(CC) $< $(CFLAGS) -c -o $@
bin/%.o : %.cpp ; $(CXX) $< $(CXXFLAGS) -c -o $@

depend/%.C.d : %.C
	$(CC) $< $(CFLAGS) -MM > $@

depend/%.cpp.d : %.cpp
	$(CXX) $< $(CXXFLAGS) -MM > $@

sinclude $(ITEM_SRC:%=depend/%.d)