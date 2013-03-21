%
%   Plots the source signals that the blink detection algorithm has determined
%   to be the blink source. The signals must be inspected manually.
%

edf_files = {
  '../eeg_data/with_blink/normal/normal0.edf',
  '../eeg_data/with_blink/normal/normal1.edf',
  '../eeg_data/with_blink/normal/normal2.edf',
  '../eeg_data/with_blink/normal/normal3.edf',
  '../eeg_data/with_blink/normal/normal4.edf',
  '../eeg_data/with_blink/normal/normal5.edf',

  '../eeg_data/simulated/sim_n0_t0.edf',
  '../eeg_data/simulated/sim_n1_t0.edf',
  '../eeg_data/simulated/sim_n2_t0.edf',
  '../eeg_data/simulated/sim_n3_t0.edf',
  '../eeg_data/simulated/sim_n4_t0.edf',
  '../eeg_data/simulated/sim_n5_t0.edf',

  '../eeg_data/simulated/sim_n0_t1.edf',
  '../eeg_data/simulated/sim_n1_t1.edf',
  '../eeg_data/simulated/sim_n2_t1.edf',
  '../eeg_data/simulated/sim_n3_t1.edf',
  '../eeg_data/simulated/sim_n4_t1.edf',
  '../eeg_data/simulated/sim_n5_t1.edf',

  '../eeg_data/simulated/sim_n0_t2.edf',
  '../eeg_data/simulated/sim_n1_t2.edf',
  '../eeg_data/simulated/sim_n2_t2.edf',
  '../eeg_data/simulated/sim_n3_t2.edf',
  '../eeg_data/simulated/sim_n4_t2.edf',
  '../eeg_data/simulated/sim_n5_t2.edf',

  '../eeg_data/simulated/sim_n0_t3.edf',
  '../eeg_data/simulated/sim_n1_t3.edf',
  '../eeg_data/simulated/sim_n2_t3.edf',
  '../eeg_data/simulated/sim_n3_t3.edf',
  '../eeg_data/simulated/sim_n4_t3.edf',
  '../eeg_data/simulated/sim_n5_t3.edf',

  '../eeg_data/simulated/sim_n0_t4.edf',
  '../eeg_data/simulated/sim_n1_t4.edf',
  '../eeg_data/simulated/sim_n2_t4.edf',
  '../eeg_data/simulated/sim_n3_t4.edf',
  '../eeg_data/simulated/sim_n4_t4.edf',
  '../eeg_data/simulated/sim_n5_t4.edf',

  '../eeg_data/simulated/sim_n0_t5.edf',
  '../eeg_data/simulated/sim_n1_t5.edf',
  '../eeg_data/simulated/sim_n2_t5.edf',
  '../eeg_data/simulated/sim_n3_t5.edf',
  '../eeg_data/simulated/sim_n4_t5.edf',
  '../eeg_data/simulated/sim_n5_t5.edf'
};

for i = 1:size(edf_files,1)
  edf = readedf(edf_files{i});
  fp1_f3 = diff_channels( edf, 'FP1', 'F3' );
  fp1_f7 = diff_channels( edf, 'FP1', 'F7' );
  fp2_f4 = diff_channels( edf, 'FP1', 'F4' );
  fp2_f8 = diff_channels( edf, 'FP1', 'F8' );

  channels = [ fp1_f3; fp1_f7; fp2_f4; fp2_f8 ];
  I = blink_detect( channels );
  [Wa,Aa,Sa,musa] = ica(edf.samples);

  j = blink_source(Sa,I,500);

  figure( floor((i - 1)/10) + 1 );
  subplot( 10, 1, mod(i - 1,10) + 1 );
  plot(Sa(j,:));
end
