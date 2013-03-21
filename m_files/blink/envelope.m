%
%   Y = envelope( X )
%
%   Finds a signal that envelopes X from above. This function works by finding
%   the points in X where slope goes from positive to non-positive and then
%   linearly interpolates between those points using interp1( ..., 'linear' ).
%
function Y = envelope( X )

  % The find statement will find all points where slope goes from positive
  % to zero, positive to negative, and zero to negative. The first and last
  % indices are included in the interpolation so that Y has the same length as
  % X. (If they are not, the beginning and end of Y are filled with the NA
  % value).
  i = [ 1 (find( diff(sign(diff( X ))) < 0 ) + 1) length(X) ];
  Y = interp1( i, X(i), 1:length(X), 'linear' );
end
