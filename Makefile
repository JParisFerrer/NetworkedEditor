LDFLAGS = -lncurses

all: editor

editor: editor.cpp
	clang++ editor.cpp $(LDFLAGS) -o editor

