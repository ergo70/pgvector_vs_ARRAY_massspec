CREATE EXTENSION vector;

CREATE TABLE spectra_index (
	library_spectra_meta_id int4 not NULL,
	mz float4[] not NULL,
	intensity float4[] not null,
	num_peaks int2 NULL,
	mz_range numrange NULL,
	mz_vector public.vector NULL
);

CREATE INDEX mz_vector_idx ON spectra_index USING hnsw (mz_vector vector_cosine_ops);

CREATE INDEX num_peaks_idx ON spectra_index (num_peaks);
