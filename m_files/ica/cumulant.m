%
%   Q = cumulant( X, M )
%
%   Generates a cumulant matrix, Q, for the observations in X according to the
%   the matrix M and the equation for Q, defined entry-wise as:
%
%                        n
%     Q_X(M) <=> q_ij = sum( cum( X_i, X_j, X_k, X_l ) * m_kl ), 1 <= i,j <= n
%                      k,l = 1
%
%   where `cum( a,b,c,d )' is the fourth-order cumulant of variables a,b,c,d,
%   defined as
%
%     cum( a, b, c, d ) = E{ a * b * c * d } - ...
%                         E{ a * b } * E{ c * d } - ...
%                         E{ a * c } * E{ b * d } - ...
%                         E{ a * d } * E{ b * c }
%
%   where E{.} is the expectation operator, and assuming that the a,b,c,d
%   variables are zero-mean.
%
%   Fitting with the above equations, the X matrix is expected to contain a set
%   of zero-mean variables, where each row of X represents a variables, and each
%   column of X represents an observation of those variables.
%
%   The M matrix is expected to be a square matrix with the same number of rows
%   as X has variables.
%
%   NOTE: This function is provided as a reference implementation -- it is very
%   slow for larger matrices.
%
function Q = cumulant( X, M )
  n = size(M,1);

  Q = zeros( size(M) );

  for i = 1:n
    for j = 1:n
      accum = 0;

      for k = 1:n
        for l = 1:n
          q = mean( X(i,:) .* X(j,:) .* X(k,:) .* X(l,:) ) - ...
              mean( X(i,:) .* X(j,:) ) * mean( X(k,:) .* X(l,:) ) - ...
              mean( X(i,:) .* X(k,:) ) * mean( X(j,:) .* X(l,:) ) - ...
              mean( X(i,:) .* X(l,:) ) * mean( X(j,:) .* X(k,:) );
          q = q * M(k,l);
          accum = accum + q;
        end
      end

      Q(i,j) = accum;
    end
  end
end
