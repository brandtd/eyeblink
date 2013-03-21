cd ../../../../m_files
runfirst;

signals = { sin((1:1000) * 0.01 * pi);
            cos((1:5000) * 0.01 * pi);
            (1:2000) };

c = {};
l = {};

for i = 1:size(signals,1)
  [c{i}, l{i}] = wavedec( signals{i}, 9, 'coif3' );
end

cd ../c_files/include/test/wavelet

cd data_wavedec
for i = 1:size(signals,1)
  csvwrite( sprintf( '%d_signal.csv', i ),   signals{i} );
  csvwrite( sprintf( '%d_c_vector.csv', i ), c{i} );
  csvwrite( sprintf( '%d_l_vector.csv', i ), l{i} );
end

cd ../data_idwt
for i = 1:size(signals,1)
  l2 = l{i}(1:end-1);
  cA = wrcoef( 'a', c{i}, l2, 'coif3', 0 );
  cD = c{i}( sum(l{i}(1:end-1)) + 1 : end );
  cD = cD(1:length(cA));
  csvwrite( sprintf( '%d_result.csv', i ), signals{i} );
  csvwrite( sprintf( '%d_coef_a.csv', i ), cA );
  csvwrite( sprintf( '%d_coef_d.csv', i ), cD );
end

cd ../data_wrcoef
k = size(signals,1);
for i = 1:size(signals,1)
  csvwrite( sprintf( '%d_result.csv', i ), wrcoef('d', c{i}, l{i}, 'coif3', 5) );
  csvwrite( sprintf( '%d_c_vector.csv', i ), c{i} );
  csvwrite( sprintf( '%d_l_vector.csv', i ), l{i} );

  csvwrite( sprintf( '%d_result.csv', i+k ), wrcoef('a', c{i}, l{i}, 'coif3', 5) );
  csvwrite( sprintf( '%d_c_vector.csv', i+k ), c{i} );
  csvwrite( sprintf( '%d_l_vector.csv', i+k ), l{i} );
end

fid = fopen( 'types.snip', 'w' );
fwrite(fid, 'ReconType types[] = {RECON_DETAIL, RECON_DETAIL, RECON_DETAIL,' );
fwrite(fid, '                     RECON_APPROX, RECON_APPROX, RECON_APPROX};');
fclose(fid);
