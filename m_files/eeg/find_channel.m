%
%   chan_i = find_channel( edf, 'chan' )
%
%   This function, given an edf struct such as the one returned from the
%   readedf() function, will find the index of the channel channel specified by
%   'chan'. Before searching, the 'chan' parameter and the channel labels given
%   in the edf struct are processed with the strtrim() function to remove excess
%   whitespace.
%
%   If the specified channel does not exist, this function returns a zero.
function chan_i = find_channel( edf, chan )
  chan = strtrim( chan );

  chan_i = 0;
  for i = 1:size(edf.labels,1)
    if strcmp(strtrim(edf.labels(i,:)), chan)
      chan_i = i;
      break;
    end
  end
end
