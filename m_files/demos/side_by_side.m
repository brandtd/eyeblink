s_files = {
  '../eeg_data/simulated/sim_n0_t0.edf',
  '../eeg_data/simulated/sim_n0_t1.edf',
  '../eeg_data/simulated/sim_n0_t2.edf',
  '../eeg_data/simulated/sim_n0_t3.edf',
  '../eeg_data/simulated/sim_n0_t4.edf',
  '../eeg_data/simulated/sim_n0_t5.edf',

  '../eeg_data/simulated/sim_n1_t0.edf',
  '../eeg_data/simulated/sim_n1_t1.edf',
  '../eeg_data/simulated/sim_n1_t2.edf',
  '../eeg_data/simulated/sim_n1_t3.edf',
  '../eeg_data/simulated/sim_n1_t4.edf',
  '../eeg_data/simulated/sim_n1_t5.edf',

  '../eeg_data/simulated/sim_n2_t0.edf',
  '../eeg_data/simulated/sim_n2_t1.edf',
  '../eeg_data/simulated/sim_n2_t2.edf',
  '../eeg_data/simulated/sim_n2_t3.edf',
  '../eeg_data/simulated/sim_n2_t4.edf',
  '../eeg_data/simulated/sim_n2_t5.edf',

  '../eeg_data/simulated/sim_n3_t0.edf',
  '../eeg_data/simulated/sim_n3_t1.edf',
  '../eeg_data/simulated/sim_n3_t2.edf',
  '../eeg_data/simulated/sim_n3_t3.edf',
  '../eeg_data/simulated/sim_n3_t4.edf',
  '../eeg_data/simulated/sim_n3_t5.edf',

  '../eeg_data/simulated/sim_n4_t0.edf',
  '../eeg_data/simulated/sim_n4_t1.edf',
  '../eeg_data/simulated/sim_n4_t2.edf',
  '../eeg_data/simulated/sim_n4_t3.edf',
  '../eeg_data/simulated/sim_n4_t4.edf',
  '../eeg_data/simulated/sim_n4_t5.edf',

  '../eeg_data/simulated/sim_n5_t0.edf',
  '../eeg_data/simulated/sim_n5_t1.edf',
  '../eeg_data/simulated/sim_n5_t2.edf',
  '../eeg_data/simulated/sim_n5_t3.edf',
  '../eeg_data/simulated/sim_n5_t4.edf',
  '../eeg_data/simulated/sim_n5_t5.edf',
};

n_files = {
  '../eeg_data/no_blink/triphasic/triphasic0.edf',
  '../eeg_data/no_blink/triphasic/triphasic1.edf',
  '../eeg_data/no_blink/triphasic/triphasic2.edf',
  '../eeg_data/no_blink/triphasic/triphasic3.edf',
  '../eeg_data/no_blink/triphasic/triphasic4.edf',
  '../eeg_data/no_blink/triphasic/triphasic5.edf',
};

s_edf = readedf( s_files{n} );
n_edf = readedf( n_files{mod((n - 1),6) + 1} );

s_fp1_f3 = diff_channels( s_edf, 'FP1', 'F3' );
s_fp1_f7 = diff_channels( s_edf, 'FP1', 'F7' );
s_fp2_f4 = diff_channels( s_edf, 'FP2', 'F4' );
s_fp2_f8 = diff_channels( s_edf, 'FP2', 'F8' );

n_fp1_f3 = diff_channels( n_edf, 'FP1', 'F3' );
n_fp1_f7 = diff_channels( n_edf, 'FP1', 'F7' );
n_fp2_f4 = diff_channels( n_edf, 'FP2', 'F4' );
n_fp2_f8 = diff_channels( n_edf, 'FP2', 'F8' );

s_chans = [ s_fp1_f3;
            s_fp1_f7;
            s_fp2_f4;
            s_fp2_f8  ];

n_chans = [ n_fp1_f3;
            n_fp1_f7;
            n_fp2_f4;
            n_fp2_f8  ];

blink_detect( s_chans, 'plot', 1 );
blink_detect( n_chans, 'plot', 2 );

n = n + 1;
