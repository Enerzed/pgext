/*
* pgext.h
*/

#pragma once


#include "postgres.h"           // Содержит различные базовые типы и функции
#include "fmgr.h"               // PG_* макросы
#include "utils/elog.h"
#include "utils/builtins.h"
#include "utils/memutils.h"
#include "utils/guc.h"
#include "catalog/pg_type.h"
#include "lib/stringinfo.h"
#include "funcapi.h"
#include <string.h>


#define NUMBER_OF_UNITS 8

PG_MODULE_MAGIC;                // Макрос, определяющий, что это модуль для PostgreSQL

typedef struct
{
    double value;
    char unit_type[32];
} unit_obj;

bool enable_logging = false;

void _PG_init(void)
{
    DefineCustomBoolVariable
    ("pgext.enable_logging",
    "Enable logging for unit conversion extension (pgext)",
    NULL,
    &enable_logging,
    false,
    PGC_USERSET,
    0,
    NULL,
    NULL,
    NULL);
}

// Таблица с коеффициентами для перевода величин следующего вида
/*
                millimeter  | centimeter    | meter | kilometer | inch  | foot  | yard  | mile  |
millimeter  |               |               |       |           |       |       |       |       |
centimeter  |               |               |       |           |       |       |       |       |
meter       |               |               |       |           |       |       |       |       |
kilometer   |               |               |       |           |       |       |       |       |
inch        |               |               |       |           |       |       |       |       |
foot        |               |               |       |           |       |       |       |       |
yard        |               |               |       |           |       |       |       |       |
mile        |               |               |       |           |       |       |       |       |
*/
static double conversion_table[8][8] = 
{{1, 0.1, 0.001, 0.00001, 0.03937, 0.003281, 0.001094, 0.000000621371},
{10, 1, 0.01, 0.00001, 0.3937, 0.03281, 0.01094, 0.00000621371},
{1000, 100, 1, 0.001, 39.37, 3.28084, 1.09361, 0.000621371},
{1000000, 100000, 1000, 1, 39370.1, 3280.84, 1093.61, 0.621371},
{25.4, 2.54, 0.0254, 0.0000254, 1, 0.08333, 0.027778, 0.0000157828},
{304.8, 30.48, 0.3048, 0.0003048, 12, 1, 0.333333, 0.000189394},
{914.4, 91.44, 0.9144, 0.0009144, 36, 3, 1, 0.000568182},
{1609344, 160934.4, 1609.344, 1.609344, 63360, 5280, 1760, 1}};

// Datum - универсальный тип в PostgreSQL
// PG_FUNCTION_ARGS - аргументы функции в PostgreSQL, может определять любое количество параметров
// Названия функций говорят сами за себя
static unit_obj* cstring_to_unit_obj(char *);
static char* unit_obj_to_cstring(unit_obj *);

Datum unit_obj_to_text(PG_FUNCTION_ARGS);
Datum text_to_unit_obj(PG_FUNCTION_ARGS);
Datum unit_obj_in(PG_FUNCTION_ARGS);
Datum unit_obj_out(PG_FUNCTION_ARGS);
Datum convert_exact(PG_FUNCTION_ARGS);
Datum convert_all(PG_FUNCTION_ARGS);

static int get_unit_id(char *);
static char* get_unit_type(int);