EXTENSION = pgext
MODULES = pgext
DATA = pgext--0.1.sql pgext.control

LDFLAGS =- lrt

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)