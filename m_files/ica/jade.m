%
%   [W, A, S, mu_S] = jade( X )
%
%   Performs Independent Component Analysis on the given observations in X using
%   the Joint Approximate Diagonalization of Eigenmatrices (JADE) algorithm.
%   Each row of X is assumed to be an observation vector, in other words, each
%   row is a random variable, each column is an observation of that variable.
%
%   This function returns four values:
%     W     - the unmixing matrix
%     A     - the mixing matrix
%     S     - the calculated source signals (zero-mean)
%     mu_S  - the calculated source signal means
%
function [W, A, S, mu_S] = jade( X )

  % This implementation is almost an exact copy of the implementation offered by
  % Jean-Francois Cardoso, available at:
  %     http://perso.telecom-paristech.fr/~cardoso/Algo/Jade/jadeR.m
  % (as of March 09, 2010).

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Make the observations zero-mean, remembering their means so that we can
  % calculate the source signals means later.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  mu_X = repmat( mean(X')', 1, length(X) );
  Z = X - mu_X;

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Make the zero-mean observations white.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  % To whiten the observations, we find the eigenvalue decomposition of the
  % zero-mean observations' covariance matrix. This lets us compute whitening
  % and dewhitening matrices. The whitening matrix will convert our zero-mean
  % observations into uncorrelated, unit variance variables. The dewhitening
  % matrix will be useful for calculating the inverse unmixing matrix later on.
  %
  % The whitening and dewhitening matrices are equal to:
  %   white   = D^(-1/2) * E'
  %   dewhite = E * D^(1/2)
  % where D is the diagonal matrix of eigenvalues and E' is the transpose of the
  % eigenvector matrix, both coming from the covariance matrix of the zero-mean
  % observations.
  [E,D] = eig(cov(Z'));
  whiten_matrix   = inv( real( sqrt(D) ) ) * E';
  dewhiten_matrix = E * real( sqrt(D) );
  Z = whiten_matrix * Z;

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Form the cumulant matrices.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  [n, T] = size(X);
  num_cm = (n * (n + 1)) / 2;

  CM = zeros(n,n*num_cm);

  scale = ones(n,1) / T;
  R     = eye(n) - 1 + 1;
  range = 1:n;

  for i = 1:n
    Zi = Z(i,:);

    Qiikl = ((scale * (Zi .* Zi)) .* Z) * Z' - R - 2 * R(:,i) * R(i,:);
    CM(:,range) = Qiikl;

    range = range + n;

    for j = 1:i-1
      Zj = Z(j,:);
      Qijkl = ((scale * (Zi .* Zj)) .* Z) * Z' - ...
               R(:,i) * R(j,:) - R(:,j) * R(i,:);
      CM(:,range) = Qijkl;

      range = range + n;
    end
  end

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % Begin performing Jacobi sweeps in an attempt to diagonalize all cumulant
  % matrices simultaneously.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  % Setup a minimum rotation angle (all rotation must be greater than this
  % angle). This minimum rotation, 'thresh', is meant to prevent us from
  % rotating when the rotation will not be 'statistically relevant'.
  thresh   = (1 / sqrt(T)) / 100;
  V        = eye(n);  % The rotation matrix, updated after every rotation.
  modified = 1;       % Flag indicating a rotation was performed this sweep.
  sweep    = 0;       % The sweep iteration counter.

  % Run a maximum of 100 Jacobi sweeps in our attempt to diagonalize all of the
  % cumulant matrices.
  while sweep < 100 && modified
    modified = 0;
    sweep    = sweep + 1;

    % Attempt to zero out all off diagonal elements. The cumulant matrices are
    % all symmetric, so we only need to work on either the upper or lower
    % triangle of the matrices. We arbitrarily choose the upper.
    for p = 1:n-1
      for q = p+1:n
        % The core of this double loop attempts to minimize the p,q'th element
        % in each cumulant matrix, it takes a whole lot of algebra and
        % trigonometry to prove that the calculations below yield the optimal
        % rotation angle--too much to put into these comments.

        i_p = p:n:n*num_cm;
        i_q = q:n:n*num_cm;

        G = [ CM(p,i_p) - CM(q,i_q);
              CM(p,i_q) + CM(q,i_p) ];
        G = G * G';

        a = G(1,1) - G(2,2);
        b = G(1,2) + G(2,1);

        theta = 1/2 * atan2( b, a + sqrt( a*a + b*b ) );

        if abs(theta) > thresh
          % If the rotation angle is greater than our 'statisically relevant'
          % threshold, then the rotation should be performed. A Jacobi rotation
          % will only modify the p,q'th rows and p,q'th columns of a matrix,
          % so we save ourselves a bunch of computations by not using matrix
          % multiplications, and only operating on those values.
          modified = 1;

          c = cos(theta);
          s = sin(theta);

          G = [ c -s;
                s  c ];

          % Update the concatenation of all rotations.
          V(:,[p q]) = V(:,[p q]) * G;

          % Calculate (G' * Q * G) for all cumulant matrices, Q, stored in CM.
          CM([p q],:) = G' * CM([p q],:);
          CM(:,[i_p i_q]) = [ c * CM(:,i_p) + s * CM(:,i_q), ...
                             -s * CM(:,i_p) + c * CM(:,i_q) ];
        end
      end
    end
  end

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % We now have a best guess at an unmixing matrix. We need to finish up our
  % computations by computing the source signals, the mixing matrix, the
  % unmixing matrix that will unmix the original, nonwhitened observations, and
  % we need to compute the means of the source signals.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  S = V' * Z;
  W = V' * whiten_matrix;
  A = dewhiten_matrix * V;

  % Recreate the source signal means.
  mu_S = W * mu_X;

sweep
end
