#ifndef _LEXER_H
#define _LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

int isLetter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || (ch == '_') || ('0' <= ch && ch <= '9') || (ch == ' ') || (ch == '-');
}

int isDigit(char ch) {
  return ('0' <= ch && ch <= '9') || (ch == '.') || (ch == '-');
}

struct Lexer {
  char* input;
  int position;
  int readPosition;
  char ch;
  void (*readChar)(struct Lexer *);
  Token (*NextToken)(struct Lexer *);
  void (*skipWhitespace)(struct Lexer *);
  char* (*readString)(struct Lexer *);
  char* (*readNumber)(struct Lexer *);
};

typedef struct Lexer Lexer;

void readChar(Lexer *l) {
  if (l->readPosition >= strlen(l->input)) {
    l->ch = 0;
  } else {
    l->ch = l->input[l->readPosition];
  }
  l->position = l->readPosition;
  l->readPosition += 1;
}

Token NextToken(Lexer *l) {
  Token tok;
  l->skipWhitespace(l);
  switch (l->ch) {
    case ',':
      tok = newToken(COMMA, ",");
      break;
    case ':':
      tok = newToken(COLON, ":");
      break;
    case '"':
      tok = newToken(QUOTATION, "\"");
      break;
    case '{':
      tok = newToken(LBRACE, "{");
      break;
    case '}':
      tok = newToken(RBRACE, "}");
      break;
    case '[':
      tok = newToken(LBRACKET, "[");
      break;
    case ']':
      tok = newToken(RBRACKET, "]");
      break;
    case 0:
      tok = newToken(EndOfFile, " ");
      break;
    default:
      /*
      if (isDigit(l->ch)) {
        tok = newToken(INT, l->readNumber(l));
        return tok;
      } 
      */
      if (isLetter(l->ch)) {
        char* Literal = NULL;
        Literal = l->readString(l);
        tok = newToken(STR, Literal);
        return tok;
      } else {
        tok = newToken(ILLEGAL, &(l->ch));
      }
  }

  l->readChar(l);
  return tok;
}

void skipWhitespace(Lexer *l) {
  while (l->ch == ' ' || l->ch == '\t' || l->ch == '\n' || l->ch == '\r') {
    l->readChar(l);
  }
}

char* readString(Lexer *l) {
  char s[64];
  int i;
  for (i = 0; isLetter(l->ch); i++) {
    s[i] = l->ch;
    l->readChar(l);
  }
  s[i] = '\0';

  char* cp = NULL;  
  cp = (char*)malloc(sizeof(char) * 64);
  strcpy(cp, s);

  return cp;
}

char* readNumber(Lexer *l) {
  char s[32];
  int i;
  for (i = 0; isDigit(l->ch); i++) {
    s[i] = l->ch;
    l->readChar(l);
  }
  s[i] = '\0';

  char* cp = NULL;  
  cp = (char*)malloc(sizeof(char) * 32);
  strcpy(cp, s);

  return cp;
}

Lexer newLexer(char *input) {
  Lexer l;
  l.input = input;
  l.position = 0;
  l.readPosition = 0;
  l.readChar = readChar;
  l.NextToken = NextToken;
  l.skipWhitespace = skipWhitespace;
  l.readString = readString;
  l.readNumber = readNumber;
  l.readChar(&l);
  return l;
}
#endif
