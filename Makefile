EXE=BlockGame
SRC_DIR=src

CC=g++
STD=c++20

WARNINGS=-Wsuggest-override -Wno-switch -Wno-parentheses -Wvolatile -Wextra-semi -Wimplicit-fallthrough -Wsequence-point
INCLUDES=-Ivendor/GLEW/include -Ivendor/GLFW/include -Ivendor/stb -Ivendor/rnd -Ivendor/noise -Ivendor/meow_hash -Ivendor/vstd -Ivendor/imgui/imgui -Ivendor/imgui/extra -Ivendor/implot/implot -Ivendor/implot/extra

CXXFLAGS=--std=$(STD) $(INCLUDES) -ftabstop=4 -fmax-errors=15 $(WARNINGS) -mavx -mavx2 -maes -fno-exceptions -fconcepts-diagnostics-depth=5 # -Wall -Wextra -pedantic # -Werror -nostdinc++
LDFLAGS_COMMON=-Lvendor/imgui/lib -Lvendor/implot/lib -l:imgui.a -l:implot.a -l:libglew32.a -l:libglfw3.a -lm -lpthread

ifeq ($(OS),Windows_NT)
LDFLAGS=$(LDFLAGS_COMMON) -Lvendor/GLEW/lib/win32 -Lvendor/GLFW/lib/win32 -lopengl32 -lgdi32
else
LDFLAGS=$(LDFLAGS_COMMON) -Lvendor/GLEW/lib/linux -Lvendor/GLFW/lib/linux -lX11 -ldl -lGL
endif

CTIME := $(shell command -v ctime 2> /dev/null)

.PHONY: all debug release clean run ctbegin ctend imgui implot

all: debug

debug: imgui
debug: implot
debug: clean
debug: CXXFLAGS+=-O0
debug: CXXFLAGS+=-g
debug: CXXFLAGS+=-DBG_DEBUG -Ivendor/cpptrace/include
debug: LDFLAGS+=-Lvendor/cpptrace/build -lcpptrace -ldwarf -lz
debug: $(EXE)

release: imgui
release: implot
release: clean
release: CXXFLAGS+=-O2
release: CXXFLAGS+=-DBG_RELEASE
release: $(EXE)

clean:
	rm -f $(EXE)

run: debug
	./$(EXE)

imgui:
	cd vendor/imgui && $(MAKE)

implot:
	cd vendor/implot && $(MAKE)

$(EXE):
	$(CC) $(CXXFLAGS) -o $(EXE) src/main.cpp $(LDFLAGS)