-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pgext" to load this file. \quit


-- Converts from one unit to other unit
CREATE OR REPLACE FUNCTION convert_exact(double precision, text, text)
RETURNS double precision
AS 'MODULE_PATHNAME', 'convert_exact'
LANGUAGE C IMMUTABLE STRICT;

-- Converts from one unit to all other units
CREATE OR REPLACE FUNCTION convert_all(double precision, text)
RETURNS SETOF unit_obj
AS 'MODULE_PATHNAME', 'convert_all'
LANGUAGE C IMMUTABLE STRICT;

-- Had to add INPUT function for unit_obj type
CREATE OR REPLACE FUNCTION unit_obj_in(cstring)
RETURNS unit_obj
AS 'MODULE_PATHNAME', 'unit_obj_in'
LANGUAGE C IMMUTABLE STRICT;

-- Had to add OUTPUT function for unit_obj type
CREATE OR REPLACE FUNCTION unit_obj_out(unit_obj)
RETURNS cstring
AS 'MODULE_PATHNAME', 'unit_obj_out'
LANGUAGE C IMMUTABLE STRICT;

--  Measuring unit (value and unit type)
CREATE TYPE unit_obj
(
    INPUT = unit_obj_in,
    OUTPUT = unit_obj_out,
    INTERNALLENGTH = 40
);
COMMENT ON TYPE unit_obj IS 'Composite type with value (double) and unit_type (char[32])';

-- Converts unit_obj to text and vice versa
CREATE OR REPLACE FUNCTION unit_obj(text)
RETURNS unit_obj
AS 'MODULE_PATHNAME', 'text_to_unit_obj'
LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION text(unit_obj)
RETURNS text
AS 'MODULE_PATHNAME', 'unit_obj_to_text'
LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (text as unit_obj) WITH FUNCTION unit_obj(text) AS IMPLICIT;
CREATE CAST (unit_obj as text) WITH FUNCTION text(unit_obj);