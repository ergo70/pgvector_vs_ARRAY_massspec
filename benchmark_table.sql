CREATE EXTENSION vector;

CREATE TABLE spectra_index (
   library_spectra_meta_id int4 NOT NULL
   num_peaks int2 NULL,
   mz_range numrange NULL,
   mz float4[] NOT NULL,
   intensity float4[] NOT NULL,
   mz_vector public.vector NULL,
   intensity_vector public.vector NULL);

CREATE INDEX spectra_index_num_peaks_idx ON spectra_index USING btree (num_peaks);
