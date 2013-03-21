%
%   [W, A, S, mu_S] = fastica( X, epsilon, 'orthog', @contrast, max_iter )
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
%   This function takes five parameters:
%     X         - the observations
%     epsilon   - convergence criteria
%     'orthog'  - which ortogonalization process to use
%     @contrast - the contrast/learning rule used
%     max_iter  - the maximum number of iterations to perform
%
%   These parameters and their valid values are explained by the ica() function.
%
%   This function is only meant to be called by the ica() function.
%
function [W, A, S, mu_S] = fastica( X, epsilon, orthog, contrast, max_iter )

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
  whiten_matrix = inv( real(sqrt(D)) ) * E';
  dewhiten_matrix = E * real( sqrt(D) );
  Z = whiten_matrix * Z;

  % Initialize the unmixing matrix.
  W = E;

  % Check which type of orthogonalization we have been told to use when finding
  % the unmixing matrix rows.
  if strcmp( orthog, 'deflate' ) == 1
    % We're using deflationary orthogonalization.

    % Find a mixing vector for each independent component (each observation
    % vector).
    for p = 1:size(X,1)
      w = W(p,:);
      w_prev = zeros(size(w));

      n_i = 0; % Number of iterations.

      % Keep searching until the vector converges. Because the w vectors are
      % unit length, we can use the dot product to determine their cosine.
      while 1 - abs(w * w_prev') > epsilon && n_i < max_iter
        n_i    = n_i + 1;
        w_prev = w;

        % Update w using the contrast function.
        w = contrast( w, Z );

        % Orthogonalize and normalize w with respect to all previously found
        % vectors.
        w = w - w * W(1:p-1,:)' * W(1:p-1,:);
        w = w / norm(w);
      end

      W(p,:) = w;
    end
  elseif strcmp( orthog, 'symmetric' ) == 1
    % We're using a symmetric orthogonalization process.
    
    W_prev = zeros(size(W));

    n_i = 0; % Number of iterations.

    % Keep searching until we reach our convergence criteria. Because the w
    % vectors are unit length, we can use dot products to get cosine values.
    while 1 - min(abs(diag(W * W_prev'))) > epsilon && n_i < max_iter
      n_i    = n_i + 1;
      W_prev = W;

      % Apply the step algorithm to the unmixing matrix.
      W = contrast(W,Z);

      % Orthogonalize the matrix (symmetrically).

      % The commented out 'simpler' method consistently creates a very slight
      % performance (i.e. hundreths of seconds added) hit.
%      e = sum(sum(abs(W*W' - eye(size(W)))));
%      while e > 0.0001
%        W = W / norm(W);
%        W = 3/2 * W - 1/2 * W * W' * W;
%        e = sum(sum(abs(W*W' - eye(size(W)))));
%      end
      W = real(inv(W * W')^(1/2)) * W;
    end
  end

  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  % We now have a best guess at an unmixing matrix. We need to finish up our
  % computations by computing the source signals, the mixing matrix, the
  % unmixing matrix that will unmix the original, nonwhitened observations, and
  % we need to compute the means of the source signals.
  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  S = W * Z;
  A = dewhiten_matrix * W';
  W = W * whiten_matrix;

  % Recreate the source signal means.
  mu_S = W * mu_X;
end
