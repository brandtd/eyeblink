%
%   Y = dyadup( X )
%
%   (This function is meant to mimic the MATLAB dyadup function, but currently
%   only performs the 'odd' mode (inserts zeros at odd indices) and only handles
%   row vectors.)
%
%   dyadup() upsamples the given X vector by creating a vector Y equal to:
%            { x(k/2),  k even
%     Y(k) = {                    , 1 <= k <= 2*length(x)
%            { 0,       k odd
%
%   The length of Y will always be twice the length of X.
%
function Y = dyadup( X )
  Y = zeros(1,length(X)*2);
  Y(2:2:end) = X;
end
