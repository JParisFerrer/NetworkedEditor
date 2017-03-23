CXX = g++
LDFLAGS = -std=c++11 -lncurses -lpthread
CXXFLAGS = -std=c++11 -c -g
EXENAME = editor

all: $(EXENAME)

$(EXENAME) : main.o editor.o server.o textcontainer.cpp textcontainer.h blockingVector.o networking.o
	$(CXX) main.o editor.o server.o blockingVector.o $(LDFLAGS) -o editor

main.o : main.cpp editor.o server.o
	$(CXX) $(CXXFLAGS) main.cpp

server.o : server.cpp server.h networking.o
	$(CXX) $(CXXFLAGS) server.cpp

editor.o: editor.cpp editor.h networking.o
	$(CXX) $(CXXFLAGS) editor.cpp

networking.o: networking.cpp networking.h
	$(CXX) $(CXXFLAGS) networking.cpp

blockingVector.o: blockingVector.cpp blockingVector.h
	$(CXX) $(CXXFLAGS) blockingVector.cpp

clean:
	-rm *.o ./editor
