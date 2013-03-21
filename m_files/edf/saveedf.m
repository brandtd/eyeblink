%
%   saveedf( edf, filename )
%
%   Saves an edf struct as given by the readedf() function to file in EDF
%   format. The EDF struct is verified to have the expected format before being
%   saved, and if the EDF format is not respected, an error will be raised.
%
%   See readedf() for a definition of the expected fields within the edf struct.
%
function saveedf( edf, filename )

  % TODO: this doesn't check the 'num_records' field.

  % The number of labels, transducers, etc. must match the number of signals.
  reqs = { { 'labels',              size(edf.labels,1) },
           { 'transducers',         size(edf.transducers,1) },
           { 'physical dimensions', size(edf.phys_dim,1) },
           { 'physical minimums',   size(edf.phys_min,1) },
           { 'physical maximums',   size(edf.phys_max,1) },
           { 'digital minimums',    size(edf.dig_min,1) },
           { 'digital maximums',    size(edf.dig_max,1) },
           { 'prefilters',          size(edf.prefilters,1) },
           { 'sample counts',       size(edf.num_samples,1) } };

  % Verify the number of labels, transducers, etc.
  for i = 1:size(reqs,1)
    if reqs{i}{2} ~= edf.num_signals
      error( 'Number of %s must match number of signals.', reqs{i}{1} );
    end
  end

  % Each cell array in reqs contains:
  %   { 'Field Name', given_value, required_length }
  % for each string field in the edf.
  reqs = { { 'Version', edf.version, 8 },
           { 'Patient ID', edf.patient_id, 80 },
           { 'Recording ID', edf.recording_id, 80 },
           { 'Start Date', edf.start_date, 8 },
           { 'Start Time', edf.start_time, 8 } };

  for i = 1:edf.num_signals
    reqs{(4*i - 3) + 5} = { 'Labels', edf.labels(i,:), 16 };
    reqs{(4*i - 2) + 5} = { 'Prefilters', edf.prefilters(i,:), 80 };
    reqs{(4*i - 1) + 5} = { 'Physical Dimensions', edf.phys_dim(i,:), 8 };
    reqs{(4*i)     + 5} = { 'Transducers', edf.transducers(i,:), 80 };
  end

  % Verify that the edf struct we were given meets the string length
  % requirements.
  for i = 1:size(reqs,2)
    if ischar( reqs{i}{2} ) ~= 1 || length( reqs{i}{2} ) ~= reqs{i}{3}
      error( '%s must be a(n) %d character string.', reqs{i}{1}, reqs{i}{3} );
    end
  end

  % Verify that the number of samples in each channel matches up with the
  % number given.
  for i = 1:edf.num_signals
    if edf.num_samples(i) ~= length(edf.samples(i,:))
      error( 'Stated number of samples does not match given number.' );
    end
  end

  % Everything seems to check out, so open the file and begin writing.

  if length(filename) <= 4 || strcmp( filename(end-3:end), '.edf' ) ~= 1
    warning( 'Saving EDF data to file ''%s'' (no .edf extension)', filename );
  end

  fid = fopen( filename, 'wb' );

  if fid == -1
    error( 'Failed to open file, ''%s''. Does the parent directory exist?', ...
           filename );
  end

  % Write the header information to the file.
  fwrite( fid, edf.version );
  fwrite( fid, edf.patient_id );
  fwrite( fid, edf.recording_id );
  fwrite( fid, edf.start_date );
  fwrite( fid, edf.start_time );
  fwrite( fid, sprintf( '%-8d', 256 + 256 * edf.num_signals ) );
  fwrite( fid, zeros(1,44) + 32, 'uchar' );
  fwrite( fid, sprintf( '%-8d', edf.num_records ) );
  fwrite( fid, sprintf( '%-8d', edf.duration ) );
  fwrite( fid, sprintf( '%-4d', edf.num_signals ) );
  fwrite( fid, edf.labels' );
  fwrite( fid, edf.transducers' );
  fwrite( fid, edf.phys_dim' );
  fwrite( fid, reshape(sprintf( '%-8d', edf.phys_min ),8,edf.num_signals) );
  fwrite( fid, reshape(sprintf( '%-8d', edf.phys_max ),8,edf.num_signals) );
  fwrite( fid, reshape(sprintf( '%-8d', edf.dig_min ), 8,edf.num_signals) );
  fwrite( fid, reshape(sprintf( '%-8d', edf.dig_max ), 8,edf.num_signals) );
  fwrite( fid, edf.prefilters' );
  fwrite( fid, reshape(sprintf( '%-8d', edf.num_samples ), 8,edf.num_signals) );
  fwrite( fid, zeros(1,32 * edf.num_signals) + 32, 'uchar' );

  % Figure out the sizes of the 'integer' used by each signal.
  integer_lens = ceil(log2(edf.dig_max - edf.dig_min)) / 8;

  % Convert the sample data into integer values.
  slope     = (edf.phys_max - edf.phys_min) ./ (edf.dig_max - edf.dig_min);
  intercept = edf.phys_max - slope .* edf.dig_max;

  for i = 1:edf.num_signals
    edf.samples(i,:) = round((edf.samples(i,:) - intercept(i)) / slope(i));
  end

  % Write each sample to file.
  for i = 1:edf.num_signals
    switch(integer_lens(i))
      case 1
        datatype = 'int8';
      case 2
        datatype = 'int16';
      case 4
        datatype = 'int32';
      otherwise
        datatype = 'int8';
    end
    fwrite( fid, edf.samples(i,:), datatype );
  end

  fclose( fid );
end
