#ifndef _JSON_H
#define _JSON_H

#include "token.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define SIZE 64

typedef enum {
  TABLE,
  LIST,
  VALUE,
} pType;

typedef struct {
  pType cdrType;
  void *car;
  void *cdr;
} Pair;

typedef struct {
  int size;
  int id;
  Pair pairs[SIZE];
} List;

typedef struct {
  int size;
  int id;
  Pair pairs[SIZE];
} Table;

Table tables[32];
int table_id = -1;
List lists[32];
int list_id = -1;

void readKey(Token *toks, int *j, Pair *parent);
void readValue(Token *toks, int *j, Pair *parent);
void readList(Token *toks, int *j, Pair *parent);
void readTable(Token *toks, int *j, Pair *parent);

void readKey(Token *toks, int *j, Pair *parent) {
  if (toks[*j].Type == STR || toks[*j].Type == INT) {
    parent->car = (void *)toks[*j].Literal;
    (*j)++;
    if (toks[*j].Type == COLON) {
      (*j)++;
    } else {
      perror("Here should be COLON\n");
    }
  }
}

void readValue(Token *toks, int *j, Pair *parent) {
  if (toks[*j].Type == STR || toks[*j].Type == INT) {
    parent->cdr = (void *)toks[*j].Literal;
    (*j)++;
    if (toks[*j].Type == COMMA) {
      (*j)++;
    } else {
      // perror("Here should be COMMA\n");
    }
  }
}

void readList(Token *toks, int *j, Pair *parent) {
  list_id++;
  int list_cnt = 0;
  lists[list_id].size = 0;
  lists[list_id].id = list_id;
  parent->cdr = &lists[list_id];
  List *l = (List*)parent->cdr;
  while (1) {
    l->pairs[list_cnt].car = parent->car;
    if (toks[*j].Type == STR || toks[*j].Type == INT) {
      readValue(toks, j, &l->pairs[list_cnt]);
      l->pairs[list_cnt].cdrType = VALUE;
    } else if (toks[*j].Type == LBRACKET) {
      (*j)++;
      readList(toks, j, &l->pairs[list_cnt]);
      l->pairs[list_cnt].cdrType = LIST;
    } else if (toks[*j].Type == LBRACE) {
      (*j)++;
      readTable(toks, j, &l->pairs[list_cnt]);
      l->pairs[list_cnt].cdrType = TABLE;
    } else if (toks[*j].Type == RBRACKET) {
      (*j)++;
      if (toks[*j].Type == COMMA) {
        (*j)++;
      }
      break;
    }
    list_cnt++;
    l->size++;
  }
}

void readTable(Token *toks, int *j, Pair *parent) {
  table_id++;
  int table_cnt = 0;
  tables[table_id].size = 0;
  tables[table_id].id = table_id;
  parent->cdr = &tables[table_id];
  Table *t = (Table*)parent->cdr;
  while (1) {
    // key
    readKey(toks, j, &t->pairs[table_cnt]);
    // val
    if (toks[*j].Type == STR || toks[*j].Type == INT) {
      readValue(toks, j, &t->pairs[table_cnt]);
      t->pairs[table_cnt].cdrType = VALUE;
    } else if (toks[*j].Type == LBRACKET) {
      (*j)++;
      readList(toks, j, &t->pairs[table_cnt]);
      t->pairs[table_cnt].cdrType = LIST;
    } else if (toks[*j].Type == LBRACE) {
      (*j)++;
      readTable(toks, j, &t->pairs[table_cnt]);
      t->pairs[table_cnt].cdrType = TABLE;
    } else if (toks[*j].Type == RBRACE) {
      (*j)++;
      if (toks[*j].Type == COMMA) {
        (*j)++;
      }
      break;
    }
    table_cnt++;
    t->size++;
  }
}

void printValue(Pair p, char *keyword);
void printList(void *v, char *keyword);
void printTable(void *v, char *keyword);
int indent = 0;

void printValue(Pair p, char *keyword) {
  if (strcmp("all", keyword) == 0) {
    for (int i = 0; i < indent; i++) {
      printf("  ");
    }
  }
  if (strcmp((char *)p.car, keyword) == 0 || strcmp("all", keyword) == 0) {
    printf("{%s: %s}\n", (char *)p.car, (char *)p.cdr);
  }
}

void printList(void *v, char *keyword) {
  List *l = (List *)v;
  for (int k = 0; k < l->size; k++) {
    if (l->pairs[k].cdrType == VALUE) {
      printValue(l->pairs[k], keyword);
    } else if (l->pairs[k].cdrType == LIST) {
      indent++;
      printList(l->pairs[k].cdr, keyword);
      indent--;
    } else if (l->pairs[k].cdrType == TABLE) {
      indent++;
      printTable(l->pairs[k].cdr, keyword);
      indent--;
    }
  }
}

void printTable(void *v, char *keyword) {
  Table *t = (Table *)v;
  for (int k = 0; k < t->size; k++) {
    if (t->pairs[k].cdrType == VALUE) {
      printValue(t->pairs[k], keyword);
    } else if (t->pairs[k].cdrType == LIST) {
      indent++;
      if (strcmp("all", keyword) == 0) {
        printf("{%s: }\n", (char *)t->pairs[k].car);
      }
      printList(t->pairs[k].cdr, keyword);
      indent--;
    } else if (t->pairs[k].cdrType == TABLE) {
      indent++;
      if (strcmp("all", keyword) == 0) {
        printf("{%s: }\n", (char *)t->pairs[k].car);
      }
      printTable(t->pairs[k].cdr, keyword);
      indent--;
    }
  }
}

void searchWord(Pair root, char *keyword) {
  printf("Search word: %s\n", keyword);
  printTable(root.cdr, keyword);
}

#endif
