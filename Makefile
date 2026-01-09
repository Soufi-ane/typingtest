FLAGS = -Iinclude -lraylib  -lm -lpthread -ldl -lrt -lGL
PRODUCTION_FLAGS = -DRELEASE -O3 -s -DNDEBUG -march=native 
EXTRA_FLAGS = -Wall -Wextra -Werror -Wpedantic 

INSTALL_DIR = /usr/local/bin
FONTS_DIR   = /usr/local/share/typingtest/fonts
ICONS_DIR   = /usr/local/share/typingtest/icons
AUDIOS_DIR  = /usr/local/share/typingtest/audios
HOME_DIR   = $(shell getent passwd $(SUDO_USER) | cut -d: -f6)
LOG_DIR     = $(HOME_DIR)/.local/share/typingtest

CC = gcc
SRC = src/*.c
OUT = build/typingtest
RELEASE = build/release/typingtest

all: $(OUT)

run : clean $(OUT)
	./$(OUT)

install: $(RELEASE) appdata
	mkdir -p build/release/
	mkdir -p $(INSTALL_DIR) $(LOG_DIR)
	chown $(SUDO_USER):$(SUDO_USER) $(LOG_DIR)
	cp $(RELEASE) $(INSTALL_DIR)

$(OUT): $(SRC)
	mkdir -p build/release/
	$(CC) -o $(OUT) $(SRC) $(FLAGS)

$(RELEASE) : clean $(SRC)
	$(CC) $(PRODUCTION_FLAGS) \
		-o $(RELEASE) $(SRC) $(FLAGS) $(EXTRA_FLAGS)

debug : clean $(SRC)
	$(CC) -g -o $(OUT) $(SRC) $(FLAGS) $(EXTRA_FLAGS)
		
appdata : 
	mkdir -p $(FONTS_DIR) $(ICONS_DIR) $(AUDIOS_DIR)
	cp ./assets/fonts/* $(FONTS_DIR)
	cp ./assets/icons/* $(ICONS_DIR)
	cp ./assets/audios/* $(AUDIOS_DIR)

clean:
	rm -f $(OUT) $(RELEASE)

