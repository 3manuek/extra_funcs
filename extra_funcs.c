//#include <stdio.h>
#include "postgres.h"
#include "executor/executor.h"
 
#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
 
PG_FUNCTION_INFO_V1(iif_c);
 
static Datum
iif_c (PG_FUNCTION_ARGS){
   bool message = PG_GETARG_BOOL(0);
   float8 first = PG_GETARG_FLOAT8(1);
   float8 second = PG_GETARG_FLOAT8(2);
 
   if(message == 1){
      PG_RETURN_FLOAT8(first);
   }else{
      PG_RETURN_FLOAT8(second);
   }
}

