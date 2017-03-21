CXX = g++
LDFLAGS = -std=c++11 -lncurses
CXXFLAGS = -std=c++11 -c -g
EXENAME = editor

all: $(EXENAME)

$(EXENAME) : main.o editor.o server.o textcontainer.cpp textcontainer.h blockingVector.o
	$(CXX) main.o editor.o server.o blockingVector.o $(LDFLAGS) -o editor

main.o : main.cpp
	$(CXX) $(CXXFLAGS) main.cpp

server.o : server.cpp server.h
	$(CXX) $(CXXFLAGS) server.cpp

editor.o: editor.cpp editor.h
	$(CXX) $(CXXFLAGS) editor.cpp

blockingVector.o: blockingVector.cpp blockingVector.h
	$(CXX) $(CXXFLAGS) blockingVector.cpp

clean:
	-rm ./editor *.o
