CREATE OR REPLACE FUNCTION massbase.cosine_similarity(a float4[], b float4[])
 RETURNS float8
 LANGUAGE plpgsql
 strict immutable parallel safe
AS $function$
declare
i int2 := 0;
l int2 := array_length(a,1);
_ma float4 := 0;
_mb float4 := 0;
_dp float4 := 0;
begin
if (l != array_length(b,1)) then
	raise exception 'Input array sizes must be equal for cosine similarity.';
end if;
for i in 1..l loop
	_dp = _dp + a[i] * b[i];
	_ma = _ma + power(a[i], 2);
	_mb = _mb + power(b[i], 2);
end loop;
return (_dp / (sqrt(_ma * _mb)))::float8;
end;
$function$;

CREATE OR REPLACE FUNCTION massbase.cosine_similarity(a float4[], b float4[])
 RETURNS float8
 LANGUAGE sql
 strict immutable parallel safe
AS $function$
select 
	(sum(r._a * r._b) /
	(sqrt(sum(power(r._a, 2)) * sum(power(r._b, 2)))))::float8
	from unnest(a, b) as r(_a, _b);
$function$;
