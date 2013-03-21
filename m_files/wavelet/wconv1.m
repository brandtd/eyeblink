%
%   Y = wconv1( X, F )
%
%   (This function is meant to mimic the MATLAB wconv1() function.)
%
%   Similar to conv(X,F), but reduces edge artifacts by padding the boundaries
%   of X. X is padded symmetrically (using padarray( ..., 'symmetric' )).
%
%   The extra values due to the padding are removed from the return value,
%   making:
%     length(Y) = length(X) + length(F) - 1;
%   identical to the behavior of conv().
%
function Y = wconv1( X, F )

  % TODO: to truly be like the MATLAB function, we must also check the current
  %       DWTMODE, as many other padding modes are valid as well.
  l_pad = length(F) - 1;

  % Pad the signal vector on both ends.
  X = [ fliplr(X(2:l_pad+1)) X fliplr(X(end-l_pad:end-1)) ];

  % Convolve the signal vector with the filter and then remove the additional
  % data due to padding from the beginning and end of the resulting vector.
  Y = conv( X, F );
  Y = Y(l_pad + 1:end - l_pad);
end
