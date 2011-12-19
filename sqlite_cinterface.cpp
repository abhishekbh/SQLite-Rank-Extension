#include <stdio.h>
#include <sqlite3.h>
#include "rank.c"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  int i;

  for (i=0; i<argc; i++) {
    printf("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    //printf("hello\n");
  }

  printf("\n");
  return 0;
}

int main(int argc, char **argv) {
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  if ( argc!=3 ) {
    fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
    return(1);
  }

  rc = sqlite3_open(argv[1], &db);
  if ( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  if (sqlite3_create_function(db, "rank", 3, SQLITE_UTF8, NULL, &rank, NULL, NULL) != 0)
    fprintf(stderr, "Problem with rank\n");

  rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);

  if ( rc!=SQLITE_OK ) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);

  return 0;
}

