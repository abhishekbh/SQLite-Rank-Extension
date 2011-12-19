#include <stdio.h>
#include <sqlite3.h>
//#include "rank.c"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  int i;

  for (i=0; i<argc; i++) {
    printf("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    //printf("hello\n");
  }

  printf("\n");
  return 0;
}

static void rank(sqlite3_context *pCtx, int nVal, sqlite3_value **apVal) {
  int *aMatchinfo;                /* Return value of matchinfo() */
  int nCol;                       /* Number of columns in the table */
  int nPhrase;                    /* Number of phrases in the query */
  int iPhrase;                    /* Current phrase */
  double score = 0.0;             /* Value to return */

  //assert( sizeof(int)==4 );

  /* Check that the number of arguments passed to this function is correct.
 *   ** If not, jump to wrong_number_args. Set aMatchinfo to point to the array
 *     ** of unsigned integer values returned by FTS function matchinfo. Set
 *       ** nPhrase to contain the number of reportable phrases in the users full-text
 *         ** query, and nCol to the number of columns in the table.
 *           */
  //if ( nVal<1 ) goto wrong_number_args;
  //aMatchinfo = (unsigned int *)sqlite3_value_blob(apVal[0]);
  aMatchinfo = (int *)sqlite3_value_blob(apVal[0]);

  nPhrase = aMatchinfo[0];
  nCol = aMatchinfo[1];
  //if( nVal!=(1+nCol) ) goto wrong_number_args;

  /* Iterate through each phrase in the users query. */
  for(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    int iCol;                     /* Current column */

    /* Now iterate through each column in the users query. For each column,
 *     ** increment the relevancy score by:
 *         **
 *             **   (<hit count> / <global hit count>) * <column weight>
 *                 **
 *                     ** aPhraseinfo[] points to the start of the data for phrase iPhrase. So
 *                         ** the hit count and global hit counts for each column are found in 
 *                             ** aPhraseinfo[iCol*3] and aPhraseinfo[iCol*3+1], respectively.
 *                                 */
    int *aPhraseinfo = &aMatchinfo[2 + iPhrase*nCol*3];
    for(iCol=0; iCol<nCol; iCol++){
      int nHitCount = aPhraseinfo[3*iCol];
      int nGlobalHitCount = aPhraseinfo[3*iCol+1];
      double weight = sqlite3_value_double(apVal[iCol+1]);
      if( nHitCount>0 ){
        score += ((double)nHitCount / (double)nGlobalHitCount) * weight;
      }
    }
  }

  sqlite3_result_double(pCtx, score);
  return;
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

