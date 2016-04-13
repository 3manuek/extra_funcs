//#include <stdio.h>
#include "postgres.h"
#include "executor/executor.h"
#include "executor/spi.h"
#include "utils/builtins.h"

// Used by median :
#include "funcapi.h"


#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif


/*
  iif:
  Simulate iif Access functon on Pg
*/

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

  TODO: return setof name (users)
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


/* median :
   Inspired on https://wiki.postgresql.org/wiki/Aggregate_Median/C_code
   Discussion http://markmail.org/message/wr6rmfd56rhpwnij
*/

/*
 * statfunc.c - statistical functions for PostgreSQL
 */


#define MAXLEN 14

/*
 * Internal state: simple array of floats.
 */
typedef struct {
	int32 vl_len_;        /* varlena header (do not touch directly!) */
	int32 nval;           /* number of values accumulated */
	float4 vals[MAXLEN];  /* array of non-null values */
} State;

/*----------------------------------------------------------------------------*\
  C Implementation
\*----------------------------------------------------------------------------*/

/*
 * Accumulates non-null values into an array.
 */
static State *c_med_trans(State *state, float4 next) {
	if (state == NULL) {
		/* first value; allocate state */
		state = palloc0(sizeof(State));
		SET_VARSIZE(state, sizeof(State));
		state->nval = 1;
		state->vals[0] = next;
	} else {
		/* next value; check capacity */
		if (state->nval == MAXLEN) {
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
							errmsg("need to increase MAXLEN in statfunc.c")));
		}
		state->vals[state->nval] = next;
		state->nval++;
	}
	return state;
}

/*
 * Comparison function for qsort.
 */
static int float4cmp(const void *p1, const void *p2) {
	float4 f1 = *(const float4 *) p1;
	float4 f2 = *(const float4 *) p2;
	if (f1 < f2)
		return -1;
	if (f1 > f2)
		return 1;
	return 0;
}

/*
 * Sorts the array and returns the median.
 */
static float4 c_med_final(State *state) {
	int32 mid;
	float4 ret;
	qsort(state->vals, state->nval, sizeof(float4), float4cmp);
	mid = state->nval / 2;
	if (state->nval % 2) {
		/* odd number of elements */
		ret = state->vals[mid];
	} else {
		/* even number of elements */
		ret = (state->vals[mid] + state->vals[mid - 1]) / 2;
	}
	return ret;
}

/*----------------------------------------------------------------------------*\
  Postgres V1 Function Prototypes
\*----------------------------------------------------------------------------*/

Datum med_trans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(med_trans);

Datum med_final(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(med_final);

/*----------------------------------------------------------------------------*\
  SQL Interface (Wrappers)
\*----------------------------------------------------------------------------*/

Datum med_trans(PG_FUNCTION_ARGS) {
	State *state;
	float4 next;
	if (!fcinfo->context || !IsA(fcinfo->context, AggState)) {
		ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("med_trans() - must call from aggregate")));
	}
	if (PG_ARGISNULL(0)) {
		state = NULL; /* new group */
	} else {
		state = (State *) PG_GETARG_BYTEA_P(0);
	}
	if (PG_ARGISNULL(1)) {
		/* discard NULL input values */
	} else {
		next = PG_GETARG_FLOAT4(1);
		state = c_med_trans(state, next);
	}
	/* return the updated state */
	if (state == NULL) {
		PG_RETURN_NULL();
	} else {
		PG_RETURN_BYTEA_P(state);
	}
}

Datum med_final(PG_FUNCTION_ARGS) {
	State *state;
	float4 ret;
	if (!fcinfo->context || !IsA(fcinfo->context, AggState)) {
		ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("med_final() - must call from aggregate")));
	}
	state = (State *) PG_GETARG_BYTEA_P(0);
	ret = c_med_final(state);
	PG_RETURN_FLOAT4(ret);
}
