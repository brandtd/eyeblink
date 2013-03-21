n = 6;
tt = 8;

figure(1)
clf;

b1 = eb_3;
t1 = ones(size(b1)) * tt / 1;

xx = eb_3(1000:3499);
b2 = [ xx xx ];
t2 = ones(size(b2)) * tt / 2;

xx = eb_3(1500:3166);
b3 = [ xx xx xx ];
t3 = ones(size(b3)) * tt / 3;

xx = eb_3(2000:3249);
b4 = [ xx xx xx xx ];
t4 = ones(size(b4)) * tt / 4;

xx = eb_3(2000:2999);
b5 = [ xx xx xx xx xx ];
t5 = ones(size(b5)) * tt / 5;

xx = eb_3(2000:2832);
b6 = [ xx xx xx xx xx xx ];
t6 = ones(size(b6)) * tt / 6;

subplot(n,1,1); hold('on'); plot( b1 ./ var(b1), 'b' ); plot(t1,'r'); axis('tight');
subplot(n,1,2); hold('on'); plot( b2 ./ var(b2), 'b' ); plot(t2,'r'); axis('tight');
subplot(n,1,3); hold('on'); plot( b3 ./ var(b3), 'b' ); plot(t3,'r'); axis('tight');
subplot(n,1,4); hold('on'); plot( b4 ./ var(b4), 'b' ); plot(t4,'r'); axis('tight');
subplot(n,1,5); hold('on'); plot( b5 ./ var(b5), 'b' ); plot(t5,'r'); axis('tight');
subplot(n,1,6); hold('on'); plot( b6 ./ var(b6), 'b' ); plot(t6,'r'); axis('tight');

figure(2)
clf;

plot(eb_2);

b1 = [ eb_2(1000:2999) eb_2(1:999) eb_2(1000:2999) ];
t1 = ones(size(b1)) * tt / 1;

b2 = eb_2;
t2 = ones(size(b2)) * tt / 2;

xx = eb_2(1:1666);
b3 = [ xx xx xx ];
t3 = ones(size(b3)) * tt / 3;

xx = eb_2(1:1249);
b4 = [ xx xx xx xx ];
t4 = ones(size(b4)) * tt / 4;

xx = eb_2(1:999);
b5 = [ xx xx xx xx xx ];
t5 = ones(size(b5)) * tt / 5;

xx = eb_2(1:832);
b6 = [ xx xx xx xx xx xx ];
t6 = ones(size(b6)) * tt / 6;

subplot(n,1,1); hold('on'); plot( b1 ./ var(b1), 'b' ); plot(t1,'r'); axis('tight');
subplot(n,1,2); hold('on'); plot( b2 ./ var(b2), 'b' ); plot(t2,'r'); axis('tight');
subplot(n,1,3); hold('on'); plot( b3 ./ var(b3), 'b' ); plot(t3,'r'); axis('tight');
subplot(n,1,4); hold('on'); plot( b4 ./ var(b4), 'b' ); plot(t4,'r'); axis('tight');
subplot(n,1,5); hold('on'); plot( b5 ./ var(b5), 'b' ); plot(t5,'r'); axis('tight');
subplot(n,1,6); hold('on'); plot( b6 ./ var(b6), 'b' ); plot(t6,'r'); axis('tight');
