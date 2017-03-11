CXX = g++
LDFLAGS = -std=c++11 -lncurses
CXXFLAGS = -std=c++11 -c

all: editor

editor: editor.o textcontainer.cpp textcontainer.h blockingVector.o
	$(CXX) editor.o blockingVector.o $(LDFLAGS) -o editor

editor.o: editor.cpp
	$(CXX) $(CXXFLAGS) editor.cpp

blockingVector.o: blockingVector.cpp blockingVector.h
	$(CXX) $(CXXFLAGS) blockingVector.cpp

clean:
	-rm ./editor *.o
