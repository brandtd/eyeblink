%
%   i_blinks = blinks_in_source( S, I, fs )
%
%   Find the temporal location of blinks in the source signal, `S', by
%   thresholding the signal with a threshold that is dependent on the number of
%   expected blinks.
%
%   The treshold cannot be fixed because the signal will be unit variance due to
%   the ICA computation, so the more blinks in the signal, the smaller each
%   individual blink.
%
%   The `I' parameter should be the location of blinks as returned by the
%   blink_detect() function, and the `fs' parameter should be the sample
%   frequency of the EEG recording.
%
function i_blinks = blinks_in_source( S, I, fs )
  % Setup the threshold. This was experimentally determined and seems to have
  % good results.
  thresh = 10 / length(I);

  % Find points where the source crosses the threshold.
  cross = find(diff(abs(S) > thresh) ~= 0);

  % We only want rising/falling edge pairs that cross the threshold, so make
  % sure that the first crossing we see is a rising edge.
  if abs(S) > thresh
    cross = cross(2,:);
  end

  % Make sure we have an even number of crossings (because we only want pairs).
  if (mod(length(cross),2) ~= 0)
    cross = cross(1:end-1);
  end

  % Find where the absolute value of the source signal is maximum between the
  % crossing points.
  maxs = zeros(1,length(cross)/2);
  for i = 1:2:length(cross)
    [unused, maxs((i+1)/2)] = max(abs(S(cross(i):cross(i+1))));
    maxs((i+1)/2) = maxs((i+1)/2) + cross(i) - 1;
  end

  % The maximums that we've found correspond to blink locations within the
  % blink source. Eliminate all locations that do not agree within 0.02 seconds
  % with the blinks detected in the original EEG.

  window = round(0.02 * fs);    % 0.02 seconds in samples.

  blinks_iter = 1;
  for i = 1:length(maxs)
    blink = maxs(i);

    found = false;    % Reset the 'found a match' flag.

    for j = blinks_iter:length(I)
      if abs(I(j) - blink) <= window
        % Update the the blinks_iter iterator so taht we don't double count any
        % blinks, set the 'found a match' flag, and then break out since we
        % already found a match.
        blinks_iter = j+1;
        found = true;
        break;
      end
    end

    % If no match was found mark the blink as invalid and keep checking.
    if ~found
      maxs(i) = -1;
    end
  end

  % Remove all the unmatched blinks.
  i_blinks = maxs(maxs ~= -1);
end
