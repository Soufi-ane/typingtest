#ifndef TYPE_H
#define TYPE_H
#include <time.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LINE_HEIGHT 47
#define TEXT_HEIGHT 26
#define LINE1_Y 400
#define LINE_X_POSITION 380
#define WORDS_COUNT 5
#define SOUND_NUM 7
#define TOTAL_WORDS_NUM 200
#define ONE_WORD_LENGTH 5

extern char* ENGLISH_WORDS[];
extern char* audioFileNames[];

typedef enum {
  TYPING,
  NORMAL
} Mode;

typedef struct test_ {
  int length;
  int currentChar;
  int currentWord;
  int currentIndex;
  int mistakeCount;
  double startedAt;
  double finishedAt;
  int isFinished;
  float accuracy;
  float rawWPM;
  float wpm;
  char* words[TOTAL_WORDS_NUM];
  Mode mode;
} Test;

void getTestWords(Test* test);

#endif

