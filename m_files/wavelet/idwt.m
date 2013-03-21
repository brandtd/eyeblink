%
%   X = idwt( cA, cD, 'wname' )
%   X = idwt( cA, cD, Lo_R, Hi_R )
%
%   (This function is meant to mimic the MATLAB idwt function.)
%
%   idwt() performs a single-level inverse discrete wavelet transform using
%   either a specified wavelet or given reconstruction filters.
%
%   cA and cD should be the approximation and detail/difference coefficients,
%   respectively.
%
%   See the wfilters() function for a list of valid 'wname' values.
%
%   With respect to MATLAB's idwt() function, this function uses the symmetric-
%   padding mode when performing its convolutions.
%
function X = idwt( cA, cD, varargin )

  % Check the number of arguments to see how we'll be getting the reconstruction
  % filters.
  if nargin == 3
    [ Lo_R, Hi_R ] = wfilters( varargin{1}, 'r' );
  elseif nargin == 4
    Lo_R = varargin{1};
    Hi_R = varargin{2};
  else
    % If given an invalid number of arguments, error out.
    error( 'Incorrect number of arguments for idwt( cA, cD, ... )' );
  end

  % Make sure the approximation and detail coefficient vectors are of the same
  % length. The reconstruction process will sometimes make the approximation
  % coefficients one value longer. We simply drop this value.
  if length(cA) == length(cD) + 1
    cA = cA(1:length(cD));
  end

  % Upsample the coefficient vectors, moving all values to an even index and
  % inserting zeros at odd indices.
  cA = dyadup( cA );
  cD = dyadup( cD );

  % Filter the upsampled coefficients and add them together, then get rid of the
  % edge artifacts.
  cA = wconv1( cA, Lo_R ) + wconv1( cD, Hi_R );
  cA = cA( length(Lo_R):end - length(Lo_R) + 2 );

  X = cA;
end
