#include <stdio.h>
#include <stdlib.h>
#include "demo.h"

#define MAX_DEMOS   1000
#define LINE_LENGTH  256

static void exit_with_help(char *pname, int ret)
{
  printf("Usage:\n"
         "%s <script>\n"
         "\n"
         "script is a text file with one demo per line describing\n"
         "the intended playback order of your demos.\n",
         pname);
  exit(ret);
}

int main(int argc, char *argv[])
{
  FILE *script;
  char *dname[MAX_DEMOS];
  int dcount;
  int fret;
  int i;
  int do_validate = 1;

  if (argc < 2) {
    exit_with_help(argv[0], -1);
  }

  script = fopen(argv[1], "r");
  if (script == NULL) {
    fprintf(stderr, "could not open script %s\n", argv[1]);
    exit(-2);
  }

  memset(dname, 0, sizeof(dname));
  dcount = 0;

  // find all demo files that are part of the chain
  while (!feof(script)) {
    char line[LINE_LENGTH];
    fret = fscanf(script, "%s.dem\n", line);
    if (fret == 1) {
      dname[dcount++] = strdup(line);
    }
  }

  printf("%d demos found:\n", dcount);
  for (i = 0; i < dcount; i++) {
    printf(" %s\n", dname[i]);
  }

  // validate all demos before we start modifying
  if (do_validate) {
    demo *d;
    flagfield rflgs[] = { {READFLAG_FILENAME, NULL},
                          {READFLAG_END, READFLAG_END} };
    dret_t dret;

    printf("validating demos...");
    for (i = 0; i < dcount; i++) {
      rflgs[0].value = dname[i];
      dret = demo_read(rflgs, &d);
      if (dret != DEMO_OK) {
        printf("\n");
        fprintf(stderr, "could not open %s:\n", dname[i]);
        fprintf(stderr, demo_error(dret));
        return -2;
      }
      demo_free(d);
    }
    printf("ok\n");
  }

  // now start modifying
  if (1) {
    demo *d;
    block *b;
    message *m;
    dret_t dret;
    flagfield rflgs[] = { {READFLAG_FILENAME, NULL},
                          {READFLAG_END, READFLAG_END} };
    flagfield wflgs[] = { {WRITEFLAG_FILENAME, NULL},
                          {WRITEFLAG_REPLACE, NULL},
                          {WRITEFLAG_END, WRITEFLAG_END} };

    for (i = 0; i < (dcount - 1); i++) {
      rflgs[0].value = dname[i];
      dret = demo_read(rflgs, &d);
      for (b = d->blocks; b && b->next; ) {
        for (m = b->messages; m; m = m->next) {
          if (m->type == DISCONNECT) {
            break;
          }
        }
        b = b->next;
      }
      if (b) {
        message *play = calloc(sizeof(message), 1);
        play->type = STUFFTEXT;
        play->data = malloc(strlen(dname[i + 1]) + strlen("playdemo \n"));
        sprintf(play->data, "playdemo %s\n", dname[i + 1]);
        play->size = strlen(play->data) + 1;
        play->next = b->messages;
        b->messages->prev = play;
        b->messages = play;
        b->length += play->size + 1;
      }
      else {
        fprintf(stderr, "sorry, %s is a broken demo\n",
                dname[i]);
      }
      wflgs[0].value = dname[i];
      dret = demo_write(wflgs, d);
      if (dret != DEMO_OK) {
        fprintf(stderr, "sorry, could not write %s:\n", dname[i]);
        fprintf(stderr, demo_error(dret));
        return -2;
      }
      printf("chained %s -> %s\n", dname[i], dname[i + 1]);
      demo_free(d);
    }
  }

  return 0;
}
