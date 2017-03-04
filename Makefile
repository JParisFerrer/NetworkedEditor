CXX = g++
LDFLAGS = -lncurses
CXXFLAGS = -std=c++11 -c

all: editor

editor: editor.o textcontainer.o
	$(CXX) editor.cpp $(LDFLAGS) -o editor

editor.o: editor.cpp
	$(CXX) $(CXXFLAGS) editor.cpp

textcontainer.o: textcontainer.cpp textcontainer.h
	$(CXX) $(CXXFLAGS) textcontainer.cpp

clean:
	rm ./editor *.o
