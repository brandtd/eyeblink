%
%   [Lo_D, Hi_D, Lo_R, Hi_R = wfilters( 'wname' )
%   [F1, F2] = wfilters( 'wname', 'type' )
%
%   (This function is meant to mimic the MATLAB wfilters function.)
%
%   [Lo_D, Hi_D, Lo_R, Hi_R = wfilters( 'wname' ) returns the four filters
%   associated with the orthogonal or biorthogonal wavelet named in the string
%   'wname'.
%
%   The four filters are:
%   + Lo_D, the decomposition low-pass filter
%   + Hi_D, the decomposition high-pass filter
%   + Lo_R, the reconstruction low-pass filter
%   + Hi_R, the reconstruction high-pass filter
%
% Available wavelet names are:
% +-----------------+---------------------------------------------------------+
% | Daubechies      |   'haar'/'db1', 'db2', ..., 'db20'                      |
% +-----------------+---------------------------------------------------------+
% | Coiflets        |   'coif1', ..., 'coif5'                                 |
% +-----------------+---------------------------------------------------------+
% | Symlets         |   'sym2', ..., 'sym20'                                  |
% +-----------------+---------------------------------------------------------+
% | Discrete Meyer  |   'dmey'                                                |
% +-----------------+---------------------------------------------------------+
% | Biorthogonal    |   'bior1.1', 'bior1.3', 'bior1.5',                      |
% |                 |   'bior2.2', 'bior2.4', 'bior2.6', 'bior2.8',           |
% |                 |   'bior3.1', 'bior3.3', 'bior3.5', 'bior3.7', 'bior3.9',|
% |                 |   'bior4.4', 'bior5.5', 'bior6.8'                       |
% +-----------------+---------------------------------------------------------+
%
% 
%   [F1, F2] = wfilters( 'wname', 'type' ) returns the following filters:
%
%   Lo_D and Hi_D (Decomposition filters)   if 'type' = 'd'
%   Lo_R and Hi_R (Reconstruction filters)  if 'type' = 'r'
%   Lo_D and Lo_R (Low-pass filters)        if 'type' = 'l'
%   Hi_D and Hi_R (High-pass filters)       if 'type' = 'h'
%
function varargout = wfilters( varargin )

  % Declare a global that will let us find the location of the wavelet
  % coefficient files. This value MUST be set somewhere else.
  % TODO: This is a weak solution.
  global WAVE_COEF_PATH;

  % Verify input/output parameter counts.
  if ~(nargout == 2 && nargin == 2) && ~(nargout == 4 && nargin == 1)
    error( 'Invalid input/output parameter list for wfilters' );
  end

  % Build the string that points to the m file containing the filter
  % coefficients.
  if strcmp( varargin{1}, 'haar' ) == 1
    run_str = fullfile( WAVE_COEF_PATH, 'db1.m' );
  else
    if strncmp( varargin{1}, 'bior', 4 ) == 1
      varargin{1}(6) = '_';
    end

    run_str = fullfile( WAVE_COEF_PATH, strcat( varargin{1}, '.m' ) );
  end

  % Verify that a valid wavelet was specified by checking that its wavelet file
  % exists.
  if ~exist( run_str )
    error( 'Invalid wavelet `"%s"''  specified.', varargin{1} );
  end

  % Fetch the filter coefficients.
  run( run_str )

  % Return coefficients according to how the function was called.
  if nargout == 4
    varargout{1} = Lo_D;
    varargout{2} = Hi_D;
    varargout{3} = Lo_R;
    varargout{4} = Hi_R;
  else
    switch varargin{2}
      case 'd'
        varargout{1} = Lo_D;
        varargout{2} = Hi_D;
      case 'r'
        varargout{1} = Lo_R;
        varargout{2} = Hi_R;
      case 'l'
        varargout{1} = Lo_D;
        varargout{2} = Lo_R;
      case 'h'
        varargout{1} = Hi_D;
        varargout{2} = Hi_R;
      otherwise
        error( 'Invalid type, `"%s"'' specified.', type );
    end
  end
end
