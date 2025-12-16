Flags = -Iinclude -lraylib  -lm -lpthread -ldl -lrt -lGL
RayMediaFlags =  -lavcodec -lswresample -lavformat -lavutil -lswscale -lm
Debug = -g -Wall
InstallDir = /usr/local/bin
FontsDir = /usr/local/share/fonts
AssetsDir = ./assets
FontName = JetBrainsMonoNF.ttf

CC = gcc
SRC = src/*.c
OUT = build/type

all: $(OUT)

run : $(OUT)
	./$(OUT)

install: $(OUT) fonts 
	mkdir -p $(InstallDir)
	cp $(OUT) $(InstallDir)
	fc-cache -fv

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(Flags) $(RayMediaFlags) 

debug : $(SRC)
	# $(CC) -o $(OUT) $(SRC) $(Flags) $(Debug)
	$(CC) -o $(OUT) $(SRC) $(Flags) $(RayMediaFlags) -g
		
fonts : 
	mkdir -p $(FontsDir)
	cp $(AssetsDir)/fonts/$(FontName) $(FontsDir)

clean:
	rm -f $(OUT)


