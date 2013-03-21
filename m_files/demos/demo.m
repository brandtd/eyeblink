fig_num = 1;
win_height = (get(0,'screensize')(4) - 130) / 2;
win_width  = get(0,'screensize')(3) / 3;

low_row = 0;
high_row = get(0,'screensize')(4) - win_height;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 2-dimensonal case.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if type == 0
  S = 2*(rand(2, 20000) - 0.5);
  axis_min = -2;
  axis_max =  2;
elseif type == 1
  S = rande(2, 20000);
  signS = rand(2, 20000);
  signS(signS < 0.5) = -1;
  signS(signS > 0.0) =  1;
  S = S .* signS;
  axis_min = -10;
  axis_max =  10;
else
  S = randg( 0.5, 2, 20000 );
  S = 2 * (S ./ (S + randg( 0.5, 2, 20000)) - 0.5);
  axis_min = -2;
  axis_max =  2;
end

A = zeros( size(S,1) );
while det(A) < 0.0001
  A = 2 * rand( size(S,1) );
end

X = A * S;

[Wa,Aa,Sa,muSa] = ica( X );

figure(fig_num, ...
       'Name', 'Two Dimesions', ...
       'Position', [0 0 win_width win_height] );

clf; fig_num = fig_num + 1;
subplot(3,1,1);
plot( S(1,:), S(2,:), '.' );
axis( [ axis_min axis_max axis_min axis_max ] );
title( 'Original source signals' );

subplot(3,1,2);
plot( X(1,:), X(2,:), '.' );
axis( [ axis_min axis_max axis_min axis_max ] );
title( 'Mixed signals' );

subplot(3,1,3);
plot( Sa(1,:), Sa(2,:), '.' );
axis( [ axis_min axis_max axis_min axis_max ] );
title( 'Unmixed signals' );

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Multi-dimensional case.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
i = 0:0.0002:0.9999;
S = [ sin( i * 20 * pi );
      cos( i * 4 * pi ) - 32;
      cos( i * 399 * pi ) + 6;
      cos( i * pi ) - 3;
      sawtooth( i * 32 * pi ) + 10;
      sawtooth( i * 15.23 * pi ) + 10;
      rand( 1, length(i) ) - 32;
      rand( 1, length(i) ) *1.2 - 2;
      rande( 1, length(i) ) + 6;
      rande( 1, length(i) ) * 0.5 - 20; ];

A = zeros( size(S,1) );
while det(A) < 0.0001
  A = 4 * rand( size(S,1) );
end

X = A * S;

[Wa,Aa,Sa,muSa] = ica( X );

% Plot original sources.
figure(fig_num, ...
       'Name', 'Multidimension: Original Signals', ...
       'Position', [0 high_row win_width win_height] );
clf; fig_num = fig_num + 1;
for i = 1:size(S,1)
  subplot(size(S,1),1,i)
  plot(S(i,:));
  axis('tight');
end
subplot(size(S,1),1,1)
title('Original signals');

% Plot mixtures.
figure(fig_num, ...
       'Name', 'Multidimension: Mixed Signals', ...
       'Position', [win_width high_row win_width win_height] );
clf; fig_num = fig_num + 1;
for i = 1:size(S,1)
  subplot(size(S,1),1,i)
  plot(X(i,:));
  axis('tight');
end
subplot(size(S,1),1,1)
title('Mixed signals');

% Plot extracted signals.
figure(fig_num, ...
       'Name', 'Multidimension: Unmixed Signals', ...
       'Position', [2*win_width high_row win_width win_height] );
clf; fig_num = fig_num + 1;
for i = 1:size(S,1)
  subplot(size(S,1),1,i)
  plot(Sa(i,:));
  axis('tight');
end
subplot(size(S,1),1,1)
title('Unmixed signals');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Show the blink removal algorithm working.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
edf = readedf('../eeg_data/with_blink/normal/normal2.edf');
eog = edf.samples(22,:) - edf.samples(23,:);

dirty_eeg = edf.samples;
clean_eeg = blink_remove( dirty_eeg );

% Plot the dirty and clean EEG signals.
figure(fig_num, ...
       'Name', 'Dirty and Cleaned EEG', ...
       'Position', [win_width low_row win_width win_height] );
clf; fig_num = fig_num + 1;
for i = 1:10
  subplot(10,2,i*2 - 1);
  plot( dirty_eeg(i,:) );
  ylabel( strtrim( edf.labels(i,:) ) );
  axis('off');

  subplot(10,2,i*2);
  plot( clean_eeg(i,:) );
  axis('off');
  axis([1 5000 min(dirty_eeg(i,:)) max(dirty_eeg(i,:))]);
end
subplot(10,2,1);
title( 'Dirty EEG' );
subplot(10,2,2);
title( 'Cleaned EEG' );

figure(fig_num, ...
       'Name', 'EOG Channel and Blink Locations', ...
       'Position', [2*win_width low_row win_width win_height] );
clf;
blink_detect( eog, 'plot', fig_num );
