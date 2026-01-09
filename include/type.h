#ifndef TYPE_H
#define TYPE_H
#include <time.h>
#include <stdbool.h>
#include <raylib.h>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LINE_HEIGHT 47
#define TEXT_HEIGHT 26
#define LINE1_Y 400
#define LINE_X_POSITION 380
#define WORDS_COUNT 15
#define SOUND_NUM 7
#define FONTS_NUM 4
#define TOTAL_WORDS_NUM 200
#define ENGLISH_WORDS_COUNT 200
#define ENGLISH_1K_WORDS_COUNT 1000
#define ENGLISH_5K_WORDS_COUNT 4957
#define ONE_WORD_LENGTH 5
#define LONG_PRESS_DELAY 0.3f
#define REPEAT_RATE 0.015f

extern char* ENGLISH_WORDS[];
extern char* ENGLISH_1K_WORDS[];
extern char* ENGLISH_5K_WORDS[];
extern char* audioFileNames[];
extern char* fontNames[];
extern char* fontDisplayNames[];

typedef enum {
  TYPING,
  NORMAL
} Mode;

typedef enum {
  ENGLISH,
  ENGLISH_1K,
  ENGLISH_5K,
} Lang;

typedef struct _font {
  char* name;
  int fileSize;
  Font font;
} FontData;

typedef struct test_ {
  int length;
  int currentIndex;
  int mistakeCount;
  int numWords;
  double startedAt;
  double finishedAt;
  bool hasStarted;
  bool gotResult;
  bool isFinished;
  bool isNewRecord;
  bool isMute;
  FontData* fonts;
  int currentFont;
  float record;
  float accuracy;
  float rawWPM;
  float wpm;
  char* text;
  Mode mode;
  Lang lang;
} Test;

void getTestContent(Test* test);

int loadFontSDF( char* path, int* fileSize, int size, Font* font);

void loadAllFonts(FontData* fonts);

#endif

