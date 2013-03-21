files = { {'../eeg_data/simulated/sim_n2_t0.edf',
           '../eeg_data/no_blink/triphasic/triphasic0.edf' },
          {'../eeg_data/simulated/sim_n2_t1.edf',
           '../eeg_data/no_blink/triphasic/triphasic1.edf' },
          {'../eeg_data/simulated/sim_n2_t2.edf',
           '../eeg_data/no_blink/triphasic/triphasic2.edf' },
          {'../eeg_data/simulated/sim_n2_t3.edf',
           '../eeg_data/no_blink/triphasic/triphasic3.edf' } };

files = { {'../eeg_data/simulated/sim_n2_t2.edf',
           '../eeg_data/no_blink/triphasic/triphasic2.edf' },
          {'../eeg_data/simulated/sim_n2_t3.edf',
           '../eeg_data/no_blink/triphasic/triphasic3.edf' },
          {'../eeg_data/simulated/sim_n2_t4.edf',
           '../eeg_data/no_blink/triphasic/triphasic4.edf' },
          {'../eeg_data/simulated/sim_n2_t5.edf',
           '../eeg_data/no_blink/triphasic/triphasic5.edf' } };

t = gen_template();

wave = 'coif3';
d_min = 6;
d_max = 6;

for i = 1:length(files)
  figure(i);clf;
  for j = 1:2
    edf = readedf( files{i}{j} );
    eog = edf.samples(22,:) - edf.samples(23,:);

    [c,l] = wavedec(eog,8,wave);
    eog_a = wrcoef('a',c,l,wave,5);
    eog_d = zeros(size(eog));

    for k = d_min:d_max
      x = wrcoef('d',c,l,wave,k);
      if length(x) == length(eog_d) + 1
        x = x(1:end-1);
      end
      eog_d = eog_d + envelope(abs(x));
    end

    corr = zeros(1,length(eog));

    for k = 1:length(eog) - length(t) + 1
      corr(k+50) = cor(t, eog_a(k:k+length(t)-1));
    end
    corr( corr < 0 ) = 0;

    if j == 1
      color = 'b';
    else
      color = 'r';
    end

    subplot(3,1,1); hold('on'); plot(eog_a, color);
    subplot(3,1,2); hold('on'); plot(corr, color);
    subplot(3,1,3); hold('on'); plot(envelope(abs(eog_d)), color);
  end
end
