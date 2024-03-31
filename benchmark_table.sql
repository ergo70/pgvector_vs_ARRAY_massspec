CREATE EXTENSION vector;

CREATE TABLE spectra_index (
	library_spectra_meta_id int4 not NULL,
	mz float4[] not NULL,
	intensity float4[] not null,
	num_peaks int2 NULL,
	mz_range numrange NULL,
	mz_vector public.vector NULL
);

CREATE INDEX spectra_index_num_peaks_idx ON massbase.spectra_index USING btree (num_peaks);
