#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

static void rank(sqlite3_context *pCtx, int nVal, sqlite3_value **apVal) {
  int *aMatchinfo;
  int nCol;
  int nPhrase;
  int iPhrase;
  double score = 0.0;

  aMatchinfo = (int *)sqlite3_value_blob(apVal[0]);

  nPhrase = aMatchinfo[0];
  nCol = aMatchinfo[1];

  for(iPhrase=0; iPhrase<nPhrase; iPhrase++){
    int iCol;

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

/* SQLite invokes this routine once when it loads the extension.
** Create new functions, collating sequences, and virtual table
** modules here.  This is usually the only exported symbol in
** the shared library.
*/
int sqlite3_extension_init(
  sqlite3 *db,
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  sqlite3_create_function(db, "rank", 3, SQLITE_ANY, 0, &rank, 0, 0);
  return 0;
}
