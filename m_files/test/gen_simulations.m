%   Creates simulated eyeblinks in EEG containing no eyeblink artifact and
%   stores the results in edf format. The process to simulate an eyeblink is as
%   follows:
%
%     1. EEG containing eyeblink artifact is processed using independent
%        component analysis. EEG with no eyeblink artifact is also processed.
%     2. The eyeblink source from the contaminated EEG is appended to the
%        sources for the uncontaminated EEG, and the weight vector for the
%        eyeblink source is appended to the mixing matrix for the uncontaminated
%        EEG.
%     3. The modified source and mixing matrix are used to create a new EEG. The
%        mean value of the eyeblink source is left at zero.
%     4. This modified EEG is saved to a file under the eeg_data/simulated/
%        directory.
%
%   The modified EEG is saved to a file with a filename of the form:
%
%     sim_A_B.edf
%
%   where 'sim_' indicates that the data is simulated, A indicates which EEG
%   dataset the blink source came from, and B indicates which EEG dataset the
%   uncontaminated EEG came from.
%
%   For example, the filename 'sim_n2_t4.edf' indicates that the blink source
%   came from
%     eeg_data/with_blink/normal/normal2.edf
%   and the uncontaminated EEG came from
%     eeg_data/no_blink/triphasic/triphasic4.edf
%   The values used in the filenames can be found by inspecting the first few
%   lines of this script file.
%
%   Due to the number of times ICA is performed in this script, this script will
%   take a few minutes to complete.
%

% Blink EEG files paired with which component contains the eyeblink artifact,
% confirmed via visual inspection, and using the function:
%   ica( eeg, 'epsilon', 0.0001, 'orthog', 'symmetric', 'contrast', 'tanh', ...
%        'max_iter', 1000 )
%
% Each cell contains a filepath to a blink contaminated EEG edf file, the
% independent component containing the eyeblink for that EEG, and a string to be
% used to build the filename of the simulated EEG edf file.
blink_files = { { '../eeg_data/with_blink/normal/normal0.edf',  9, 'n0' },
                { '../eeg_data/with_blink/normal/normal1.edf', 22, 'n1' },
                { '../eeg_data/with_blink/normal/normal2.edf', 23, 'n2' },
                { '../eeg_data/with_blink/normal/normal3.edf', 23, 'n3' },
                { '../eeg_data/with_blink/normal/normal4.edf', 23, 'n4' },
                { '../eeg_data/with_blink/normal/normal5.edf', 23, 'n5' } };

% No blink EEG edf files. The cell values are the same as above, except that
% there's no component number since it's not needed for these guys.
noblink_files = { { '../eeg_data/no_blink/normal/normal0.edf', 'n0' },
                  { '../eeg_data/no_blink/normal/normal1.edf', 'n1' },
                  { '../eeg_data/no_blink/normal/normal2.edf', 'n2' },
                  { '../eeg_data/no_blink/normal/normal3.edf', 'n3' },
                  { '../eeg_data/no_blink/normal/normal4.edf', 'n4' },
                  { '../eeg_data/no_blink/normal/normal5.edf', 'n5' },
                  { '../eeg_data/no_blink/triphasic/triphasic0.edf', 't0' },
                  { '../eeg_data/no_blink/triphasic/triphasic1.edf', 't1' },
                  { '../eeg_data/no_blink/triphasic/triphasic2.edf', 't2' },
                  { '../eeg_data/no_blink/triphasic/triphasic3.edf', 't3' },
                  { '../eeg_data/no_blink/triphasic/triphasic4.edf', 't4' },
                  { '../eeg_data/no_blink/triphasic/triphasic5.edf', 't5' },
                  { '../eeg_data/no_blink/firda/firda0.edf', 'f0' },
                  { '../eeg_data/no_blink/firda/firda1.edf', 'f1' },
                  { '../eeg_data/no_blink/firda/firda2.edf', 'f2' },
                  { '../eeg_data/no_blink/firda/firda3.edf', 'f3' },
                  { '../eeg_data/no_blink/firda/firda4.edf', 'f4' },
                  { '../eeg_data/no_blink/firda/firda5.edf', 'f5' } };

% Mix the blink artifact from each 'blink' file with each 'noblink' file. We
% don't just run ICA an every EEG and then process the results because we may
% not have enough memory to hold that much data.
for b_file = 1:size(blink_files,1)
  bedf = readedf( blink_files{b_file}{1} );

  disp( sprintf('Processing blink file %d out of %d', b_file, ...
        size(blink_files,1)) );
  fflush(1);
  [Wb,Ab,Sb,mu_Sb] = ica( bedf.samples, 'epsilon', 0.0001, ...
                          'orthog', 'symmetric', 'contrast', 'tanh', ...
                          'max_iter', 1000 );

  for nb_file = 1:size(noblink_files,1)
    printf( '  Processing no-blink file %d out of %d\r', nb_file, ...
            size(noblink_files,1) );
    fflush(1);

    nedf = readedf( noblink_files{nb_file}{1} );
    mixed_edf = nedf;

    [Wn,An,Sn,mu_Sn] = ica( nedf.samples, 'epsilon', 0.0001, ...
                            'orthog', 'symmetric', 'contrast', 'tanh', ...
                            'max_iter', 1000 );

    % Pull out the blink source and weight vector from the blink-contaminated
    % EEG and append it to the sources and mixing matrix obtained for the
    % 'clean' EEG.
    Sn(26,:) = Sb( blink_files{b_file}{2},: );
    An(:,26) = Ab( :,blink_files{b_file}{2} );

    % Set the mean for the blink source to zero.
    mu_Sn(26,:) = zeros(1,length(mu_Sn));

    % Create a new EEG that mixes in the blink source with the 'clean' EEG to
    % create a new 'blink contaminated' EEG recording.
    mixed_edf.samples = An * (Sn + mu_Sn);
    mixed_edf.phys_max = round(max(mixed_edf.samples'))';
    mixed_edf.phys_min = round(min(mixed_edf.samples'))';

    % Modify the header information of the EDF data to indicate that this is a
    % simulation.
    mixed_edf.patient_id = sprintf( ...
              'Simulated Blink. Blink source: ''%s''. Clean source: ''%s''', ...
              blink_files{b_file}{3}, noblink_files{nb_file}{2} );
    mixed_edf.patient_id = sprintf( '%-80s', mixed_edf.patient_id );
    mixed_edf.recording_id = mixed_edf.patient_id;
    mixed_edf.start_date = strftime('%d.%m.%y', localtime(time()));
    mixed_edf.start_time = strftime('%H.%M.%S', localtime(time()));

    % Build a filename for the simulated EEG edf file and save the EEG to file.
    filename = sprintf( '../eeg_data/simulated/sim_%s_%s.edf', ...
                        blink_files{b_file}{3}, ...
                        noblink_files{nb_file}{2} );
    saveedf( mixed_edf, filename );
  end
  printf('\n');
  fflush(1);
end
