# Makefile
#
# Author: Luigi Pertoldi - pertoldi@pd.infn.it
# Created: Sun 24 Mar 2019
#
CXX    = c++
FLAGS  = -std=c++11 -g -O3 -Wall
FLAGS += $$(root-config --libs --cflags) -lTreePlayer
FLAGS += $$(gerda-ada-config --libs --cflags)
FLAGS += $$(gelatio-config --libs --cflags)
FLAGS += $$(mgdo-config --libs --cflags)
FLAGS += $$(databricxx-config --libs --cflags)
FLAGS += -Iprogressbar

EXE = bin/create-larmap bin/larmap-doctor bin/map-merger bin/map-smoother

all : init $(EXE)

init :
	@mkdir -p bin

bin/% : %.cc progressbar/ProgressBar.cc
	$(CXX) $(FLAGS) -o $@ $^

clean :
	-rm $(EXE)
