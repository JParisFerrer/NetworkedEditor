CXX = g++
LDFLAGS = -std=c++11 -lncurses
CXXFLAGS = -std=c++11 -c

all: editor

editor: editor.o textcontainer.cpp textcontainer.h
	$(CXX) editor.cpp $(LDFLAGS) -o editor

editor.o: editor.cpp
	$(CXX) $(CXXFLAGS) editor.cpp

clean:
	rm ./editor *.o
