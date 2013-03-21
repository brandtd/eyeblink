%
%   [W, A, S, mu_S] = ica( X, ..., 'property', value, ... )
%
%   Performs Independent Component Analysis on the given observations X. Each
%   row of X is assumed to be an observation vector, in other words, each row
%   is a random variable, each column is an observation of that variable.
%
%   This function returns four values:
%     W     - the inverse mixing matrix
%     A     - the mixing matrix
%     S     - the calculated source signals (zero-mean)
%     mu_S  - the calculated source signal means
%
%   This function has several configurable properties, listed in the following
%   table:
%
%    property     |    default  | description
%   --------------+-------------+-----------------------------------------------
%    'method'     |  'fastica'  | Which ICA method to use. Valid values are:
%                 |             |  'fastica'
%                 |             |  'jade'
%                 |             | When using 'jade', all other properties are
%                 |             | ignored.
%                 |             | 
%   --------------+-------------+-----------------------------------------------
%    'epsilon'    |      0.0001 | Convergence criteria. An iterative process is
%                 |             | used to find the inverse mixing matrix, and
%                 |             | this number defines how small a change must be
%                 |             | in the calculated matrix before it is called
%                 |             | 'converged'.
%                 |             | 
%                 |             | Convergence means that the cosine of the angle
%                 |             | between the previous unmixing vectors and the
%                 |             | current vectors is within 'epsilon' of +/- 1.
%   --------------+-------------+-----------------------------------------------
%    'orthog'     | 'symmetric' | Orthogonalization process; either 'deflate' or
%                 |             | 'symmetric'.
%   --------------+-------------+-----------------------------------------------
%    'contrast'   |      'tanh' | The contrast/learning rule that is used to
%                 |             | find the mixing matrix. Valid values that use
%                 |             | negentropy estimation through a nonlinear
%                 |             | function are:
%                 |             |   'tanh'    using g(y) = tanh( y )
%                 |             |   'cube'    using g(y) = y^3
%                 |             |   'gauss'   using g(y) = y * exp( -y^2 / 2 )
%   --------------+-------------+-----------------------------------------------
%    'max_iter'   |        1000 | The maximum number of iterations to perform
%                 |             | before giving up on achieving convergence.
%
%   This function works by assuming that the observations, X, are a linear
%   combination of some unknown source signals, S, represented by the equation:
%
%     X = A * S
%
%   The goal of this function is to find the transform, W, that will yield:
%
%     W * X = S
%
%   Thus, W = A^-1
%
%   To make finding W easier, X is first transformed into a zero-mean set of
%   observations, resulting in a new matrix, represented by:
%     _               _
%     X + mu_X = A * (S + mu_S)
%         _     _
%   Where X and S are both zero mean. We can then ignore both mu_X and mu_S
%   until we have calculated W, at which point it is simple to add them back in.
%                                                          _
%   The S matrix returned by this function is actually the S matrix referenced
%   above. The original observations, X, may be reconstructed by calculating:
%              _
%     X = A * (S + mu_S)
%
function [W, A, S, mu_S] = ica( X, varargin )

  % Setup default values.
  epsilon  = 0.0001;
  orthog   = 'symmetric';
  contrast = @negent_tanh;
  max_iter = 1000;
  use_jade = 0;

  % Extract and verify our arguments.
  for i = 1:2:nargin-1
    if strcmp( varargin{i}, 'method' ) == 1
      if strcmp( varargin{i+1}, 'jade' )
        use_jade = 1;
        break;
      end
    elseif strcmp( varargin{i}, 'epsilon' ) == 1
      % Adjust the default convergence epsilon to the given value.
      epsilon = varargin{i+1};

      if epsilon <= 0
        error( 'Epsilon value, %f, invalid. Must be greater than 0.', epsilon );
      end
    elseif strcmp( varargin{i}, 'orthog' ) == 1
      % Change and verify the specified orthogonalization process.
      orthog = varargin{i+1};

      if strcmp( orthog, 'symmetric' ) ~= 1 && ...
         strcmp( orthog, 'deflate' ) ~= 1
        error( 'Unknown orthogonalization process, ''%s''.', orthog );
      end
    elseif strcmp( varargin{i}, 'contrast' ) == 1
      % Switch which contrast function we use to find the mixing matrix.
      if strcmp( varargin{i+1}, 'tanh' ) == 1
        contrast = @negent_tanh;
      elseif strcmp( varargin{i+1}, 'cube' ) == 1
        contrast = @negent_cube;
      elseif strcmp( varargin{i+1}, 'gauss' ) == 1
        contrast = @negent_gauss;
      else
        error( 'Unknown contrast method, ''%s''.', varargin{i+1} );
      end
    elseif strcmp( varargin{i}, 'max_iter' ) == 1
      if varargin{i+1} > 0
        max_iter = round(varargin{i+1});
      else
        error( 'Invalid maximum number of iterations, %d, must be > 0.', ...
               varargin{i+1} );
      end
    else
      % If we get an unexpected property, error out.
      error( 'Unknown property, ''%s''.', varargin{i} );
    end
  end

  % Switch which ICA method we use.
  if use_jade
    [W,A,S,mu_S] = jade( X );
  else
    [W,A,S,mu_S] = fastica( X, epsilon, orthog, contrast, max_iter );
  end
end
