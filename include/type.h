#ifndef TYPE_H
#define TYPE_H
#include <time.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LINE_HEIGHT 47
#define TEXT_HEIGHT 26
#define LINE1_Y 400
#define LINE_X_POSITION 380
#define WORDS_COUNT 20
#define SOUND_NUM 7
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

typedef enum {
  TYPING,
  NORMAL
} Mode;

typedef enum {
  ENGLISH,
  ENGLISH_1K,
  ENGLISH_5K,
} Lang;

typedef struct test_ {
  int length;
  int currentIndex;
  int mistakeCount;
  double startedAt;
  bool hasStarted;
  double finishedAt;
  bool isFinished;
  float accuracy;
  float rawWPM;
  float wpm;
  char* text;
  Mode mode;
  Lang lang;
} Test;

void getTestContent(Test* test);

#endif

