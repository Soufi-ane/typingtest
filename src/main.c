#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "type.h"

// bool isDebugMode = false;
double longPressTime = 0.0f;
bool hasLoadFonts = false;

const char* getLanguageName(Lang l) {
  switch(l){
    case ENGLISH:
      return "English";
    case ENGLISH_1K:
      return "English 1K";
    case ENGLISH_5K:
      return "English 5K";
    default : 
      return "English";
  }
}

void getResult(Test* test,FILE** resultFile,float last_record){
  if(test->mistakeCount < 1 || test->length < 1) test->accuracy = 1.0f;
  else test->accuracy =  (float) (test->length - test->mistakeCount) / test->length;
  float wordsTyped = (float) test->length / 5; 
  float typingTime = test->finishedAt - test->startedAt;
  float raw =  wordsTyped * (60 / typingTime);
  test->rawWPM = raw;
  test->wpm = raw - raw * (1.0f - test->accuracy);
  test->isNewRecord = test->wpm > last_record;
  test->record = test->isNewRecord ? test->wpm : last_record;

  if(*resultFile != NULL){
    printf("Result file found\n");
    fprintf(*resultFile,"%.2f\n",test->wpm);
    fclose(*resultFile);
  }else {
    printf("Coudn't open the file\n");
  }
}

void reloadTest(Test* test,char* charState){
   test->mode = TYPING;
   test->hasStarted = 0;
   test->currentIndex = 0;
   test->mistakeCount = 0;
   test->finishedAt = 0.0;
   test->isFinished = false;
   test->gotResult = false;
   test->accuracy = 0.0f;
   test->wpm = 0.0f;
   test->rawWPM = 0.0f;
   test->length = 0;
   test->isNewRecord = false;
   getTestContent(test);
   memset(charState,0,sizeof(char) * 1024 * 100);
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
    // case '?' : 
      // isDebugMode = !isDebugMode;
      // break;
    case 'm' : 
      test->isMute = !test->isMute;
      break;
    case 'l' : 
      if(test->lang == ENGLISH) test->lang = ENGLISH_1K;
      else if(test->lang == ENGLISH_1K) test->lang = ENGLISH_5K;
      else test->lang = ENGLISH;
      break;
    case '+' : 
      if(test->numWords < 50) test->numWords++;
      break;
    case '-' : 
      if(test->numWords > 1) test->numWords--;
      break;
    case 'f' :
      if(hasLoadFonts) {
        if(test->currentFont > FONTS_NUM - 2) test->currentFont = 0; 
        else test->currentFont++;
      }
      break;
  }
}
void handleBackSpace(Test* test,char* charState){
  if(test->currentIndex){
    test->currentIndex--;
    if(charState[test->currentIndex] == 'F'){
      test->mistakeCount--;
      charState[test->currentIndex] = 0;
    } 
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
    longPressTime = GetTime();
    handleBackSpace(test,charState);
    return;
  }

  if (IsKeyDown(KEY_BACKSPACE)) {
    double now = GetTime();
    if(now - longPressTime > LONG_PRESS_DELAY){
      handleBackSpace(test,charState);
      longPressTime = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
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
      if(!test->isMute) PlaySound(sounds[randomIndex]);
      if(test->currentIndex > test->length - 1 ) return;
      if(test->text[test->currentIndex] != c){
        charState[test->currentIndex] = 'F'; // for false 
        test->mistakeCount++;
      } 
      test->currentIndex++;
    }
  }
}

int compare(const void* a, const void* b){
  return (int) *(float*) b - *(float*) a;
}
void sortScores(float* scores,int* num_scores){
  qsort(scores,*num_scores,sizeof(float),compare);
}


float* readScores(FILE** resultFile,const char* name,int* num_scores){
  printf("reading previous scores\n");
  char line[100];
  int num_lines = 0;
  *resultFile = fopen(name,"a+");
  float* scores = NULL;
  if(*resultFile != NULL) {
    while(fgets(line,sizeof(line),*resultFile) != NULL) num_lines++;
    rewind(*resultFile);
    scores = malloc(sizeof(float) * num_lines);
    int i = 0;
    while(fgets(line,sizeof(line),*resultFile) != NULL){
      float score;
      if(sscanf(line, "%f",&score) == 1){
        if(score > 0.0f) scores[i++] = score;
      } 
    }
    *num_scores = i;
    sortScores(scores,num_scores);
  }
  return scores;
}
void UpdateDrawFrame();
const char *home;
char *filePath;
FILE* resultFile;
float highest_score = 0.0f;
float * scores;
int num_scores = 0;

Test test = {
  .mode = NORMAL,
  .lang = ENGLISH,
  .numWords = WORDS_COUNT,
};
FontData fonts[FONTS_NUM];
Sound allSounds[SOUND_NUM];
char path[1024];
Font fontSDF = {0};
char* charState;
Image soundImage;
Image noSoundImage;
Texture2D soundTexture;
Texture2D noSoundTexture;
RenderTexture2D target;

int main() {

#ifdef RELEASE
  home = getenv("HOME");
  if (home == NULL) {
    fprintf(stderr, "Error: Could not find HOME environment variable\n");
    return 1;
  }
  snprintf(filePath, sizeof(filePath), "%s/.local/share/typingtest/typingtest.log", home);
#else
  const char* filePath = "results";
#endif
  printf("logs file : %s\n",filePath);
  srand(time(NULL));

  getTestContent(&test);


  scores = readScores(&resultFile,filePath,&num_scores);
  if(scores != NULL && num_scores){
    highest_score = test.record = scores[0];
  }

  // int fileSize = 0;
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Type");
  target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
  // Font fontSDF = {0};
  fonts[0].name = fontDisplayNames[0];
  fonts[0].fileSize = 0;
#ifdef RELEASE
  sprintf(path,"/usr/local/share/typingtest/fonts/%s",fontNames[0]);
#else
  sprintf(path,"assets/fonts/%s",fontNames[0]);
#endif
  int fontError = loadFontSDF(path, &fonts[0].fileSize, 128, &fontSDF);
  if(fontError) printf("failed to load font %s\n",path);
  fonts[0].font = fontSDF;
  test.fonts = fonts;

  charState = calloc(1024 * 100,sizeof(char));

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  SetExitKey(KEY_NULL);

  InitAudioDevice();
  for(int i = 0 ; i < SOUND_NUM /* number of files */; i++){
    Sound s;
#ifdef RELEASE
    s = LoadSound(TextFormat("/usr/local/share/typingtest/audios/%s",audioFileNames[i]));
#else
    s = LoadSound(TextFormat("assets/audios/%s",audioFileNames[i]));
#endif
    allSounds[i] = s;
  }

#ifdef RELEASE
  soundImage = LoadImage("/usr/local/share/typingtest/icons/sound.png"); 
  noSoundImage = LoadImage("/usr/local/share/typingtest/icons/no-sound.png"); 
#else
  soundImage = LoadImage("assets/icons/sound.png"); 
  noSoundImage = LoadImage("assets/icons/no-sound.png"); 
#endif

  soundTexture = LoadTextureFromImage(soundImage);
  noSoundTexture = LoadTextureFromImage(noSoundImage);
  UnloadImage(soundImage);
  UnloadImage(noSoundImage);

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

  UnloadRenderTexture(target);
  for(int i=0; i < FONTS_NUM; i++) UnloadFont(fonts[i].font);
  for(int i=0; i < SOUND_NUM; i++) UnloadSound(allSounds[i]);
  CloseWindow();
  return 0;
}


void UpdateDrawFrame(){
  float scale = MIN((float)GetScreenWidth()/SCREEN_WIDTH, (float)GetScreenHeight()/SCREEN_HEIGHT);
    if(!hasLoadFonts){
      loadAllFonts(test.fonts);
      hasLoadFonts = true;
    }
    if(test.isFinished && !test.gotResult){
      free(scores);
      scores = readScores(&resultFile,filePath,&num_scores);
      if(scores != NULL && num_scores) highest_score = scores[0];
      getResult(&test,&resultFile,highest_score);
      test.gotResult = true;
    } 
    if (test.mode != TYPING) {
      SetExitKey(KEY_Q);
    } else if (test.mode == TYPING) {
      SetExitKey(KEY_NULL);
    }
    handleKeys(&test,charState,allSounds);
    BeginTextureMode(target);
    ClearBackground(BLACK);
    int y_offset = 0;
    if(!test.isFinished){
      for(int i = 0,x_offset=0; i < test.length ;i++,x_offset++){
        char* next_space = strchr(test.text+i,' ');
        if(x_offset + next_space - test.text - i >= 60) {  // how far the next space is
          y_offset++;
          x_offset = 0;
        }
          if(test.currentIndex == i){
            DrawRectangle(
              LINE_X_POSITION + 21 * x_offset,
              LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset),
              18, 40, WHITE
            );
          }
          DrawTextCodepoint(fonts[test.currentFont].font,
             test.text[i],
            (Vector2){LINE_X_POSITION + 21 * x_offset,
            LINE1_Y - TEXT_HEIGHT + (LINE_HEIGHT * y_offset)},
            44, (test.currentIndex < i + 1) ? GRAY :
            (charState[i] == 'F' ? RED : WHITE ));
      }

      // time
      const char* display_time = TextFormat("%.1f",GetTime() - test.startedAt);
      DrawTextEx(fonts[test.currentFont].font, test.hasStarted ? display_time : "0.0" ,
          (Vector2){LINE_X_POSITION,LINE1_Y - 100}, 44, 0, GRAY);
      // language
      DrawTextEx(fonts[test.currentFont].font, getLanguageName(test.lang) ,
          (Vector2){LINE_X_POSITION,LINE1_Y - 200}, 36, 0, GRAY);
      // number of words
      DrawTextEx(fonts[test.currentFont].font, TextFormat("%d words",test.numWords) ,
          (Vector2){LINE_X_POSITION + 300,LINE1_Y - 200}, 36, 0, GRAY);

      // font
      DrawTextEx(fonts[test.currentFont].font, fontDisplayNames[test.currentFont] ,
          (Vector2){LINE_X_POSITION + 600,LINE1_Y - 200}, 36, 0, GRAY);

    } else {
      const char* accuracyText = TextFormat("Accuracy : %.2f %%",test.accuracy * 100);
      const char* rawSpeedText = TextFormat("Raw speed : %.2f WPM",test.rawWPM);
      const char* speedText = TextFormat("Speed : %.2f WPM",test.wpm);
      Vector2 accuracySize = MeasureTextEx(fonts[test.currentFont].font, accuracyText, 60, 0);
      Vector2 rawSpeedSize = MeasureTextEx(fonts[test.currentFont].font, rawSpeedText, 60, 0);
      Vector2 speedSize = MeasureTextEx(fonts[test.currentFont].font, speedText, 60, 0);
      DrawTextEx(fonts[test.currentFont].font, speedText ,
       (Vector2){SCREEN_WIDTH / 2 - speedSize.x /2 , SCREEN_HEIGHT/ 2 - speedSize.y * 2},
       60, 0, GRAY);
      DrawTextEx(fonts[test.currentFont].font, rawSpeedText ,
       (Vector2){SCREEN_WIDTH / 2 - rawSpeedSize.x /2 , SCREEN_HEIGHT/ 2},
       60, 0, GRAY);
      DrawTextEx(fonts[test.currentFont].font, accuracyText ,
       (Vector2){SCREEN_WIDTH / 2 - accuracySize.x /2 , SCREEN_HEIGHT/ 2 + accuracySize.y},
       60, 0, GRAY);
      if(test.isNewRecord){
        DrawTextEx(fonts[test.currentFont].font, "New Record !!",
         (Vector2){SCREEN_WIDTH / 10, SCREEN_HEIGHT/ 6},
         70, 0, GRAY);
      }
    }
    // Mode
    DrawTextEx(fonts[test.currentFont].font, test.mode == NORMAL ? "Normal" : "Typing" , (Vector2){15,20}, 44, 0, GRAY);
    // if(isDebugMode) {
      // DrawTextEx(fonts[0].font, TextFormat("Index[%d]",test.currentIndex) , (Vector2){300,15}, 36, 4, GRAY);
      // DrawTextEx(fonts[0].font, TextFormat("Miskates[%d]",test.mistakeCount) , (Vector2){600,15}, 36, 4, GRAY);
    // }
    //
    const char* recordText = TextFormat("Highest wpm: %.2f",test.record);
    Vector2 recordSize = MeasureTextEx(fonts[test.currentFont].font, recordText, 36, 0);
    DrawTextEx(fonts[test.currentFont].font, recordText, (Vector2){SCREEN_WIDTH/2 - recordSize.x / 2,20}, 36, 0, GRAY);

    // icons
    if(!test.isMute){
      DrawTextureEx(soundTexture, (Vector2){ SCREEN_WIDTH - 100, 40 }, 0.0f, 1.0f, WHITE);
    } else {
      DrawTextureEx(noSoundTexture, (Vector2){ SCREEN_WIDTH - 100, 40 }, 0.0f, 1.0f, WHITE);
    }
    EndTextureMode();
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture, 
      (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
      (Rectangle){ (GetScreenWidth() - ((float)SCREEN_WIDTH * scale)) * 0.5f, 
                   (GetScreenHeight() - ((float)SCREEN_HEIGHT * scale)) * 0.5f, 
                   (float)SCREEN_WIDTH * scale, (float)SCREEN_HEIGHT * scale }, 
      (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}
