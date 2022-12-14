#include "parser/lexer.h"
#include "parser/token.h"
#include "parser/json.h"
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SERV_TCP_PORT 20000

char keyword[32];
Pair root;

void err_msg(char *msg) {
  perror(msg);
  exit(1);
}

void multiSearchWord(int *thum) {
  if (strcmp("all", keyword) != 0) {
    printf("Thread%d Search word: %s\n", *thum, keyword);
  }
  Table *t = (Table *)root.cdr;
  if (t->pairs[*thum].cdrType == VALUE) {
    printValue(t->pairs[*thum], keyword);
  } else if (t->pairs[*thum].cdrType == LIST) {
    indent++;
    if (strcmp("all", keyword) == 0) {
      printf("{%s: }\n", (char *)t->pairs[*thum].car);
    }
    printList(t->pairs[*thum].cdr, keyword);
    indent--;
  } else if (t->pairs[*thum].cdrType == TABLE) {
    indent++;
    if (strcmp("all", keyword) == 0) {
      printf("{%s: }\n", (char *)t->pairs[*thum].car);
    }
    printTable(t->pairs[*thum].cdr, keyword);
    indent--;
  }
  pthread_exit(thum);
}


int main(int argc, char *argv[]) {
  char *filename = argv[1];
  FILE *fp = fopen(filename, "r");
  struct stat fstat;
  stat(filename, &fstat);
  char *str = malloc(fstat.st_size);
  Token toks[10000];

  int i = 0;
  while (fscanf(fp, "%[^\n] ", str) != EOF) {
    Lexer l = newLexer(str);
    while (1) {
      Token tok = l.NextToken(&l);
      if (tok.Type == QUOTATION) {
        toks[i] = l.NextToken(&l);
        if (l.NextToken(&l).Type != QUOTATION) {
          perror("QUATATION must be pair");
        }
      } else {
        toks[i] = tok;
      }
      if (toks[i].Type == EndOfFile) {
        break;
      }
      i++;
    }
  }
  fclose(fp);

  int j = 0;
  int cnt = 0;
  // Table root;
  root.car = (void*)"root";

  if (toks[j].Type == LBRACE) {
    j++;
    root.cdrType = TABLE;
    readTable(toks, &j, &root);
  } else if (toks[j].Type == LBRACKET) {
    j++;
    root.cdrType = LIST;
    readList(toks, &j, &root);
  } else {
    perror("json should start { or [");
  }

  for (int k = 0; k < i; k++) {
    //printf("{%s: %s}\n", toks[k].TypeStr, toks[k].Literal);
  }

  //searchWord(root, "all");

  int chcnt = 0;

  int n = ((Table*)root.cdr)->size;
  pthread_t threads[n];
  int param[n];
  for (int i = 0; i < n; i++) {
    param[i] = i;
  }
  int *status;

  int port_no;
  int sockid;
  int sockfd;
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int rflag;   // if rflag is zero then send is required else input is required
  int wflag;   // if wflag is zero then reseive is required else output is
               // required
  int endflag; // typed or received ^A
  int iobytes;
  char sch, rch;

  port_no = SERV_TCP_PORT;
  printf("????????????????????????????????? %d ?????????\n", port_no);
  if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    err_msg("srever: can't open datastream socket");
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port_no);

  if (bind(sockid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    err_msg("srever: can't bind local address");
  }
  if (listen(sockid, 5) == -1) {
    err_msg("srever: listen failed");
  }
  if ((sockfd = accept(sockid, (struct sockaddr *)&cli_addr, &cli_len)) < 0) {
    err_msg("server: can't accept");
  }
  printf("???????????????????????????????????????????????????????????????????????????????????????????????????\n"
         "Ctrl-A ????????????????????????????????????\n");

  fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
  // fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  rflag = 1;
  wflag = 0;
  while (1) {
    if (rflag == 1) {
      if (read(fileno(stdin), &sch, 1) == 1) {
        if (sch == '\001') {
          endflag = 0;
          break; // ^A
        }
        rflag = 0;
      }
    }
    if (rflag == 0) {
      if ((iobytes = write(sockfd, &sch, 1)) == 1) {
        rflag = 1;
      } else if (iobytes == -1) {
        if (errno != EAGAIN) {
          perror("write");
          break;
        }
      }
    }
    if (wflag == 0) {
      if ((iobytes = read(sockfd, &rch, 1)) == 1) {
        if (rch == '\001') {
          endflag = 1;
          break; // ^A
        } else if (rch == '\n') {
          //searchWord(root, keyword);
          for (int i = 0; i < n; i++) {
            pthread_create(&threads[i], NULL, (void (*))multiSearchWord, &param[i]);
          }
          for (int i = 0; i < n; i++) {
            pthread_join(threads[i], (void **)&status);
            //printf("Finish thread %d with return value %d\n", i, *status);
          }
          chcnt = 0;
        } else {
          keyword[chcnt] = rch;
          chcnt++;
        }
        wflag = 1;
      } else if (iobytes == -1) {
        if (errno != EAGAIN) {
          perror("read");
          break;
        }
      }
    }
    if (wflag == 1) {
      wflag = 0;
    }
    usleep(10000); // For save CPU usage
  }
  if (endflag == 0) {
    while (write(sockfd, &sch, 1) != 1);
  }
  close(sockfd);
  close(sockid);

  return 0;
}
