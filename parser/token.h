#ifndef _TOKEN_H
#define _TOKEN_H

#include <string.h>

enum TokenType {
  ILLEGAL,
  EndOfFile,
  INT,
  STR,
  COMMA,
  COLON, 
  QUOTATION,
  LBRACE,
  RBRACE,
  LBRACKET,
  RBRACKET,
};

struct Token {
  enum TokenType Type;
  const char *Literal;
  const char *TypeStr;
};
typedef struct Token Token;

Token newToken(enum TokenType Type, const char *Literal) {
  struct Token tok;
  tok.Type = Type;
  tok.Literal = Literal;
  switch (tok.Type) {
    case ILLEGAL:
      tok.TypeStr = "ILLEGAL";
      break;
    case EndOfFile:
      tok.TypeStr = "EndOfFile";
      break;
    case INT:
      tok.TypeStr = "INT";
      break;
    case STR:
      tok.TypeStr = "STR";
      break;
    case COMMA:
      tok.TypeStr = "COMMA";
      break;
    case COLON:
      tok.TypeStr = "COLON";
      break;
    case QUOTATION:
      tok.TypeStr = "QUOTATION";
      break;
    case LBRACE:
      tok.TypeStr = "LBRACE";
      break;
    case RBRACE:
      tok.TypeStr = "RBRACE";
      break;
    case LBRACKET:
      tok.TypeStr = "LBRACKET";
      break;
    case RBRACKET:
      tok.TypeStr = "RBRACKET";
      break;
  } 
  return tok;
}
#endif
