% Add our wavelet functions to the path.
addpath( fullfile( pwd, 'wavelet' ) );

% Add our ICA functions to the path.
addpath( fullfile( pwd, 'ica' ) );

% Add our blink functions to the path.
addpath( fullfile( pwd, 'blink' ) );

% Add our EDF file functions to the path.
addpath( fullfile( pwd, 'edf' ) );

% Add our EEG specific functions to the path.
addpath( fullfile( pwd, 'eeg' ) );

% Add our test scripts to the path.
addpath( fullfile( pwd, 'test' ) );

% Add our files for handling Pesin's data to the path.
addpath( fullfile( pwd, 'pesin' ) );

% Set the global path variable that lets our wfilters() function find the
% wavelet coefficient files.
global WAVE_COEF_PATH;
WAVE_COEF_PATH = fullfile( pwd, 'wavelet', 'wave_coefs' );
