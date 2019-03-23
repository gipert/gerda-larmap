create-larmap : create-larmap.cc progressbar/ProgressBar.cc
	c++ -std=c++11 -g -O3 -Wall -o $@ $^             \
        $$(root-config --libs --cflags) -lTreePlayer \
        $$(gerda-ada-config --libs --cflags)         \
        $$(gelatio-config --libs --cflags)           \
        $$(mgdo-config --libs --cflags)              \
        $$(databricxx-config --libs --cflags)        \
        -Iprogressbar

clean :
	-rm create-larmap
