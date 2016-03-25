# contrib/citext/Makefile

MODULES = extra_funcs

EXTENSION = extra_funcs
DATA = extra_funcs--0.1.sql extra_funcs--unpackaged--0.1.sql
PGFILEDESC = "extra_funcs - Just a couple of example and extra functions"

REGRESS = extra_funcs

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/extra_funcs
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
