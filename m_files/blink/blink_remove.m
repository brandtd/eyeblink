%
%   R = blink_remove( edf, ... )
%   R = blink_remove( ..., 'property', value )
%
%   Removes eyeblink artifacts from an EEG. The edf parameter is expected to be
%   a an edf struct as returned by the readedf() function.
%
%   Various properties of the algorithm are configurable through the parameter
%   value pairs:
%
%    property     |    default  | description
%   --------------+-------------+-----------------------------------------------
%    'implem'     |   'fastica' | Which ICA implementation to use. May be one of
%                 |             | 'fastica' or 'jade'. If 'jade' is specified,
%                 |             | the 'epsilon', 'contrast', and 'max_iter'
%                 |             | properties are unused.
%   --------------+-------------+-----------------------------------------------
%    'epsilon'    |      0.0001 | Convergence criteria used within the ICA
%                 |             | algorithm. See ica() for an explanation.
%   --------------+-------------+-----------------------------------------------
%    'contrast'   |     'gauss' | The contrast/learning rule that is used in the
%                 |             | ICA algorithm. See ica() for an explanation.
%   --------------+-------------+-----------------------------------------------
%    'max_iter'   |         400 | The maximum number of iterations to run the
%                 |             | ICA algorithm. See ica() for an explanation.
%   --------------+-------------+-----------------------------------------------
%    't1'         |          15 | The 't1' threshold given to the blink_detect()
%                 |             | function.
%   --------------+-------------+-----------------------------------------------
%    't_cor'      |        0.75 | The 't_cor' threshold given to blink_detect().
%   --------------+-------------+-----------------------------------------------
%    'fs'         |         500 | Sample frequency (in Hz).
%
function R = blink_remove( edf, varargin )

  % Setup default values.
  epsilon  = 0.0001;
  implem   = 'fastica';
  contrast = 'gauss';
  max_iter = 400;
  t1       = 15;
  t_cor    = 0.75;
  fs       = 500;

  % Extract and verify our arguments.
  for i = 1:2:nargin-1
    if strcmp( varargin{i}, 'epsilon' )
      epsilon = varargin{i+1};
    elseif strcmp( varargin{i}, 'implem' )
      implem = varargin{i+1};

      if ~strcmp(implem, 'fastica') && ~strcmp(implem, 'jade')
        error( 'Invalid ''implem'' value, ''%s''.', implem );
      end
    elseif strcmp( varargin{i}, 'contrast' )
      contrast = varargin{i+1};
    elseif strcmp( varargin{i}, 'max_iter' )
      max_iter = varargin{i+1};
    elseif strcmp( varargin{i}, 't1' )
      t1 = varargin{i+1};

      if t1 <= 0
        error( 'Invalid ''t1'' threshold value, %f.', t1 );
      end
    elseif strcmp( varargin{i}, 't_cor' )
      t_cor = varargin{i+1};
    else
      error( 'Unknown property, ''%s''.', varargin{i} );
    end
  end

  % Extract some channels from the front of the head to use for blink detection.
  channels = [ diff_channels( edf, 'FP1', 'F3' );
               diff_channels( edf, 'FP1', 'F7' );
               diff_channels( edf, 'FP2', 'F4' );
               diff_channels( edf, 'FP2', 'F8' ) ];

  % Get a first guess at the location of blinks by examining the EOG.
  i_blinks = blink_detect( channels, 'plot', 0, 't1', t1, 't_cor', t_cor, ...
                           'fs', fs );

  if length(i_blinks) == 0
    % No blinks were found, so just return the original data since it doesn't
    % appear that there are any blinks to remove.
    R = edf.samples;
    return
  end

  % Perform ICA on the EEG to extract the eyeblink source.
  [W, A, S, mu_S] = ica( edf.samples, 'method',   implem, ...
                                      'contrast', contrast, ...
                                      'max_iter', max_iter, ...
                                      'epsilon',  epsilon );

  % Determine which independent component is the eyeblink source and then find
  % the location of blinks within that component.
  i_source = blink_source( S, i_blinks, fs );
  i_blinks = blinks_in_source( S(i_source,:), i_blinks, fs );

  % Check to see if we've got any blinks left.
  if length( i_blinks ) == 0
    % Our independent component and source signal blink locations don't agree,
    % so just return the original EEG.
    R = edf.samples;
    return
  end

  % Remove the eyeblink artifact from its source by flattening the source signal
  % for 0.4 seconds centered around the location of the blink.

  window = round(0.2 * fs); % 0.2 seconds in samples.

  for i = 1:length(i_blinks)
    if i_blinks(i) - window < 1
      % If the blink occurs near the beginning, make the source assume the value
      % after the blink during the entire blink duration.
      S(i_source,1:i_blinks(i) + window) = S(i_source,i_blinks(i)+window+1);
    elseif i_blinks(i) + window > length(S(i_source,:))
      % If the blink occurs near the end, make the source assume the value
      % before the blink during the entire blink duration.
      S(i_source,i_blinks(i) - window:end) = S(i_source,i_blinks(i)-window-1);
    else
      % Finally, if the blink occurs in middle, make the source assume the
      % average of the source immediately before and after the blink.
      S(i_source,i_blinks(i) - window:i_blinks(i) + window) = ...
          1/2 * (S(i_source,i_blinks(i)-window-1) + ...
                 S(i_source,i_blinks(i)+window+1));
    end
  end

  % Reconstruct the EEG with the modified eyeblink source.
  R = A * S + A * mu_S;

  % Retain the original LEOG and REOG channels (keeping the EOG intact).
  leog_i = find_channel( edf, 'LEOG' );
  reog_i = find_channel( edf, 'REOG' );
  R(leog_i,:) = edf.samples(leog_i,:);
  R(reog_i,:) = edf.samples(reog_i,:);
end
