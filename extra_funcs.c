//#include <stdio.h>
#include "postgres.h"
#include "executor/executor.h"
#include "executor/spi.h"
#include "utils/builtins.h"



//#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
//#endif
 
PG_FUNCTION_INFO_V1(iif);
 
Datum
iif (PG_FUNCTION_ARGS){
   bool message = PG_GETARG_BOOL(0);
   float8 first = PG_GETARG_FLOAT8(1);
   float8 second = PG_GETARG_FLOAT8(2);
 
   if(message == 1){
      PG_RETURN_FLOAT8(first);
   }else{
      PG_RETURN_FLOAT8(second);
   }
}


PG_FUNCTION_INFO_V1(whoislogged);


/*
whoislogged: 
  Return the number of users logged and the details in INFO format.
*/

Datum
whoislogged (PG_FUNCTION_ARGS){
    //char *command;
    int ret;
    int proc;

    //command = text_to_cstring(sql);

    /* Convert given text object to a C string */
    //command = sql;

    /* connect to SPI manager */
    if ((ret = SPI_connect()) < 0)
        elog(ERROR, " SPI_connect returned %d", ret);

    ret = SPI_exec("SELECT distinct usename FROM pg_stat_activity", 0); // http://www.postgresql.org/docs/9.5/static/spi-spi-exec.html

    proc = SPI_processed;
    /*
     * If some rows were fetched, print them via elog(INFO).
     */
    if (ret > 0 && SPI_tuptable != NULL)
    {
        TupleDesc tupdesc = SPI_tuptable->tupdesc;
        SPITupleTable *tuptable = SPI_tuptable;
        char buf[8192];
        int i, j;

        for (j = 0; j < proc; j++)
        {
            HeapTuple tuple = tuptable->vals[j];

            /*for (i = 1, buf[0] = 0; i <= tupdesc->natts; i++)
                snprintf(buf + strlen (buf), sizeof(buf) - strlen(buf), " %s%s",
                        SPI_getvalue(tuple, tupdesc, i),
                        (i == tupdesc->natts) ? " " : " |");
            */
            elog(INFO, "USER LOGGED: %s", SPI_getvalue(tuple, tupdesc,1));
        }
    }

    SPI_finish();
    //pfree(command);
    PG_RETURN_INT32(proc);
    //return (proc);
}
