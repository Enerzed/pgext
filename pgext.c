/*
* pgext.c
*/


#include "pgext.h"


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

static unit_obj* cstring_to_unit_obj(char *cstring)
{
    double value;
    char unit_type[32];

    if(sscanf(cstring, "(%lf, %s)", &value, &unit_type[32]) != 2)
    {
        ereport(ERROR, errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), errmsg("Invalid input for unit_obj %s", cstring));
    }
    unit_obj *unit = (unit_obj*) palloc(sizeof(unit_obj));

    unit->value = value;
    strcpy(unit->unit_type, unit_type);

    return unit;
}

static char* unit_obj_to_cstring(unit_obj *unit)
{
    char *cstring = psprintf("(%lf, %s)", unit->value, unit->unit_type);
    return cstring;
}

PG_FUNCTION_INFO_V1(unit_obj_in);
Datum unit_obj_in(PG_FUNCTION_ARGS)
{
    char* cstring = PG_GETARG_CSTRING(0);
    PG_RETURN_POINTER(cstring_to_unit_obj(cstring));
}

PG_FUNCTION_INFO_V1(unit_obj_out);
Datum unit_obj_out(PG_FUNCTION_ARGS)
{
    unit_obj* unit = PG_GETARG_POINTER(0);
    PG_RETURN_CSTRING(unit_obj_to_cstring(unit));
}

PG_FUNCTION_INFO_V1(text_to_unit_obj);
Datum text_to_unit_obj(PG_FUNCTION_ARGS)
{
    text* txt = PG_GETARG_TEXT_P(0);
    char* str = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(txt)));
    PG_RETURN_POINTER(cstring_to_unit_obj(str));
}

PG_FUNCTION_INFO_V1(unit_obj_to_text);
Datum unit_obj_to_text(PG_FUNCTION_ARGS)
{
    unit_obj* unit = PG_GETARG_POINTER(0);
    text* out = DirectFunctionCall1(textin, PointerGetDatum(unit_obj_to_cstring(unit)));
    PG_RETURN_TEXT_P(out);
}

PG_FUNCTION_INFO_V1(convert_exact);
Datum convert_exact(PG_FUNCTION_ARGS)
{
    double x = PG_GETARG_FLOAT8(0);
    char *from_unit = text_to_cstring(PG_GETARG_TEXT_P(1));
    char *to_unit = text_to_cstring(PG_GETARG_TEXT_P(2));
    int from_unit_id = get_unit_id(from_unit);
    int to_unit_id = get_unit_id(to_unit);

    if (from_unit_id == -1 || to_unit_id == -1)
    {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("unsupported unit type for conversion: %s to %s", from_unit, to_unit)));
    }

    double result;

    // Конвертация
    result = x * conversion_table[from_unit_id][to_unit_id];

    PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(convert_all);
Datum convert_all(PG_FUNCTION_ARGS)
{
    double x = PG_GETARG_FLOAT8(0);
    char *from_unit = text_to_cstring(PG_GETARG_TEXT_P(1));
    int from_unit_id = get_unit_id(from_unit);

    if (from_unit_id == -1)
    {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("unsupported unit type for conversion: %s", from_unit)));
    }

    FuncCallContext *func_context;

    if (SRF_IS_FIRSTCALL())
    {
        MemoryContext old_context;

        func_context = SRF_FIRSTCALL_INIT();

        old_context = MemoryContextSwitchTo(func_context->multi_call_memory_ctx);

        func_context->max_calls = NUMBER_OF_UNITS;

        MemoryContextSwitchTo(old_context);
    }
    else
    {
        func_context = SRF_PERCALL_SETUP();
    }

    if (func_context->call_cntr < func_context->max_calls) 
    {
        unit_obj *result = (unit_obj *) palloc(sizeof(unit_obj));
        result->value = x * conversion_table[from_unit_id][func_context->call_cntr];
        strncpy(result->unit_type, get_unit_type(func_context->call_cntr), sizeof(result->unit_type) - 1);
        result->unit_type[sizeof(result->unit_type) - 1] = '\0';
        SRF_RETURN_NEXT(func_context, PointerGetDatum(result));
    }
    else
    {
        SRF_RETURN_DONE(func_context);
    }
}

static int get_unit_id(char *unit_type)
{
    if (strcmp(unit_type, "millimeter") == 0)
        return 0;
    else if (strcmp(unit_type, "centimeter") == 0)
        return 1;
    else if (strcmp(unit_type, "meter") == 0)
        return 2;
    else if (strcmp(unit_type, "kilometer") == 0)
        return 3;
    else if (strcmp(unit_type, "inch") == 0)
        return 4;
    else if (strcmp(unit_type, "foot") == 0)
        return 5;
    else if (strcmp(unit_type, "yard") == 0)
        return 6;
    else if (strcmp(unit_type, "mile") == 0)
        return 7;
    else
        return -1;
}

static char* get_unit_type(int unit_id)
{
    for(int i = 0; i < NUMBER_OF_UNITS; i++)
    {
        switch(unit_id)
        {
            case 0: return "millimeter"; break;
            case 1: return "centimeter"; break;
            case 2: return "meter"; break;
            case 3: return "kilometer"; break;
            case 4: return "inch"; break;
            case 5: return "foot"; break;
            case 6: return "yard"; break;
            case 7: return "mile"; break;
            default: ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("unsupported unit type for conversion: %i", unit_id)));
            break;
        }
    }
    return "UNIT DOES NOT EXISTS";
}