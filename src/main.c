#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include "raymedia.h"
#include "type.h"

void getResult(Test* test){
  if(test->mistakeCount < 1) test->accuracy = 1.0f;
  else test->accuracy =  (float) (test->length - test->mistakeCount) / test->length;
}

void handle_normal_mode_keys(Test* test,char c,char* charState){
  switch(c) {
    case 'i' : 
      test->mode = TYPING;
    case 'r' : 
      test->currentChar = 0;
      test->currentIndex = 0;
      test->currentWord = 0;
      test->length = 0;
      getTestWords(test);
      memset(charState,0,test->length * sizeof(char));
      test->isFinished = 0;
  }
}

void handleKeys(Test* test,char* charState,Sound sounds[]) {
  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_CAPS_LOCK)) {
    test->mode = NORMAL;
  }
  int c; 
  if(test->length <= test->currentIndex && test->mode == TYPING){
    test->isFinished = 1;
    test->mode = NORMAL;
    return;
  }
  if((c = GetCharPressed()) >= 7) {
    if (test->mode == NORMAL) handle_normal_mode_keys(test,c,charState);
    else {
      int randomIndex = rand() % 7;
      PlaySound(sounds[randomIndex]);
      if(test->currentChar > test->length - 1 ) return;
      if(test->words[test->currentWord][test->currentChar] != c){
        if(test->currentChar < strlen(test->words[test->currentWord]) - 1
           || c != ' ' ) {
          charState[test->currentIndex] = 'F'; // for false 
          test->mistakeCount++;
          PlaySound(sounds[7]);
        }
      } 
      if(test->currentChar < strlen(test->words[test->currentWord])){
        test->currentChar++;
      }else {
        test->currentChar = 0;
        test->currentWord++;
      }
      test->currentIndex++;
    }
  }
}

void loadFontSDF(
		char* path,
		int* fileSize,
		int size,
		Font* font
){
	unsigned char* fontFile = LoadFileData(path,fileSize);
	if(fontFile == NULL || fontFile == 0){
		fontFile = LoadFileData("assets/fonts/JetBrainsMonoNF.ttf",fileSize);
	}
	font->baseSize = size;
	font->glyphCount = 95;
	font->glyphs = LoadFontData(fontFile,*fileSize,size,0,0,FONT_SDF);
	Image atlas = GenImageFontAtlas(font->glyphs,&font->recs,95,size,0,1);
	font->texture = LoadTextureFromImage(atlas);
	UnloadImage(atlas);
	UnloadFileData(fontFile) ;
	SetTextureFilter(font->texture,TEXTURE_FILTER_BILINEAR);
}



int main() {
  Sound allSounds[SOUND_NUM];
  srand(time(NULL));
  Test test = {
    .currentChar = 0,
    .currentWord = 0,
    .currentIndex = 0,
    .mistakeCount = 0,
    .isFinished = 0,
    .accuracy = 0.0f,
    .mode = NORMAL,
  };
  getTestWords(&test);
  for(int x=0; x < WORDS_COUNT;x++){
    printf("words %d = [%s]\n",x+1,test.words[x]);
  }
  int fileSize = 0;
  Font fontSDF = {0};
  char* charState = malloc(test.length * sizeof(char));
  memset(charState,0,test.length * sizeof(char));

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Type");
  SetExitKey(KEY_NULL);
  loadFontSDF(
      // "assets/fonts/JetBrainsMonoNF.ttf",
      "/usr/local/share/fonts/JetBrainsMonoNF.ttf",
      &fileSize,
      128,
      &fontSDF);

  InitAudioDevice();
  for(int i = 0 ; i < SOUND_NUM /* number of files */; i++){
    Sound s = LoadSound(TextFormat("assets/%s",audioFileNames[i]));
    allSounds[i] = s;
  }
  MediaStream media = LoadMedia("assets/charlie_scream.mp4");

  SetTargetFPS(60);
  int hasPlayedCharlieSound = 0;
  while (!WindowShouldClose()) {
    if(test.isFinished && !hasPlayedCharlieSound){
      getResult(&test);
      if(test.accuracy == 1.0f){
        PlaySound(allSounds[8]);
        hasPlayedCharlieSound = 1;
      } 
    } 
    if (test.mode != TYPING) {
      SetExitKey(KEY_Q);
    } else if (test.mode == TYPING) {
      SetExitKey(KEY_NULL);
    }
    handleKeys(&test,charState,allSounds);
    BeginDrawing();
    ClearBackground(BLACK);
    int y_offset = 0;
    int i,x_offset;
    int last_space = 0;
    int charIndex = 0;
    if(!test.isFinished){
      for(int i = 0,x_offset=0; i < WORDS_COUNT ;i++){
        int wordLength = strlen(test.words[i]);
        if(x_offset + wordLength >= 80) {
          y_offset++;
          x_offset = 0;
        }
        for(int j=0; j <= wordLength ;j++,x_offset++,charIndex++){
          if(test.words[i][j] == ' ') last_space = i;
          DrawTextCodepoint(fontSDF,
            j == wordLength ? ' ' : test.words[i][j],
            (Vector2){LINE_X_POSITION + 14 * x_offset,
            LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset)},
            32,
            (test.currentIndex < charIndex + 1) ? GRAY :
            (charState[charIndex] == 'F' ? RED : WHITE ));
        }
      }
    } else {
      if(test.accuracy == 1.0f){
        UpdateMedia(&media);
        DrawTextureEx(media.videoTexture, (Vector2){0,0}, 0.0f, 1.5f, WHITE);
      }
      const char* accuracyText = TextFormat("Accuracy : %.2f %%",test.accuracy * 100);
      Vector2 accuracySize = MeasureTextEx(fontSDF, accuracyText, 90, 4);
      DrawTextEx(fontSDF, accuracyText ,
       (Vector2){SCREEN_WIDTH / 2 - accuracySize.x /2 , SCREEN_HEIGHT/ 2 + accuracySize.y},
       90, 4, GREEN);
    }
    // Mode
    DrawTextEx(fontSDF, test.mode == NORMAL ? "NORMAL" : "TYPING" , (Vector2){15,7}, 44, 4, GRAY);
    EndDrawing();
  }
  UnloadMedia(&media);
  CloseWindow();
  return 0;
}
