CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=koma/core/game_object_manager.cpp koma/core/game.cpp koma/core/time_manager.cpp koma/core/input_manager.cpp \
	koma/core/rendering_manager.cpp koma/core/physics_manager.cpp koma/game_object/game_object.cpp koma/main.cpp \
	koma/utils/subject.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=komaengine
EXEDIR=build

all: $(SOURCES) $(EXEDIR)/$(EXECUTABLE)
    
$(EXEDIR)/$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(EXEDIR)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	find . -name '*.o' -exec rm -r {} \;

remove:
	rm -Rf $(EXEDIR)
