CXX = g++
LDFLAGS = -std=c++11 -lncurses -lpthread
CXXFLAGS = -std=c++11 -c -g

all: editor

editor: editor.o textcontainer.cpp textcontainer.h blockingVector.o lockfreelist.o
	$(CXX) editor.o blockingVector.o lockfreelist.o $(LDFLAGS) -o editor

editor.o: editor.cpp
	$(CXX) $(CXXFLAGS) editor.cpp

blockingVector.o: blockingVector.cpp blockingVector.h
	$(CXX) $(CXXFLAGS) blockingVector.cpp

lockfreelist.o: lockfreelist.cpp lockfreelist.h
	$(CXX) $(CXXFLAGS) lockfreelist.cpp

clean:
	-rm ./editor *.o
