%
%   I = threshdet( X, T, nS )
%
%   threshdet() find the locations within X where abs(X) exceeds the threshold,
%   T within windows of size, nS. All locations where abs(X) exceeds T within
%   nS samples are grouped together and their average index is recorded as the
%   location where X exceeded the threshold.
%
%   Example:
%
%   If abs(X) exceeds T at indices [ i1 i2 i3 i4 i5 ] and:
%     i2 - i1 < nS
%     i3 - i2 < nS
%     i4 - i3 > ns
%     i5 - i4 < ns
%   then two groups will be formed, g1 = [ i1 i2 i3 ]; g2 = [ i4 i5 ], and
%   the returned indices will be equal to:
%     I = [ round(mean( g1 )), round(mean( g2 )) ]
%
%   This is true even if i3 - i1 > nS.
%
%function I = threshdet( X, T, nS )
%function I = threshdet( X, nS )
function I = threshdet( X, mu, sig, nS )
  
%  i_thresh = find( abs(X) > T );
%i_thresh = find( abs( (X - mean(X)) ) > 2*std(X) );
i_thresh = find( abs(X - mu) > 3 * sig );

  if length( i_thresh ) == 0
    I = [];
    return
  end

  % Find the places where the difference in index is greater than the specified
  % number of samples.
  i_splits = find(diff(i_thresh) > nS);

  % Initialize the vector that will contain the computed indices. We already
  % know that there is at least one threshold crossing, and that the last
  % crossing will not have crossings more than nS samples away (or it wouldn't
  % be the last crossing), so we know the number of crossings is equal to the
  % number of crossings more than nS samples away, plus 1.
  I = zeros(1,length(i_splits) + 1);

  % Compute the mean location of threshold crossings. We need to handle the
  % last location differently, because it won't have an entry in the i_splits
  % list.
  k = 1;
  for i = 1:length(i_splits)
    I(i) = round( mean( i_thresh(k:i_splits(i)) ) );
    k = i_splits(i) + 1;
  end
  I(end) = round( mean( i_thresh(k:end) ) );
end
