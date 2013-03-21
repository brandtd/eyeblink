#!/bin/bash

rm -f data_idwt/*.csv
rm -f data_wavedec/*.csv
rm -f data_wrcoef/*.csv

rm -f data_idwt/*.snip
rm -f data_wavedec/*.snip
rm -f data_wrcoef/*.snip

octave -q gen_files.m > /dev/null

DATA_SUB="wrcoef"
cd data_${DATA_SUB}

echo "" > ${DATA_SUB}_include.snip

for f in `ls *.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "NUMTYPE ${name}[] = {
                             #include \"test/wavelet/data_${DATA_SUB}/${f}\"
                           };
        unsigned int len_${name} = sizeof(${name}) / sizeof(NUMTYPE);
        " >> ${DATA_SUB}_include.snip
done

echo "MultiArray results[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_result.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray c_vectors[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_c_vector.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray l_vectors[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_l_vector.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

cat types.snip >> ${DATA_SUB}_include.snip

DATA_SUB="wavedec"
cd ../data_${DATA_SUB}

echo "" > ${DATA_SUB}_include.snip

for f in `ls *.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "NUMTYPE ${name}[] = {
                             #include \"test/wavelet/data_${DATA_SUB}/${f}\"
                           };
        unsigned int len_${name} = sizeof(${name}) / sizeof(NUMTYPE);
        " >> ${DATA_SUB}_include.snip
done

echo "MultiArray signals[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_signal.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray c_vectors[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_c_vector.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray l_vectors[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_l_vector.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

DATA_SUB="idwt"
cd ../data_${DATA_SUB}

echo "" > ${DATA_SUB}_include.snip

for f in `ls *.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "NUMTYPE ${name}[] = {
                             #include \"test/wavelet/data_${DATA_SUB}/${f}\"
                           };
        unsigned int len_${name} = sizeof(${name}) / sizeof(NUMTYPE);
        " >> ${DATA_SUB}_include.snip
done

echo "MultiArray results[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_result.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray a_coefs[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_coef_a.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip

echo "MultiArray d_coefs[] = {" >> ${DATA_SUB}_include.snip
for f in `ls *_coef_d.csv` ; do
  name=`echo x$f | sed -e 's/.csv$//'`;
  echo "{ ${name}, len_${name} }," >> ${DATA_SUB}_include.snip
done
echo "};" >> ${DATA_SUB}_include.snip
