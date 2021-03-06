CXX = g++
LDFLAGS = -std=c++11 -lncurses -lpthread
CXXFLAGS = -std=c++11 -c -g -O0
EXENAME = editor

ifeq ($(LOCKFREE), 1)
CXXFLAGS += -DLOCKFREE
LDFLAGS += -DLOCKFREE
endif

all: $(EXENAME)

$(EXENAME) : main.o editor.o server.o textcontainer.cpp textcontainer.h blockingVector.o lockfreelist.o networking.o
	$(CXX) main.o editor.o server.o blockingVector.o lockfreelist.o networking.o $(LDFLAGS) -o editor

main.o : main.cpp editor.h server.h
	$(CXX) $(CXXFLAGS) main.cpp

server.o : server.cpp server.h networking.h
	$(CXX) $(CXXFLAGS) server.cpp

editor.o: editor.cpp editor.h networking.h
	$(CXX) $(CXXFLAGS) editor.cpp

networking.o: networking.cpp networking.h
	$(CXX) $(CXXFLAGS) networking.cpp

blockingVector.o: blockingVector.cpp blockingVector.h
	$(CXX) $(CXXFLAGS) blockingVector.cpp

lockfreelist.o: lockfreelist.cpp lockfreelist.h
	$(CXX) $(CXXFLAGS) lockfreelist.cpp

clean:
	-rm *.o ./editor
