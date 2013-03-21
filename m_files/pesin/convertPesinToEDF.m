%
%   convertPesinToEDF( 'in_file', 'out_file_dir' )
%
%   This function converts the data files given in Jimy Pesin's thesis to EDF
%   files. I think I've got the labels and units correct, but I cannot be
%   positive because they are not documented. I'm guessing the mapping of data
%   columns to EEG channel labels from how his code labels things, and I'm
%   guessing physical dimensions by comparing the values I've seen in Xltek
%   files to the values I've seen in Jimy's files.
%
%   'in_file' should be the filename of one of Jimy's data files.
%
%   'out_file_dir' should be where to store the resulting EDF files. This
%   function will create three EDF files per input file. The output files will
%   have the same name as the input file, with once of '_1', '_2', or '_3'
%   appended (and, of course, the extension changed to '.edf').
%
function convertPesinToEDF( in_file, out_file_dir )
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Find the start date and time from the first two fields in Pesin's file.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  fid = fopen( in_file );
  if fid == -1
    error( 'Failed to open file, ''%s''.', in_file );
  end

  % Read in each field, discarding the separator between fields.
  month = char(fread(fid, [1 2])); fread(fid, 1);
  day   = char(fread(fid, [1 2])); fread(fid, 1);
  year  = char(fread(fid, [1 4])); fread(fid, 1);

  hour   = char(fread(fid, [1 2])); fread(fid, 1);
  minute = char(fread(fid, [1 2])); fread(fid, 1);
  second = char(fread(fid, [1 2])); fread(fid, 1);

  fclose(in_file);

  % We only want the decade--don't care about the century.
  year = year(:,3:4);

  % Build the date/time strings expected within and EDF file.
  start_date = sprintf('%s.%s.%s', day, month, year);
  start_time = sprintf('%s.%s.%s', hour, minute, second);

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Build the header for the EDF file. All EDF files created from Pesin's data
  % files will have the same header.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  edf = struct;

  edf.version      = '0       ';
  edf.patient_id   = char( ones(1,80) * 32 );
  edf.recording_id = char( ones(1,80) * 32 );
  edf.start_date   = start_date;
  edf.start_time   = start_time;
  edf.num_records  = 1;
  edf.duration     = 10;
  edf.num_signals  = 25;
  edf.labels       = [ 'C3              ';
                       'C4              ';
                       'CZ              ';
                       'F3              ';
                       'F4              ';
                       'F7              ';
                       'F8              ';
                       'FZ              ';
                       'FP1             ';
                       'FP2             ';
                       'O1              ';
                       'O2              ';
                       'P3              ';
                       'P4              ';
                       'PZ              ';
                       'T7              ';
                       'T8              ';
                       'P7              ';
                       'P8              ';
                       'FT9             ';
                       'FT10            ';
                       'LEOG            ';
                       'REOG            ';
                       'A1              ';
                       'A2              ' ];
  edf.transducers   = char( ones(25,80) * 32 );
  edf.phys_dim      = [ 'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ';
                        'uV      ' ];
  edf.dig_min        = ones(25,1) * -32768;
  edf.dig_max        = ones(25,1) *  32767;
  edf.prefilters     = char( ones(25,80) * 32 );
  edf.num_samples    = ones(25,1) * 5000;

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Save Pesin's data in 5000 sample chunks.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  data = getData( in_file ) * 100; % I think it's stored as mV?
  [dir, name, ext] = fileparts( in_file );

  % Each data file contains three chunks. The sample counts never exactly match
  % up to 15000, so we have to improvise a little.
  start_data = data(:,1:5000);
  mid_data   = data(:,5001:10000);
  end_data   = data(:,(end-4999):end);

  % Each data file requires a different physical minimum and maximum.
  edf.phys_min = min(start_data')';
  edf.phys_max = max(start_data')';
  edf.samples  = start_data;

  out_file = fullfile( out_file_dir, strcat( name, '_1', '.edf' ) );
  saveedf( edf, out_file );

  edf.phys_min = min(mid_data')';
  edf.phys_max = max(mid_data')';
  edf.samples  = mid_data;

  out_file = fullfile( out_file_dir, strcat( name, '_2', '.edf' ) );
  saveedf( edf, out_file );

  edf.phys_min = min(end_data')';
  edf.phys_max = max(end_data')';
  edf.samples  = end_data;

  out_file = fullfile( out_file_dir, strcat( name, '_3', '.edf' ) );
  saveedf( edf, out_file );
end
