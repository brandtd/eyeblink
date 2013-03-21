%
%   j = blink_source( S, I, fs )
%
%   Identifies the component that is most likely to be the eyeblink source.
%
%   `S' should be the extracted source signal matrix, `I' should be the detected
%   eyeblink indices (as returned by the blink_detect() function), and `fs'
%   should be the sample frequency used when recording the EEG.
%
function j = blink_source( S, I, fs )
  % Build the template to correlate against by placing a template eyeblink at
  % every index indicated by `I'.
  eb_template_left  =   linspace( 0,   -1,   0.08 * fs);
  eb_template_right = [ linspace(-1,   -0.4, 0.08 * fs), ...
                        linspace(-0.4, -0.2, 0.06 * fs)(2:end)];
  eb_template = [eb_template_left, eb_template_right];

  template = zeros(1,size(S,2));
  for i = 1:length(I)
    % TODO: make sure the low and high do not exceed the bounds of the vector.
    low_i  = I(i) - size(eb_template_left, 2);
    high_i = I(i) + size(eb_template_right,2) - 1;
    template(low_i:high_i) = eb_template;
  end

  % Find the source signal with the largest absolute correlation.
  [unused, j] = max(abs(cor(template, S')));
end
