eyeblink
========

Source code for the thesis "Investigation of GPGPU for use in Processing of EEG in Real-time"

Source Code Notes
-----------------

1. The EEG data used with this work is not releasable (also, it's pretty
   large), so only the source code for this thesis has been included. This
   probably affects some of the test code within the matlab scripts, and may
   affect some test code within the C files as well. The algorithms will not be
   affected. I modified the script that will generate test data for the C code
   so that it doesn't expect the EEG data to exist, so you should still be able
   to generate some simple test data and run the tests I created to make sure
   that the code works.
2. The code includes the GUI. I did not do a good job on that and there are
   threading issues within the GUI that will cause it to crash.
3. The CUDA code should all be found under the c_files/include/ica and
   c_files/src/ica directories. The kernels for the JADE algorithm are declared
   in c_files/include/ica/jade/kernels.h and they are defined within
   c_files/src/ica/jade/cuda/kernals.cu. The code for actually using the
   kernels (including all the required setup) is in
   c_files/ica/jade/cuda/jade.cu.

It's been a while since I looked at this code, but if there are any questions,
I will do my best to help out.
