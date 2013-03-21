%
%   I = blink_detect( channels )
%   I = blink_detect( ..., property, value, ... )
%
%   blink_detect() attempts to locate eyeblinks with the given set of EEG
%   channels. 
%
%   The full blink detection algorithm has the following properties which may be
%   configured:
%
%   Properties  | Default value | Meaning
%   ------------+---------------+--------------------------------------------
%     'fs'      | 500           | Sampling frequency of EEG in HZ.
%     't1'      | 15            | Threshold to use in single channel detection.
%     't_cor'   | 0.75          | Correlation threshold.
%     'plot'    | 0             | Which figure to plot results to (0 -> no plot)
%
%   blink_detect() returns a row vector, I, containing the indices of the
%   estimated eyeblink locations within the EOG.
%
%   The basic operation of this function is:
%     1. Process each channel individually, finding possible blink locations in
%        each of the channels.
%     2. For every possible blink location found in the first channel, see if
%        all other channels have possible blinks within 0.02 seconds of the
%        blink found in the first channel.
%       2a. If all channels have a blink at the location, record it is an
%           actual blink location.
%       2b. If the possible blink is not found in all channels, discard it.
%     3. Return the list of recorded blink locations.
%
%   The process to find possible blinks within a single channel is:
%     1. Sum the upper envelope of the absolute values of the 5th, 6th, and 7th
%        order coiflet3 wavelet details of the channel.
%     2. Calculate a moving average of the summed details. The moving average
%        is calculated as the average of a 512 sample window.
%     3. Create a threshold equal to the moving average plus 't1' ('t1' defaults
%        to 15).
%     4. Find points in the details that cross the threshold and group them in
%        pairs so that the first point is a rising edge and the second is a
%        falling edge. Discard points that have no corresponding rising/falling
%        edge.
%     5. Between each point pair, find the point at which the 3rd order coiflet3
%        wavelet approximation of the channel is minimum. These points are the
%        potential eyeblink locations.
%     6. For each potential eyeblink location, correlate the 3rd order coiflet3
%        wavelet approximation of the channel around that point with a template
%        that approximates the shape of an eyeblink. Wliminate any point whose
%        correlation coefficient is below the threshold 't_cor' ('t_cor'
%        defaults to 0.75).
%     7. The potential eyeblink locations that remain are returned as likely
%        blink locations.
%
%   The purpose of step 1 in the single channel processing algorithm is to
%   apply a filter that emphasizes features of the eyeblink.
%
%   Steps 2 and 3 allow us to create a sort of adaptive threshold that makes the
%   processing more robust to different people and any bursts of activity or
%   slow level drifts in the channel.
%
%   Steps 4 and 5 significantly reduce the amount of work for us to do when we
%   perform the template matching in step 6.
%
%   Step 6 is a type of template matching that helps eliminate any signals that
%   have the same time-frequency content as an eyeblink, but do not have its
%   characteristic shape and sign.
%     
function I = blink_detect( channels, varargin )

  % Setup default values.
  fs       = 500;
  plot_fig = 0;
  t1       = 15;
  t_cor    = 0.75;

  % Parse our arguments to see if we've been given any specific properties.
  for i = 1:2:length(varargin)
    switch varargin{i}
      case 'fs'
        fs       = varargin{i+1};
      case 'plot'
        plot_fig = varargin{i+1};
      case 't1'
        t1       = varargin{i+1};
      case 't_cor'
        t_cor    = varargin{i+1};
      otherwise
        % If given an invalid property, error out.
        error( 'Invalid property, `%s'' given to blink_detect.', varargin{i} );
    end
  end

  % Find the locations of blinks according to each individual channel.
  pos_blinks = {};
  for i = 1:size(channels,1)
    pos_blinks{i} = find_possible_blinks( channels(i,:), fs, t1, t_cor );
  end

  % Look through the first set of possible blink locations. For a blink to
  % be considered valid, its index must be seen in all channels, within 0.02
  % seconds of the each other. 

  % Setup an iterator for each channel.
  c_iters = ones( size(channels,1 ) );

  % Figure out what 0.02 seconds is in samples.
  window = round(0.02 * fs);

  I = pos_blinks{1};

  for i_blink = 1:length(I)
    test_blink = I(i_blink);

    for chan = 2:size(channels,1)
      found = false;
      for i = c_iters(chan):size( pos_blinks{chan}, 2 )

        % If the blink found in channel `chan' is within `window' samples of
        % `test_blink', then we call them a match.
        if abs(pos_blinks{chan}(i) - test_blink) <= window
          % Update the matched channel's iterator so that we don't double count 
          % any blinks.
          c_iters(chan) = i+1;

          % Set the `found' flag and break out since we've already matched.
          found = true;
          break;
        end
      end

      % If no match was found then there's no need to search the other channels,
      % and we should mark the test blink for removal later on.
      if ~found
        I(i_blink) = -1;
        break;
      end
    end
  end

  % Remove all the false blinks from our results.
  I = I(I ~= -1);

  % Show the EOG waveform, it's wavelet de/reconstruction, and the locations
  % of blinks, but only if we've been told to.
  if plot_fig ~= 0
    plot_bl = zeros(size(channels(1,:)));
    plot_bl(I) = 1;

    nPlots = size(channels,1) + 1;
    figure(plot_fig); clf;

    % Use the stairs plot to represent that this is discrete data.
    for i = 1:size(channels,1)
      subplot(nPlots, 1, i); plot( channels(i,:) );
      title( sprintf('Channel %d', i) ); axis('tight');
    end

if 0
    subplot(nPlots, 1, 2); plot( eog_d );
    title( sprintf( '%s Summed 5, 6, and 7 details', wavelet ) ); axis('tight');
    hold('on'); plot(ma,'r'); plot(ma+t1,'k');
    legend( 'Summed Details', 'Moving Average', 'Threshold' );
end

    subplot(nPlots, 1, nPlots); stairs( 1:length(plot_bl), plot_bl );
    title( 'Blinks '); axis('tight');
  end
end


%
%   I = find_possible_blinks( channel, fs, t1, t_cor )
%
%   This function finds possible blink locations within a single channel.
%
%   A row vector, I, is returned containing the indices of the estimated
%   eyeblink locations within the given channel.
%
%   This function takes two parameters:
%     channel - the EEG channel to process. This should be a vector.
%     fs      - the sample frequency of channel in Hz.
%     t1      - the moving average threshold value to use
%     t_cor   - the correlation threshold to use
%
function I = find_possible_blinks( channel, fs, t1, t_cor )
  % Extract the appropriate time/frequency information using a wavelet transform
  % so that we can estimate the temporal location of eyeblinks.
  wavelet = 'coif3';
  [C,L] = wavedec( channel, 7, wavelet );
  chan_a = wrcoef( 'a', C, L, wavelet, 3 );

  % Sum the upper envelope of the absolute values of the 5th, 6th, and 7th
  % details of the wavelet transform of the EOG.
  % TODO: this is implicitly assuming the sample frequency of the channel is
  %       500 Hz. If that changed, the detail levels to sum would also need to
  %       change.
  chan_d = zeros(size(channel));
  for i = 5:7
    chan_d = chan_d + envelope(abs( wrcoef('d', C, L, wavelet, i) ));
  end

  % Find the moving average of the summed details. wconv1() is used because we
  % want symmetric padding in the convolution.
  ma = wconv1( chan_d, ones(1,512) / 512 );
  ma = ma(256:end-256);

  % Set the threshold based on the moving average.
  thresh = ma + t1;

  % Find the points where the summed details cross the threshold.
  t_cross = find(diff(chan_d > thresh) ~= 0);

  % We're only looking for rising/falling edge pairs that cross the threshold,
  % so make sure that the first crossing we see is a rising edge. If the first
  % value is above the threshold, then the first edge we see will have to be
  % a falling edge.
  if chan_d(1) > thresh
    t_cross = t_cross(2:end);
  end

  % With the first crossing checked, make sure we've got an even number of
  % crossings. If we've got an odd number, then the last crossing is a rising
  % edge without a subsequent falling edge, and it must be eliminated.
  if mod( length(t_cross), 2 ) ~= 0
    t_cross = t_cross(1:end-1);
  end

  % Find the minimums of the approximated channel between the crossing points.
  mins = zeros(1,length(t_cross)/2);
  for i = 1:2:length(t_cross)
    [unused, mins((i+1)/2)] = min(chan_a(t_cross(i):t_cross(i+1)));
    mins((i+1)/2) = mins((i+1)/2) + t_cross(i) - 1;
  end

  % The minimums represent our initial guess of eyeblink locations. We now need
  % to perform some template matching to make sure these are eyeblinks and not
  % some other signal that happens to have the same time/frequency content.

  % Build the template.
  template = [linspace(1,   0,   0.08 * fs), ...
              linspace(0,   0.6, 0.08 * fs), ...
              linspace(0.6, 0.8, 0.06 * fs)(2:end)];

  % Find the minimum points that correlate with the template.
  cors = zeros(size(mins));
  for i = 1:length(mins)
    % Handle edge conditions.
    low_chan  = round(mins(i) - 0.08 * fs);
    high_chan = round(mins(i) + (0.08 + 0.06) * fs - 2);

    low_temp  = 1;
    high_temp = length(template);

    if low_chan < 1
      low_temp = 1 - low_chan + 1;
      low_chan = 1;
    end

    if high_chan > length(chan_a)
      high_temp = length(template) - (high_chan - length(chan_a));
      high_chan = length(chan_a);
    end

    cors(i) = cor(chan_a(low_chan:high_chan), template(low_temp:high_temp));
  end

  % Eliminate the minimum points that do not have a high enough, positive
  % correlation with the template.
  I = mins( cors > t_cor );
end
