%
%   X = waverec( C, L, 'wname' )
%   X = waverec( C, L, Lo_R, Hi_R )
%
%   (This function is meant to mimic the MATLAB waverec function.)
%
%   waverec() performs a one-dimensional reconstruction of the given wavelet
%   coefficients in C using either the reconstruction filters given or those
%   specified by 'wname'. See the wfilters() function for a list of valid
%   'wname' values.
%
%   The C and L vectors should be the same format as those returned by the
%   wavedec() function.
%
function X = waverec( C, L, varargin )

  % Check the number of arguments to see if the user expects us to use the
  % built-in filter values or if they gave their own.
  if nargin == 3
    [ Lo_R, Hi_R ] = wfilters( varargin{1}, 'r' );
  elseif nargin == 4
    Lo_R = varargin{1};
    Hi_R = varargin{2};
  else
    % If given an invalid number of arguments, error out.
    error( 'Incorrect number of arguments for waverec( C, L, ... )' );
  end

  % We've got our low and high pass reconstruction filters. Time to use them.

  % Fetch the first set of approximation coefficients and initialize the index
  % into the C vector.
  cA = C(1:L(1));
  i  = 1 + L(1);

  for j = 2:length(L)
    % Fetch the detail/difference coefficients and update the index used to get
    % those coefficients.
    cD = C(i:i+L(j)-1);
    i  = i + L(j);

    % Transform the coefficients.
    cA = idwt( cA, cD, Lo_R, Hi_R );
  end

  X = cA;
end
