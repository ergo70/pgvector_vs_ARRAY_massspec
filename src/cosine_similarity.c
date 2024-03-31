#include <math.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "access/htup.h"
#include "catalog/pg_type.h"
#include "vector.h"

/*
CREATE FUNCTION cosine_similarity_array(float4[], float4[]) RETURNS float8
     AS '$libdir/cosine_similarity', 'cosine_similarity_array'
     LANGUAGE C strict immutable parallel safe;
    
CREATE FUNCTION cosine_similarity_vector(vector, vector) RETURNS float8
     AS '$libdir/cosine_similarity', 'cosine_similarity_vector'
     LANGUAGE C strict immutable parallel safe;
*/

PG_MODULE_MAGIC;

PGDLLEXPORT PG_FUNCTION_INFO_V1(cosine_similarity_array);

Datum cosine_similarity_array(PG_FUNCTION_ARGS)
{
	ArrayType* a = PG_GETARG_ARRAYTYPE_P(0), * b = PG_GETARG_ARRAYTYPE_P(1);
	Oid elemtypeA = ARR_ELEMTYPE(a), elemtypeB = ARR_ELEMTYPE(b);
	Datum* datumsA = NULL, * datumsB = NULL;
	int     countA = 0, countB = 0;
	int16   elemWidthA, elemWidthB;
	bool    elemTypeByValA, elemTypeByValB;
	char    elemAlignmentCodeA, elemAlignmentCodeB;
	float4  fieldA = 0.0f, fieldB = 0.0f, dp = 0.0f, ma = 0.0f, mb = 0.0f;
	float8  retval = -666.0;

	if (elemtypeA != FLOAT4OID || elemtypeB != FLOAT4OID)
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION), errmsg("float4 OID arrays needed. Got %d %d", elemtypeA, elemtypeB)));

	if (ARR_NDIM(a) != 1 || ARR_NDIM(b) != 1)
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION), errmsg("One-dimensional arrays needed. Got %d %d", ARR_NDIM(a), ARR_NDIM(b))));

	if (ArrayGetNItems(ARR_NDIM(a), ARR_DIMS(a)) != ArrayGetNItems(ARR_NDIM(b), ARR_DIMS(b)))
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION), errmsg("Arrays of equal size needed. Got %d %d", ArrayGetNItems(ARR_NDIM(a), ARR_DIMS(a)), ArrayGetNItems(ARR_NDIM(b), ARR_DIMS(b)))));

	get_typlenbyvalalign(elemtypeA, &elemWidthA, &elemTypeByValA, &elemAlignmentCodeA);
	get_typlenbyvalalign(elemtypeB, &elemWidthB, &elemTypeByValB, &elemAlignmentCodeB);

	deconstruct_array(a, elemtypeA, elemWidthA, elemTypeByValA, elemAlignmentCodeA, &datumsA, NULL, &countA);
	deconstruct_array(b, elemtypeB, elemWidthB, elemTypeByValB, elemAlignmentCodeB, &datumsB, NULL, &countB);

	for (int i = 0; i < countA; i++)
	{
		// This is the correct way to access the array elements. But it will not be auto-vectorized. Since the Datums are float4/int32 unions
		// internally, we can cast them to float4 as long as PostgreSQL does not change the implementation of Datum. Hack below!
		fieldA = DatumGetFloat4(datumsA[i]);
		fieldB = DatumGetFloat4(datumsB[i]);
		dp += fieldA * fieldB;
		ma += fieldA * fieldA;
		mb += fieldB * fieldB;

		// MSVC: Auto-vectorized (SIMD instructions, aka. MMX, SSE, AVX, NEON). ONLY when /fp:fast is set
		// Replace: Datum* datumsA = NULL, * datumsB = NULL;
		// With: float4* datumsA = NULL, * datumsB = NULL;
		//dp += datumsA[i] * datumsB[i];
		//ma += datumsA[i] * datumsA[i];
		//mb += datumsB[i] * datumsB[i];
	}

	if (NULL != datumsA)
		pfree(datumsA);

	if (NULL != datumsB)
		pfree(datumsB);

	retval = dp / (sqrt(ma * mb));

	PG_RETURN_FLOAT8(retval);
}

PGDLLEXPORT PG_FUNCTION_INFO_V1(cosine_similarity_vector);

Datum cosine_similarity_vector(PG_FUNCTION_ARGS)
{
	Vector* a = PG_GETARG_VECTOR_P(0);
	Vector* b = PG_GETARG_VECTOR_P(1);
	float* ax = a->x;
	float* bx = b->x;
	float		distance = 0.0f;
	float		norma = 0.0f;
	float		normb = 0.0f;
	float8		similarity = -666.0;

	if (a->dim != b->dim)
		ereport(ERROR,
			(errcode(ERRCODE_DATA_EXCEPTION),
				errmsg("different vector dimensions %d and %d", a->dim, b->dim)));

	/* Auto-vectorized. ONLY when /fp:fast is set */
	for (int i = 0; i < a->dim; i++)
	{
		distance += ax[i] * bx[i];
		norma += ax[i] * ax[i];
		normb += bx[i] * bx[i];
	}

	similarity = (double)distance / sqrt((double)norma * (double)normb);

	PG_RETURN_FLOAT8(similarity);
}
