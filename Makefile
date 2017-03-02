LDFLAGS = -lncurses
CXXFLAGS = -std=c++11 

all: editor

editor: editor.cpp
	g++ editor.cpp $(CXXFLAGS) $(LDFLAGS) -o editor

