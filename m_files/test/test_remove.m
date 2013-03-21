edf = readedf('../eeg_data/with_blink/normal/normal2.edf' );

R = blink_remove( edf );

bimode(edf,0);
edf.samples = R;
bimode(edf,1);

if 0
figure(1); clf;
for i = 1:10;
  subplot(10, 1, i);
  hold('on');
  plot(edf.samples(i,:), 'b');
  plot(R(i,:), 'r');
end

figure(2); clf;
for i = 11:20
  subplot(10, 1, i - 10);
  hold('on');
  plot(edf.samples(i,:), 'b');
  plot(R(i,:), 'r');
end
end
