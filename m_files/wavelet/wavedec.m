%
%   [C, L] = wavedec( X, N, 'wname' )
%   [C, L] = wavedec( X, N, Lo_D, Hi_D )
%
%   (This function is meant to mimic the MATLAB wavedec function.)
%
%   wavedec() performs one-dimensional deconstruction of the given X vector
%   using either the wavelet specified by 'wname' or the filters provided in
%   Lo_D and Hi_D. See the wfilters() function for a list of valid 'wname'
%   values.
%
%   [C, L] = wavedec( X, N, ... ) returns the appropriate wavelet decomposition
%   of the signal X to decomposition level N. The coefficients are returned in
%   the C vector, while the length of these coefficient vectors is returned in
%   the L vector. For example, for a three level decomposition:
%
%          +-----+-----+---------+-----------------+
%     C -> | cA3 | cD3 |   cD2   |       cD1       |
%          +-----+-----+---------+-----------------+
%
%     L -> [ length(cA3), length(cD3), length(cD2), length(cD1) ]
%
%   If N is larger than the maximum possible value for signal X then the
%   decomposition will stop at whatever the maximum value is (the moment when
%   no more than one detail/difference and one approximate coefficient is
%   extracted). The level of decomposition can be determined by inspecting the
%   L vector.
%
function [C,L] = wavedec( X, N, varargin )

  % Check the number of arguments to see if the user expects us to use the
  % built-in filter values or if they gave their own.
  if nargin == 3
    [ Lo_D, Hi_D ] = wfilters( varargin{1}, 'd' );
  elseif nargin == 4
    Lo_D = varargin{1};
    Hi_D = varargin{2};
  else
    % If an invalid number of arguments was given, error out.
    error( 'Incorrect number of arguments for wavedec( X, N, ... )' );
  end

  % The maximum deconstruction level is related to the length of the input
  % signal and the length of the filters. The filter must be able to fit within
  % the input signal at least twice.
  max_level = floor( log2( length(X) / length(Lo_D) ) );
  if N > max_level
    N = max_level;
  end

  % If N <= 0, we aren't deconstructing anything and our approximation is the
  % signal itself.
  if N <= 0
    C = X;
    L = length(X);
    return
  end

  % The low-pass and high-pass decomposition filters have been built; it's time
  % to use them.

  % Calculate the length of the resulting 'C' vector. Each decomposition level
  % will result in two sets of coefficients, each of length:
  %   g(n) = floor((n - 1) / 2) + M / 2
  % where:
  %   n -> length of input to filters
  %   M -> length of filters (always even)
  % For every decomposition level except for the first and last level, we send
  % one set of those coefficients back through the filters and store the other
  % set by appending it to the 'C' vector. For the last decomp. level we append
  % both sets of coefficients.
  %
  % Thus, the total length of 'C', k, is equal to:
  %   k = g(n_0) + sum( g(n_i), i = 1:N-1 ) + g(n_(N-1))
  % where:
  %   N     -> decomposition level
  %   g(.)  -> as defined above
  %   n_0   -> length of input vector
  %   n_i   -> g( n_(i-1) )
  %
  % TODO: there's probably a way to calculate this without iteration, but the
  %       'floor' function throws me off, and I can't figure it out.
  g = floor( (length(X) - 1) / 2 ) + length( Lo_D ) / 2;
  k = g;

  for i = 1:N-1;
    g = floor( (g - 1) / 2 ) + length( Lo_D ) / 2;
    k = k + g;

    if g == 1
      N = i + 1;
      break
    end
  end
  k = k + g;

  % Initialize the 'C' and 'L' vectors.
  C = zeros( 1, k );
  L = zeros( 1, N + 1 );

  % Figure out each decomposition level after initializing some values.
  cA = X; % Initial input to filters and the zero-order 'approximation' coeffs.
  ri = 0; % Reverse index specifying where to insert calculated detail coeffs.

  for i = 1:N
    % Convole the current input with each filter.
    cD = wconv1( cA, Hi_D ); % The detail/difference values from the high pass.
    cA = wconv1( cA, Lo_D ); % The approximate values come from the low pass.

    % Down sample by taking only even indices.
    cD = cD( 2:2:end );
    cA = cA( 2:2:end );

    % Insert the detail/difference coefficients into the 'C' vector and record
    % the number of those coefficients in the 'L' vector.
    k = length(cD);
    ri = ri + k;
    C( end-ri+1:end-ri+k ) = cD;
    L( end-i+1 ) = k;
  end

  % Insert the final approximation coefficients into the head of the 'C' vector.
  C( 1:length(cA) ) = cA;
  L( 1 ) = length(cA);
end
