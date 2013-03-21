%
%   Y = diff_channels( edf, 'left', 'right' )
%
%   This function, given an edf struct such as the one returned from the
%   readedf() function, will compute the channel given by subtracting the
%   channel specified by 'right' from the channel specified by 'left'.
%
%   For example, to compute the `FP1 - F3' channel, this function could be
%   called like:
%
%     fp1_f3 = diff_channels( edf, 'FP1', 'F3' )
%
%   for an edf struct containing channels labeled 'FP1' and 'F3'. Before
%   comparison, all channel labels are processed with the strtrim() function to
%   remove excess whitespace.
%
%   If the specified channels do not exist, this function returns a zero-length
%   vector.
function Y = diff_channels( edf, left, right )
  left  = strtrim( left );
  right = strtrim( right );

  l_i = r_i = 0;
  for i = 1:size(edf.labels,1)
    if strcmp(strtrim(edf.labels(i,:)), left)
      l_i = i;
    elseif strcmp(strtrim(edf.labels(i,:)), right)
      r_i = i;
    end

    if (l_i ~= 0 && r_i ~= 0)
      break;
    end
  end

  if l_i == 0 || r_i ==0 
    Y = [];
  else
    Y = edf.samples(l_i,:) - edf.samples(r_i,:);
  end
end
