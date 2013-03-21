x = rand(3, 400);
mx = repmat(mean(x')', 1, 400);
zx = x - mx;
[e,d] = eig(zx * zx' * 1/400);
[p,k] = sort(diag(d));
w = diag( p.^(-1/2) ) * e(1:3,k)';
wx = w * zx;

cm = jadeR(x);

a = 1/sqrt(2);

disp('-=-=-=-=-');
m = [ 1 0 0;
      0 0 0;
      0 0 0 ];
q = cumulant( wx, m )
cm(1:3,1:3)

disp('-=-=-=-=-');
m = [ 0 0 0;
      0 1 0;
      0 0 0 ];
q = cumulant( wx, m )
cm(1:3,4:6)

disp('-=-=-=-=-');
m = [ 0 a 0;
      a 0 0;
      0 0 0 ];
q = cumulant( wx, m )
cm(1:3,7:9)

disp('-=-=-=-=-');
m = [ 0 0 0;
      0 0 0;
      0 0 1 ];
q = cumulant( wx, m )
cm(1:3,10:12)

disp('-=-=-=-=-');
m = [ 0 0 a;
      0 0 0;
      a 0 0 ];
q = cumulant( wx, m )
cm(1:3,13:15)

disp('-=-=-=-=-');
m = [ 0 0 0;
      0 0 a;
      0 a 0 ];
q = cumulant( wx, m )
cm(1:3,16:18)
