#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "type.h"

bool isDebugMode = false;

void getResult(Test* test){
  if(test->mistakeCount < 1 || test->length < 1) test->accuracy = 1.0f;
  else test->accuracy =  (float) (test->length - test->mistakeCount) / test->length;
  float wordsTyped = (float) test->length / 5; 
  float typingTime = test->finishedAt - test->startedAt;
  float raw =  wordsTyped * (60 / typingTime);
  test->rawWPM = raw;
  test->wpm = raw - raw * (1.0f - test->accuracy);
}

void reloadTest(Test* test,char* charState){
   test->mode = TYPING;
   test->hasStarted = 0;
   test->currentChar = 0;
   test->currentWord = 0;
   test->currentIndex = 0;
   test->mistakeCount = 0;
   test->finishedAt = 0.0;
   test->isFinished = false;
   test->accuracy = 0.0f;
   test->wpm = 0.0f;
   test->rawWPM = 0.0f;
   test->length = 0;
   getTestWords(test);
   memset(charState,0,test->length * sizeof(char));
}

void handle_normal_mode_keys(Test* test,char c,char* charState){
  switch(c) {
    case 'i' : 
      if(!test->isFinished) {
        test->mode = TYPING;
      } 
      break;
    case 'r' : 
      reloadTest(test,charState);
      break;
    case '?' : 
      isDebugMode = !isDebugMode;
      break;
  }
}

void handleKeys(Test* test,char* charState,Sound sounds[]) {
  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_CAPS_LOCK)) {
    reloadTest(test,charState);
    test->mode = NORMAL;
  }
  int c; 
  if(test->length <= test->currentIndex && test->mode == TYPING){
    test->isFinished = true;
    test->finishedAt = GetTime();
    test->mode = NORMAL;
    return;
  }

  if(IsKeyPressed(KEY_BACKSPACE)){
    if(test->currentIndex){
      test->currentIndex--;
      if(charState[test->currentIndex] == 'F'){
        test->mistakeCount--;
        charState[test->currentIndex] = 0;
      } 
    } 
    if(!test->currentChar){
      if(test->currentWord){
        test->currentWord--;
        // we hit a space
        // TODO
        test->currentChar = strlen(test->words[test->currentWord]) - 1;
      } 
    } else test->currentChar--;
    return;
  }
  if((c = GetCharPressed()) >= 7) {
    if (test->mode == NORMAL) handle_normal_mode_keys(test,c,charState);
    else {
      if(!test->hasStarted) {
        test->hasStarted = 1;
        test->startedAt = GetTime();
      } 
      int randomIndex = rand() % 7;
      PlaySound(sounds[randomIndex]);
      if(test->currentChar > test->length - 1 ) return;
      if(test->words[test->currentWord][test->currentChar] != c){
        if(test->currentChar < strlen(test->words[test->currentWord]) - 1
           || c != ' ' ) {
          charState[test->currentIndex] = 'F'; // for false 
          test->mistakeCount++;
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
    .mode = NORMAL,
    .lang = ENGLISH_5K,
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

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    if(test.isFinished){
      getResult(&test);
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
        if(x_offset + wordLength >= 60) {
          y_offset++;
          x_offset = 0;
        }
        for(int j=0; j <= wordLength ;j++,x_offset++,charIndex++){
          if(test.words[i][j] == ' ') last_space = i;

          if(test.currentIndex == charIndex){
            DrawRectangle(
              LINE_X_POSITION + 19 * x_offset,
              LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset),
              18, 40, WHITE
            );
          }
          DrawTextCodepoint(fontSDF,
            j == wordLength ? ' ' : test.words[i][j],
            (Vector2){LINE_X_POSITION + 19 * x_offset,
            LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset)},
            44,
            (test.currentIndex < charIndex + 1) ? GRAY :
            (charState[charIndex] == 'F' ? RED : WHITE ));
        }
      }

      // time
      const char* display_time = TextFormat("%.1f",GetTime() - test.startedAt);
      DrawTextEx(fontSDF, test.hasStarted ? display_time : "0.0" ,
          (Vector2){LINE_X_POSITION,LINE1_Y - 100}, 44, 4, GRAY);
    } else {
      const char* accuracyText = TextFormat("Accuracy : %.2f %%",test.accuracy * 100);
      const char* rawSpeedText = TextFormat("Raw speed : %.2f WPM",test.rawWPM);
      const char* speedText = TextFormat("Speed : %.2f WPM",test.wpm);
      Vector2 accuracySize = MeasureTextEx(fontSDF, accuracyText, 70, 4);
      Vector2 rawSpeedSize = MeasureTextEx(fontSDF, rawSpeedText, 70, 4);
      Vector2 speedSize = MeasureTextEx(fontSDF, speedText, 100, 4);
      DrawTextEx(fontSDF, speedText ,
       (Vector2){SCREEN_WIDTH / 2 - speedSize.x /2 , SCREEN_HEIGHT/ 2 - speedSize.y * 2},
       100, 4, GREEN);
      DrawTextEx(fontSDF, rawSpeedText ,
       (Vector2){SCREEN_WIDTH / 2 - rawSpeedSize.x /2 , SCREEN_HEIGHT/ 2},
       70, 4, GREEN);
      DrawTextEx(fontSDF, accuracyText ,
       (Vector2){SCREEN_WIDTH / 2 - accuracySize.x /2 , SCREEN_HEIGHT/ 2 + accuracySize.y},
       70, 4, GREEN);
    }
    // Mode
    DrawTextEx(fontSDF, test.mode == NORMAL ? "NORMAL" : "TYPING" , (Vector2){15,7}, 44, 4, GRAY);
    if(isDebugMode) {
      DrawTextEx(fontSDF, TextFormat("Word[%d]",test.currentWord + 1) , (Vector2){200,15}, 36, 4, GRAY);
      DrawTextEx(fontSDF, TextFormat("Char[%d]",test.currentChar + 1) , (Vector2){400,15}, 36, 4, GRAY);
      DrawTextEx(fontSDF, TextFormat("Index[%d]",test.currentIndex) , (Vector2){600,15}, 36, 4, GRAY);
      DrawTextEx(fontSDF, TextFormat("Miskates[%d]",test.mistakeCount) , (Vector2){800,15}, 36, 4, GRAY);
    }
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
