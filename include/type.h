#ifndef TYPE_H
#define TYPE_H

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LINE_HEIGHT 47
#define TEXT_HEIGHT 26
#define LINE1_Y 400
#define LINE_X_POSITION 380
#define WORDS_COUNT 120
#define SOUND_NUM 9
#define TOTAL_WORDS_NUM 200

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
  int isFinished;
  float accuracy;
  char* words[TOTAL_WORDS_NUM];
  Mode mode;
} Test;

void getTestWords(Test* test);

#endif

