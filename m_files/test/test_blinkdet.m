%
%   Tests the blink detection algorithm against authentic and simulated data,
%   and generates a short report listing the number of blinks in the EEG, the
%   number of blinks that were found, and the number of blinks that were falsely
%   detected (i.e., detections where there were no blinks).
%

% File names with fiducial index of blink location.
edf_files = { {'../eeg_data/with_blink/normal/normal0.edf', [ 3135 ]},
              {'../eeg_data/with_blink/normal/normal1.edf', [ 2340 ]},
              {'../eeg_data/with_blink/normal/normal2.edf', [ 493, 3870 ]},
              {'../eeg_data/with_blink/normal/normal3.edf', [ 2247 ]},
              {'../eeg_data/with_blink/normal/normal4.edf', [ 182, 3319 ]},
              {'../eeg_data/with_blink/normal/normal5.edf', [ 1802 ]},
              {'../eeg_data/no_blink/normal/normal0.edf', []},
              {'../eeg_data/no_blink/normal/normal1.edf', []},
              {'../eeg_data/no_blink/normal/normal2.edf', []},
              {'../eeg_data/no_blink/normal/normal3.edf', []},
              {'../eeg_data/no_blink/normal/normal4.edf', []},
              {'../eeg_data/no_blink/normal/normal5.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic0.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic1.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic2.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic3.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic4.edf', []},
              {'../eeg_data/no_blink/triphasic/triphasic5.edf', []},
              {'../eeg_data/no_blink/firda/firda0.edf', []},
              {'../eeg_data/no_blink/firda/firda1.edf', []},
              {'../eeg_data/no_blink/firda/firda2.edf', []},
              {'../eeg_data/no_blink/firda/firda3.edf', []},
              {'../eeg_data/no_blink/firda/firda4.edf', []},
              {'../eeg_data/no_blink/firda/firda5.edf', []} };

% Load the simulated EEG data as well. The fiducial index to use depends on the
% EEG used to add the eyeblink, which will have been one of normal[0-5].edf.
n_files = length(edf_files);

% Add the simulated normal EEG.
for i = 0:5
  sim_edfs = ls(sprintf('../eeg_data/simulated/sim_n%d_n*', i ));

  for j = 1:size(sim_edfs,1)
    n_files = n_files + 1;
    edf_files{n_files} = { sim_edfs(j,:), edf_files{i+1}{2} };
  end
end

% Add the simulated triphasic EEG.
for i = 0:5
  sim_edfs = ls(sprintf('../eeg_data/simulated/sim_n%d_t*', i ));

  for j = 1:size(sim_edfs,1)
    n_files = n_files + 1;
    edf_files{n_files} = { sim_edfs(j,:), edf_files{i+1}{2} };
  end
end

% Add the simulated FIRDA EEG.
for i = 0:5
  sim_edfs = ls(sprintf('../eeg_data/simulated/sim_n%d_f*', i ));

  for j = 1:size(sim_edfs,1)
    n_files = n_files + 1;
    edf_files{n_files} = { sim_edfs(j,:), edf_files{i+1}{2} };
  end
end

results = {};

for i = 1:length(edf_files)
  edf = readedf( edf_files{i}{1} );

  % Find the LEOG and REOG channels.
  for j = 1:edf.num_signals
    if strcmp( strtrim(edf.labels(j,:)), 'LEOG' ) == 1
      i_leog = j;
    elseif strcmp( strtrim(edf.labels(j,:)), 'REOG' ) == 1
      i_reog = j;
    end
  end

  % Calculate the channels to pass to the blink detection algorithm.
  channels = [ diff_channels( edf, 'FP1', 'F3' );
               diff_channels( edf, 'FP1', 'F7' );
               diff_channels( edf, 'FP2', 'F4' );
               diff_channels( edf, 'FP2', 'F8' ) ];

  % Find the sample frequency for the channels.
  sample_freq = edf.num_samples(1) / edf.duration;

  % Run the blink detection algorithm.
  blinks = blink_detect( channels, 'fs', sample_freq );

  % Time to find out how well the blink detection algorithm performed with our
  % expectations.
  acceptable_error = 0.25;
  acc_error_samples = acceptable_error * sample_freq;

  % Setup the result struct in which to store results for this test.
  result = struct;
  result.file = edf_files{i}{1};
  result.num_blinks = length(edf_files{i}{2});
  result.correct_blinks = 0;
  result.false_blinks = 0;

  % Determine how many blinks were correctly determined.
  for j = 1:length(edf_files{i}{2})
    % No use checking anything if we have no more blinks to test against.
    if length(blinks) == 0
      break;
    end

    known_blink = edf_files{i}{2}(j);

    % Find the blink index returned by the blink_detect() function that is
    % closest to the known location of a blink.
    lower = max( blinks(blinks <= known_blink) );
    upper = min( blinks(blinks >= known_blink) );

    if length(upper) == 0
      closest = lower;
    elseif length(lower) == 0
      closest = upper;
    else
      if (known_blink - lower) < (upper - known_blink)
        closest = lower;
      else
        closest = upper;
      end
    end

    % Determine if the closest blink is within the error range.
    if abs(closest - known_blink) <= acc_error_samples
      result.correct_blinks = result.correct_blinks + 1;

      % Remove this blink from the list so that we don't count it twice.
      blinks = blinks( blinks ~= closest );
    end
  end

  % Because we removed blinks as we found them, any left over are false
  % detections.
  result.false_blinks = length(blinks);

  % Record the result.
  results{i} = result;
end

% Print some statistics of the results.
blinks = zeros(1,6);
det = zeros(1,6);
false = zeros(1,6);

% Normal EEG. There are 6 normal EEG without blinks, and 6 normal with blinks.
for i = 1:12
  blinks(1) = blinks(1) + length(edf_files{i}{2});
  det(1) = det(1) + results{i}.correct_blinks;
  false(1) = false(1) + results{i}.false_blinks;
end

% Triphasic EEG. There are 6 triphasic EEG without blinks.
for i = 13:18
  blinks(2) = blinks(2) + length(edf_files{i}{2});
  det(2) = det(2) + results{i}.correct_blinks;
  false(2) = false(2) + results{i}.false_blinks;
end

% FIRDA EEG. There are 6 FIRDA EEG without blinks.
for i = 19:24
  blinks(3) = blinks(3) + length(edf_files{i}{2});
  det(3) = det(3) + results{i}.correct_blinks;
  false(3) = false(3) + results{i}.false_blinks;
end

% Simulated normal EEG. There are 6*6 simulated normal EEG.
for i = 25:60
  blinks(4) = blinks(4) + length(edf_files{i}{2});
  det(4) = det(4) + results{i}.correct_blinks;
  false(4) = false(4) + results{i}.false_blinks;
end

% Simulated triphasic EEG. There are 6*6 simulated triphasic EEG.
for i = 61:96
  blinks(5) = blinks(5) + length(edf_files{i}{2});
  det(5) = det(5) + results{i}.correct_blinks;
  false(5) = false(5) + results{i}.false_blinks;
end

% Simulated FIRDA EEG. There are 6*6 simulated FIRDA EEG.
for i = 97:132
  blinks(6) = blinks(6) + length(edf_files{i}{2});
  det(6) = det(6) + results{i}.correct_blinks;
  false(6) = false(6) + results{i}.false_blinks;
end

disp(          '      Type      |  Blinks in | Correctly |   False' );
disp(          '                |     EEG    |   Found   | Detections' );
disp(          '----------------+------------+-----------+-----------' );
disp( sprintf( 'Normal          |     %3d    |    %3d    |    %3d', ...
                blinks(1),det(1),false(1) ) );
disp( sprintf( 'Triphasic       |     %3d    |    %3d    |    %3d', ...
                blinks(2),det(2),false(2) ) );
disp( sprintf( 'FIRDA           |     %3d    |    %3d    |    %3d', ...
                blinks(3),det(3),false(3) ) );
disp( sprintf( 'Sim (normal)    |     %3d    |    %3d    |    %3d', ...
                blinks(4),det(4),false(4) ) );
disp( sprintf( 'Sim (triphasic) |     %3d    |    %3d    |    %3d', ...
                blinks(5),det(5),false(5) ) );
disp( sprintf( 'Sim (FIRDA)     |     %3d    |    %3d    |    %3d', ...
                blinks(6),det(6),false(6) ) );
