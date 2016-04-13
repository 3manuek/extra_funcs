
\echo Use "CREATE EXTENSION extra_funcs" to load this file. \quit


CREATE OR REPLACE FUNCTION iif(BOOLEAN, DOUBLE PRECISION, DOUBLE PRECISION) RETURNS DOUBLE PRECISION
     AS 'MODULE_PATHNAME'
     LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION whoislogged() RETURNS int
     AS 'MODULE_PATHNAME'
     LANGUAGE C IMMUTABLE STRICT;


 DROP AGGREGATE IF EXISTS med(REAL);
 DROP FUNCTION IF EXISTS med_trans(state bytea, NEXT REAL);
 DROP FUNCTION IF EXISTS med_final(state bytea);

 CREATE FUNCTION med_trans(state bytea, NEXT REAL)
 RETURNS bytea AS '$libdir/statfunc'
 LANGUAGE C IMMUTABLE;
  
 	COMMENT ON FUNCTION med_trans(state bytea, NEXT REAL)
 	IS 'accumulates non-null values into an array';

 CREATE FUNCTION med_final(state bytea)
 RETURNS REAL AS '$libdir/statfunc'
 LANGUAGE C IMMUTABLE STRICT;

 	COMMENT ON FUNCTION med_final(state bytea)
 	IS 'sorts the array and returns the median';

 CREATE AGGREGATE med(REAL) (
   SFUNC = med_trans,
   STYPE = bytea,
   FINALFUNC = med_final
 );

 	COMMENT ON AGGREGATE med(REAL)
 	IS 'median of all input values, excluding nulls';
