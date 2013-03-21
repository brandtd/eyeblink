function bimode(edf,varargin)
  % EOG = LEOG - REOG

  % I want channels:
  %   FP1 - F7
  %   F7 - T7
  %   T7 - P7
  %   P7 - O1
  %
  %   FP1 - F3
  %   F3 - C3
  %   C3 - P3
  %   P3 - O1
  %
  %   FP2 - F4
  %   F4 - C4
  %   C4 - P4
  %   P4 - O2
  %
  %   FP2 - F8
  %   F8 - T8
  %   T8 - P8
  %   P8 - O2

  channels = {};

  % Far left side of the head.
  channels{1,1} = { 'FP1 - F7', diff_channels( edf, 'FP1', 'F7' ) };
  channels{2,1} = { 'F7 - T7', diff_channels( edf, 'F7', 'T7' ) };
  channels{3,1} = { 'T7 - P7', diff_channels( edf, 'T7', 'P7' ) };
  channels{4,1} = { 'P7 - O1', diff_channels( edf, 'P7', 'O1' ) };

  % Near left side of the head.
  channels{1,2} = { 'FP1 - F3', diff_channels( edf, 'FP1', 'F3' ) };
  channels{2,2} = { 'F3 - C3', diff_channels( edf, 'F3', 'C3' ) };
  channels{3,2} = { 'C3 - P3', diff_channels( edf, 'C3', 'P3' ) };
  channels{4,2} = { 'P3 - O1', diff_channels( edf, 'P3', 'O1' ) };

  % Near right side of the head.
  channels{1,3} = { 'FP2 - F4', diff_channels( edf, 'FP2', 'F4' ) };
  channels{2,3} = { 'F4 - C4', diff_channels( edf, 'F4', 'C4' ) };
  channels{3,3} = { 'C4 - P4', diff_channels( edf, 'C4', 'P4' ) };
  channels{4,3} = { 'P4 - O2', diff_channels( edf, 'P4', 'O2' ) };

  % Far right side of the head.
  channels{1,4} = { 'FP2 - F8', diff_channels( edf, 'FP2', 'F8' ) };
  channels{2,4} = { 'F8 - T8', diff_channels( edf, 'F8', 'T8' ) };
  channels{3,4} = { 'T8 - P8', diff_channels( edf, 'T8', 'P8' ) };
  channels{4,4} = { 'P8 - O2', diff_channels( edf, 'P8', 'O2' ) };

  % Plot channels.
  %clf;
  for row = 1:4
    for col = 1:4
      subplot(4,4,(row-1) * 4 + col);
      if nargin == 2
        hold('on')
      end

      if nargin == 2 && varargin{1} == 0
        plot(channels{row,col}{2}, 'b');
      else
        plot(channels{row,col}{2}, 'r');
      end
      title(channels{row,col}{1});
      axis('off');
    end
  end
end
