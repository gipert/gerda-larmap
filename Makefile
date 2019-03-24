# Makefile
#
# Author: Luigi Pertoldi - pertoldi@pd.infn.it
# Created: Sun 24 Mar 2019
#
FLAGS  = -std=c++11 -g -O3 -Wall
FLAGS += $$(root-config --libs --cflags) -lTreePlayer
FLAGS += $$(gerda-ada-config --libs --cflags)
FLAGS += $$(gelatio-config --libs --cflags)
FLAGS += $$(mgdo-config --libs --cflags)
FLAGS += $$(databricxx-config --libs --cflags)
FLAGS += -Iprogressbar

all : create-larmap larmap-doctor map-merger

create-larmap : create-larmap.cc progressbar/ProgressBar.cc
	c++ $(FLAGS) -o $@ $^

larmap-doctor : larmap-doctor.cc progressbar/ProgressBar.cc
	c++ $(FLAGS) -o $@ $^

map-merger : map-merger.cc
	c++ $(FLAGS) -o $@ $^

clean :
	-rm create-larmap larmap-doctor map-merger
