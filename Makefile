CXX := clang++
#CXX_FLAGS := -mmacosx-version-min=12.6 -O3 -std=c++17 -ggdb -lncurses `pkg-config --cflags allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5`
CXX_FLAGS := -mmacosx-version-min=12.6 -O3 -std=c++17 -ggdb -lncurses -I/usr/local/include -rpath /usr/local/lib


BIN := bin
SRC := src
INCLUDE := include

#LIBRARIES := `pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5`
LIBRARIES := -L/usr/local/lib -lallegro -lallegro_font -lallegro_ttf -lallegro_image -lallegro_primitives -lallegro_audio -lallegro_acodec -lsqlite3
EXECUTABLE := main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)

clean:
	-rm -r $(BIN)/*
