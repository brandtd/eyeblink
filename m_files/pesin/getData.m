function data = getData( file )
  fid = fopen( file );
  a = fscanf( fid, '%*s %*s %*s %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %e %*s');
  fclose( fid );

  num_samples = length(a) / 25;
  data = reshape(a, 25, num_samples);
end
