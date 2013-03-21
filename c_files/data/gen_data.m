% Generate sets of observation data and the mixing, unmixing, and source
% matrices and source signal means that should be extracted from those
% observations using ICA.
%
% This will allow us to verify not that the C implementation of ICA is correct,
% but that it gives the same results as the MATLAB implementation.
cd ../../m_files/   % Get the ICA library.
runfirst;

X = {}; Sa = {}; Aa = {}; Wa = {}; mu_Sa = {};
contrast_rules = { 'tanh', 'cube', 'gauss' };
n = 1;  % The index we use for the above cells.

% Generate some test source signals, mix them, and then record that mixture
% and what our MATLAB implementation of ICA gives us.
i = 0:0.0002:0.9999;
S = [ sin( i * 20 * pi );
      cos( i * 4 * pi ) - 32;
      cos( i * 399 * pi ) + 6;
      cos( i * pi ) - 3;
      sawtooth( i * 32 * pi ) + 10;
      sawtooth( i * 15.23 * pi ) + 10;
      rand( 1, length(i) ) - 32;
      rand( 1, length(i) ) * 71 - 2;
      rande( 1, length(i) ) + 6;
      rande( 1, length(i) ) * 13 - 20; ];

A = zeros( size(S,1) );
while det(A) < 0.0001
  A = 4 * rand( size(S,1) );
end

% Find the expected results using all three of our contrast rules.
for i = 1:3
  X{n} = A * S;
  [Wa{n}, Aa{n}, Sa{n}, mu_Sa{n}] = ica( X{n}, 'max_iter', 400, ...
                                         'contrast', contrast_rules{i} );
  n += 1;
end

% Generate a set of gaussian random variables so that we can see worst case
% performance.
S = randn( 25, 5000 );
A = zeros( size(S,1) );
while det(A) < 0.0001
  A = rand( size(S,1) );
end

% Find the expected results using all three of our contrast rules.
for i = 1:3
  X{n} = A * S;
  [Wa{n}, Aa{n}, Sa{n}, mu_Sa{n}] = ica( X{n}, 'max_iter', 400, ...
                                         'contrast', contrast_rules{i} );
  n += 1;
end

%% Finally, process some real EEG data.
%edf = readedf('../eeg_data/with_blink/normal/normal0.edf' );
%% Find the expected results using all three of our contrast rules.
%for i = 1:3
%  X{n} = edf.samples;
%  [Wa{n}, Aa{n}, Sa{n}, mu_Sa{n}] = ica( X{n}, 'max_iter', 400, ...
%                                         'contrast', contrast_rules{i} );
%  n += 1;
%end

% Now that we've got all our data, save it to files that we can open and
% process in C.
cd ../c_files/data

% Save everything in column major format.
for i = 1:length(X)
  csvwrite( sprintf( 'x%d.csv', i ), X{i}' );
  csvwrite( sprintf( 'w%d.csv', i ), Wa{i}' );
  csvwrite( sprintf( 'a%d.csv', i ), Aa{i}' );
  csvwrite( sprintf( 's%d.csv', i ), Sa{i}' );
  csvwrite( sprintf( 'mu%d.csv', i ), mu_Sa{i}' );
end
