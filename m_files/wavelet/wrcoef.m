%
%   X = wrcoef( 'type', C, L, 'wname', N )
%   X = wrcoef( 'type', C, L, Lo_R, Hi_R, N )
%   X = wrcoef( 'type', C, L, 'wname' )
%   X = wrcoef( 'type', C, L, Lo_R, Hi_R )
%
%   (This function is meant to mimic the MATLAB wrcoef function.)
%
%   wrcoef() reconstructs the coefficients from a single branch of a
%   one-dimensional wavelet deconstruction given by the C and L vectors.
%
%   The C and L vectors should be the same format as those returned by the
%   wavedec() function.
%
%   The reconstruction filters used are those specified by either the 'wname'
%   parameter or the Lo_R/Hi_R parameters. See wfilters() for a list of valid
%   'wname' values.
%
%   The 'type' parameter specifies whether to reconstruct detail ('type' = 'd')
%   or approximation ('type' = 'a') coefficients.
%
%   The N parameter specifies which level to reconstruct. If 'type' == 'a', then
%   N may equal zero, at which point the original signal is reconstructed.
%   N must always satisfy: N <= length(L) - 1. If N exceeds a boundary, it will
%   be set to the nearest valid value.
%   
function X = wrcoef( type, C, L, varargin )

  % Figure out how we're getting the reconstruction filters and which level we
  % are reconstructing by parsing our argument list.
  if nargin == 4
    [ Lo_R, Hi_R ] = wfilters( varargin{1}, 'r' );
    N = length(L) - 1;
  elseif nargin == 5
    if ischar( varargin{1} )
      [ Lo_R, Hi_R ] = wfilters( varargin{1}, 'r' );
      N = varargin{2};
    else
      Lo_R = varargin{1};
      Hi_R = varargin{2};
      N    = length(L) - 1;
    end
  elseif nargin == 6
    Lo_R = varargin{1};
    Hi_R = varargin{2};
    N    = varargin{3};
  else
    % If an invalid number of arguments was given, error out.
    error( 'Incorrect number of arguments for wrcoef( ''type'', C, L, ... )' );
  end

  % Make sure N is valid. If it's not, set it to the nearest valid value.
  if N > length(L) - 1
    N = length(L) - 1;
  elseif N < 0
    N = 0;
  end

  if (N == 0) && (strcmp(type, 'd'))
    N = 1;
  end

  if strcmp( type, 'a' )
    % We're reconstructing the approximation coefficients. We need to either
    % fetch those from the C vector or reconstruct both the approximation and
    % detail coefficients until we get to the approximation coefficients of the
    % specified level, N.
    cA = C(1:L(1));
    c_index = 1 + L(1);

    k = (length(L) - 1) - N;

    % Perform the full reconstruction until we get to the approxmiation coefs
    % that we're supposed to be reconstructing.
    for i = 1:k
      cD = C(c_index:c_index+L(i + 1)-1);
      c_index = c_index + L(i + 1);
      cA = idwt( cA, cD, Lo_R, Hi_R );
    end
  else
    % We're reconstructing the detail/difference coefficients, so we've got to
    % fetch those from the C vector.
    k = (length(L) - 1) - N;
    c_index = 1 + sum( L(1:k+1) );

    cD = C(c_index:c_index + L(k + 2) - 1);
    cA = zeros( 1, length(cD) );

    cA = idwt( cA, cD, Lo_R, Hi_R );
    k = k + 1;
  end

  % Reconstruct the waveform with the coefficients that we've got.
  for i = k+1:length(L) - 1
    cD = zeros( 1, L(i+1) );
    cA = idwt( cA, cD, Lo_R, Hi_R );
  end

  X = cA;
end
