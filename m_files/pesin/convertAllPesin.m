%
%   Converts all of Jimy Pesin's data files into EDF files. The input files
%   are taken from '../eeg_data/pesins_data/*.txt'. The output file are stored
%   in '../eeg_data/pesins_data/as_eeg/'.
%

files = { '../eeg_data/pesins_data/ap1.txt';
          '../eeg_data/pesins_data/ap2.txt';
          '../eeg_data/pesins_data/ap3.txt';
          '../eeg_data/pesins_data/ap4.txt';
          '../eeg_data/pesins_data/ap5.txt';
          '../eeg_data/pesins_data/ap6.txt';
          '../eeg_data/pesins_data/ap7.txt';
          '../eeg_data/pesins_data/ap8.txt';
          '../eeg_data/pesins_data/ap9.txt';
          '../eeg_data/pesins_data/ap10.txt';
          '../eeg_data/pesins_data/ap11.txt';
          '../eeg_data/pesins_data/dm1.txt';
          '../eeg_data/pesins_data/dm2.txt';
          '../eeg_data/pesins_data/dm3.txt';
          '../eeg_data/pesins_data/dm4.txt';
          '../eeg_data/pesins_data/dm5.txt';
          '../eeg_data/pesins_data/dm6.txt';
          '../eeg_data/pesins_data/dm7.txt';
          '../eeg_data/pesins_data/dm8.txt';
          '../eeg_data/pesins_data/kn1.txt';
          '../eeg_data/pesins_data/kn2.txt';
          '../eeg_data/pesins_data/kn3.txt';
          '../eeg_data/pesins_data/kn4.txt';
          '../eeg_data/pesins_data/kn5.txt';
          '../eeg_data/pesins_data/kn6.txt';
          '../eeg_data/pesins_data/kn7.txt';
          '../eeg_data/pesins_data/kn8.txt';
          '../eeg_data/pesins_data/kn9.txt';
          '../eeg_data/pesins_data/kn10.txt';
          '../eeg_data/pesins_data/kn11.txt';
          '../eeg_data/pesins_data/rv1.txt';
          '../eeg_data/pesins_data/rv2.txt';
          '../eeg_data/pesins_data/rv3.txt' };

for i = 1:size(files,1)
  convertPesinToEDF( files{i}, '../eeg_data/pesins_data/as_edf/' );
end
