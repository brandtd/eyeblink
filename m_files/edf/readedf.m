%
%   edf = readedf( filename )
%
%   Reads and parses the EDF (European Data Format) file pointed to by the
%   string 'filename' and builds a struct with the following fields containing
%   the file's data:
%
%     |    field     | description
%     +--------------+------------------------------------------
%     | version      |  version of this data format
%     | patient_id   |  local patient identification
%     | recording_id |  local recording identification
%     | start_date   |  start date of recording (dd.mm.yy)
%     | start_time   |  start time of recording (hh.mm.ss)
%     | num_records  |  number of data records
%     | duration     |  duration of a data record, in seconds
%     | num_signals  |  number of signals in data record
%     | labels       |  signal labels
%     | transducers  |  transducer types
%     | phys_dim     |  physical dimension (e.g., uV, degreeC)
%     | phys_min     |  physical minimum
%     | phys_max     |  physical maximum
%     | dig_min      |  digital minimum
%     | dig_max      |  digital maximum
%     | prefilters   |  prefiltering data
%     | num_samples  |  number of samples in each data record
%     | samples      |  the samples
%
%   The samples that are returned have been linearly scaled in a way that maps
%   the digital maximum/minimum for a signal to the physical maximum/minimum for
%   that channel.
%
%   Signal information is stored in row-major format (each row represents a
%   different signal).
%
function edf = readedf( filename )
  edf = struct;

  fid = fopen( filename );

  if fid == -1
    error( 'Failed to open file, ''%s''.', filename );
  end

  edf.version       = char(fread(fid, [1 8]));
  edf.patient_id    = char(fread(fid, [1 80]));
  edf.recording_id  = char(fread(fid, [1 80]));
  edf.start_date    = char(fread(fid, [1 8]));
  edf.start_time    = char(fread(fid, [1 8]));

  % Discard the 8 bytes containing the length of the header field and the 44
  % bytes of reserved space following it.
  fread(fid, 8 + 44);

  edf.num_records   = str2num( char(fread(fid, [1  8])) );
  edf.duration      = str2num( char(fread(fid, [1  8])) );
  edf.num_signals   = str2num( char(fread(fid, [1  4])) );
  edf.labels        =          char(fread(fid, [16 edf.num_signals])');
  edf.transducers   =          char(fread(fid, [80 edf.num_signals])');
  edf.phys_dim      =          char(fread(fid, [8  edf.num_signals])');
  edf.phys_min      = str2num( char(fread(fid, [8  edf.num_signals])') );
  edf.phys_max      = str2num( char(fread(fid, [8  edf.num_signals])') );
  edf.dig_min       = str2num( char(fread(fid, [8  edf.num_signals])') );
  edf.dig_max       = str2num( char(fread(fid, [8  edf.num_signals])') );
  edf.prefilters    =          char(fread(fid, [80 edf.num_signals])');
  edf.num_samples   = str2num( char(fread(fid, [8  edf.num_signals])') );

  % Discard some more reserved stuff.
  fread(fid, 32 * edf.num_signals);

  % Figure out the sizes of the 'integer' used by each signal in the EDF file.
  integer_lens = ceil(log2( edf.dig_max - edf.dig_min )) / 8;

  % We're going to assume that each signal used the same integer size and that
  % each signal recorded the same number of samples.
  switch (integer_lens(1))
    case 1
      datatype = 'int8';
    case 2
      datatype = 'int16';
    case 4
      datatype = 'int32';
    otherwise
      datatype = 'int8';
  end

  edf.samples = fread(fid, [edf.num_samples(1) edf.num_signals], datatype)';

  % We're also going to assume a linear mapping between the digital and physical
  % values.
  slope     = (edf.phys_max - edf.phys_min) ./ (edf.dig_max - edf.dig_min);
  intercept = edf.phys_max - slope .* edf.dig_max;

  for iSignal = 1:edf.num_signals;
    edf.samples(iSignal, :) = edf.samples(iSignal,:) * slope(iSignal) + ...
                              intercept( iSignal );
  end

  fclose( fid );
end
