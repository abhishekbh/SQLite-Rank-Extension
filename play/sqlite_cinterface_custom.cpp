/*
  Very simple C++ program.

   Compile:
    g++ -o myfuncpp myfuncpp.cc  -Wall -W -O2 -Wl,-R/usr/local/lib -lsqlite3

   Note sqlite3 shared library, by default, installs in /usr/local/lib. 
   The compile command above will directly link the full path of 
   this library into this program.


*/
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <sstream>

#include <sqlite3.h>
#include <assert.h>
#include "extract.h"


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  NotUsed=0;
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

/*
** Implementation of the sign() function
*/
static void msignFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  assert( argc==1 );
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      long long int iVal = sqlite3_value_int64(argv[0]);
      iVal = ( iVal > 0) ? 1 : ( iVal < 0 ) ? -1 : 0;
      sqlite3_result_int64(context, iVal);
      break;
    }
    case SQLITE_NULL: {
      sqlite3_result_null(context);
      break;
    }
    default: {
      double rVal = sqlite3_value_double(argv[0]);
      rVal = ( rVal > 0) ? 1 : ( rVal < 0 ) ? -1 : 0;
      sqlite3_result_double(context, rVal);
      break;
    }
  }
}




static void IFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  assert( argc==2 );
  char *buf=NULL;
  std::stringstream s;
  std::string ts;
  extract e;
  e.setdelims(", ");
  s.str("");

  s << sqlite3_value_text(argv[0]);
  e.strip(s.str());

  s.str(e.I(sqlite3_value_int64(argv[1])));
  buf = (char *) malloc (sizeof(char)*(s.str().size()+2));
  if (buf == NULL)
    fprintf(stderr,"malloc error in SNFunc, buf\n");
  snprintf(buf,s.str().size()+1,"%s",s.str().c_str());
  sqlite3_result_text(context,buf,s.str().size()+1,free );

}


static void FFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  assert( argc==1 );
  char *buf=NULL;
  std::stringstream s;
  std::string ts;
  extract e;
  e.setdelims(", ");
  s.str("");

  s << sqlite3_value_text(argv[0]);
  e.strip(s.str());

  s.str(e.F());
  buf = (char *) malloc (sizeof(char)*(s.str().size()+2));
  if (buf == NULL)
    fprintf(stderr,"malloc error in SNFunc, buf\n");
  snprintf(buf,s.str().size()+1,"%s",s.str().c_str());
  sqlite3_result_text(context,buf,s.str().size()+1,free );

}



static void SFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  std::stringstream s;
  std::string d;
  double sum=0;
  char *buf=NULL;
  int i;

  s.str("");

  s << "(";    
  d="";
  for(i=0; i < argc; i++)
    {
  switch( sqlite3_value_type(argv[i]) ){
    case SQLITE_INTEGER: {
      sum+=(double) sqlite3_value_int64(argv[i]);
      s << d << sum;
      d=",";
      break;
    }
    case SQLITE_NULL: {
      s << d << "()";
      d=",";
      break;
    }
    default: {
      sum+=sqlite3_value_int64(argv[i]);
      s << d <<  sum;
      d=",";
      break;
     }
    }

    }

  s << ")";
  buf = (char *) malloc (sizeof(char)*(s.str().size()+2));
  if (buf == NULL)
    fprintf(stderr,"malloc error in SNFunc, buf\n");
  snprintf(buf,s.str().size()+1,"%s",s.str().c_str());
  sqlite3_result_text(context,buf,s.str().size()+1,free );

}                                                                                          





static void TFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  std::stringstream s;
  std::string d;
  char *buf=NULL;
  long i;
  long j;
  double di;
  double dj;
  double dk;
  s.str("");

  if (argc == 1)
    {
      s << "(";
      d = "";
     j= (int) sqlite3_value_int64(argv[0]);
     for(i=0; i < j; ++i)
       {
	 s << d << i;
       d=",";
       }

    }


  if (argc == 2)
    {
      s << "(";
      d = "";
      if ( sqlite3_value_int64(argv[0]) < sqlite3_value_int64(argv[1]))
	{
                i=  sqlite3_value_int64(argv[0]);
                j=  sqlite3_value_int64(argv[1]);
	} else 	  {
                i=  sqlite3_value_int64(argv[1]);
                j=  sqlite3_value_int64(argv[0]);
	 }
     for( ; i <= j; ++i)
       {
       s << d << i;
       d=",";
       }

    }


  if (argc == 3)
    {
      s << "(";
      d = "";

      di=  sqlite3_value_double(argv[0]);
      dj=  sqlite3_value_double(argv[1]);
      dk=  sqlite3_value_double(argv[2]);       

      if ( di < dj && dk > 0 )
	for( ; di <= dj; di+=dk)
        {
         s << d << di;
         d=",";
        }

      if ( di > dj && dk < 0 )
	for( ; di >= dj; di+=dk)
        {
         s << d << di;
         d=",";
        }

      if (  dk == 0 )
        {
         s << di;
        }

    }
  


  s << ")";
  buf = (char *) malloc (sizeof(char)*(s.str().size()+2));
  if (buf == NULL)
    fprintf(stderr,"malloc error in SNFunc, buf\n");
  snprintf(buf,s.str().size()+1,"%s",s.str().c_str());
  sqlite3_result_text(context,buf,s.str().size()+1,free );

}



/*
** An instance of the following structure holds the context of a
** S()
*/
#define MAXSSC 100

typedef struct SCtx SCtx;
struct SCtx {
  double sum;     /* Sum of terms */
  int cnt;        /* Number of elements summed */
  int sscnt;
};

std::stringstream ss[MAXSSC];
int sscnt=0;

/*
** Routines used to compute the sum 
*/
static void SStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  SCtx *p=NULL;
  int i;

  std::string d;
  if( argc<1 ) return;
  p = (SCtx *) sqlite3_aggregate_context(context, sizeof(*p));
  if( p->cnt == 0)
    {
      if ( sscnt  >= MAXSSC )
	{ fprintf(stderr,"MAXSSC needs to increase\n");
	  exit(1);
	}
      p->sscnt=sscnt;
      sscnt++;
      ss[p->sscnt].str("");
      ss[p->sscnt] << "(";
     d="";
    } else {
       d=",";
       
    }    

    p->sum += sqlite3_value_double(argv[0]);
    p->cnt++;
    ss[p->sscnt] << d <<  p->sum ;


/*
 *      If the simple function is not used this
 *      comes into play.
 */
    if (p->cnt == 1)
      {
	for(i=1; i< argc; ++i) {
          p->cnt++;
	  p->sum+=sqlite3_value_double(argv[i]);
          ss[p->sscnt] <<  "," << p->sum ;
	  }

      }




}
static void SFinalize(sqlite3_context *context){
  SCtx *p=NULL;
  char *buf=NULL;
  p = (SCtx *) sqlite3_aggregate_context(context, sizeof(*p));

  ss[p->sscnt] << ")";
  buf = (char *) malloc (sizeof(char)*(ss[p->sscnt].str().size()+1));
  if (buf == NULL)
    fprintf(stderr,"malloc error in SNFinalize, buf\n");



  snprintf(buf,ss[p->sscnt].str().size()+1,"%s",ss[p->sscnt].str().c_str());
  sqlite3_result_text(context,buf,ss[p->sscnt].str().size()+1,free );
  sscnt--;

}





#define MAXSSL 100

typedef struct Ltx Ltx;
struct Ltx {
  int cnt;        /* Number of elements summed */
  int sscnt;
};

std::stringstream ssL[MAXSSL];
int sscntL=0;

/*
** An instance of the following structure holds the context of a
** list() from aggregate
*/

static void listStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  Ltx *p;
  int i;
  std::string d="";

  if( argc<1 ) 
     return;

  p = (Ltx *) sqlite3_aggregate_context(context, sizeof(*p));
  if ( p->cnt == 0)
    {
      if (sscntL >= MAXSSL )
        { 
	  fprintf(stderr,"Above in listStep increase MAXSSL size\n");
          exit(1);
	}
    p->sscnt=sscntL;
    sscntL++;
    ssL[p->sscnt].str("");
    ssL[p->sscnt] << "(";
    }	else {

      d=",";
    }



  p->cnt++;   
  //ssL[p->sscnt] << "(" <<  p->cnt<< "," << sqlite3_value_text(argv[0]) << ")";
  ssL[p->sscnt] << d <<  sqlite3_value_text(argv[0]);
  d=",";

  if (p->cnt == 1)
   {
    for(i=1; i< argc; ++i) {
     p->cnt++;
     ssL[p->sscnt] << d  << sqlite3_value_text(argv[i]) ;
      }
    }
}

static void listFinalize(sqlite3_context *context){

  Ltx *p=NULL;

  char *buf=NULL;

  p = (Ltx *) sqlite3_aggregate_context(context, sizeof(*p));
  ssL[p->sscnt] << ")";
  
  buf = (char *) malloc (sizeof(char)*(ssL[p->sscnt].str().size()+2));
  if (buf == NULL)
    fprintf(stderr,"malloc error in listFinalize, buf\n");
  

  snprintf(buf,ssL[p->sscnt].str().size()+1,"%s",ssL[p->sscnt].str().c_str());
  sqlite3_result_text(context,buf,ssL[p->sscnt].str().size()+2,free );
  ssL[p->sscnt].clear();
  sscntL--;

}





int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  if( argc!=3 ){
    fprintf(stderr, "Usage: %s DATABASE 'select S(1,2,3,4,5,6)'\n", argv[0]);
    exit(1);
  }
  rc = sqlite3_open(argv[1], &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }

  if (sqlite3_create_function(db, "msign", 1, SQLITE_UTF8, NULL, &msignFunc, NULL,
			      NULL) != 0)
    fprintf(stderr,"Problem with msign\n");



  if (sqlite3_create_function(db, "T", -1, SQLITE_UTF8, NULL, &TFunc, NULL,
			      NULL) != 0)
    fprintf(stderr,"Problem with T using just TFunc \n");



  if (sqlite3_create_function(db, "F", 1, SQLITE_UTF8, NULL, &FFunc, NULL,
			      NULL) != 0)
    fprintf(stderr,"Problem with F using just FFunc \n");


  if (sqlite3_create_function(db, "I", 2, SQLITE_UTF8, NULL, &IFunc, NULL,
			      NULL) != 0)
    fprintf(stderr,"Problem with I using just IFunc \n");








  /*
   *  With 2 or more arguments call the simple function ssum. Simple functions
   *  can be used within an expression. Aggregate functions can only be used
   *  in a select.
   */
  
  if (sqlite3_create_function(db, "S", -1, SQLITE_UTF8, NULL, &SFunc, NULL,
			      NULL) != 0)
    fprintf(stderr,"Problem with S using just SFunc \n");
  


  /*
   *  With one argument call the aggregate function.
   */

  if (sqlite3_create_function(db, "S", 1, SQLITE_UTF8, NULL, NULL, &SStep,
			      &SFinalize) != 0)
    fprintf(stderr,"Problem with S using SStep and SFinalize\n");


  

  if (sqlite3_create_function(db, "L", 1, SQLITE_UTF8, NULL, NULL, &listStep,
			      &listFinalize) != 0)
    fprintf(stderr,"Problem with list using listStep and listFinalize\n");


  rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
  }
  sqlite3_close(db);
  return 0;
}
