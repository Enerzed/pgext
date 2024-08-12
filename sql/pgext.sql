CREATE EXTENSION pgext;

SELECT * FROM convert_exact(10, 'meter', 'yard');

SELECT * FROM convert_exact(0, 'mile', 'inch');

SELECT * FROM convert_all(10, 'meter');

SELECT * FROM convert_all(0, 'kilometer');