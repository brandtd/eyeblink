#ifndef FASTICA_KERNELS_H
#define FASTICA_KERNELS_H

/**
 * Name: fica_sumAbs
 *
 * Description:
 * Sums the absolute value of each element of the given matrix. This function
 * is very much not optimized, but it is assumed that this will not be a
 * bottleneck since this function should only be used on relatively small sets
 * of data.
 *
 * PRE:
 * This kernel assumes the given matrix is square and that one block has been
 * given for the entire matrix, with one thread per row. It is also assumed that
 * there are at least 8 rows in the matrix.
 *
 * Parameters:
 * @param d_sum     where to store the sum
 * @param d_X       the matrix to process
 */
void __global__ fica_sumAbs( float *d_sum, float *d_X );

/**
 * Name: fica_scaleMatrix
 *
 * Description:
 * Scales a given matrix by the given scalar.
 *
 * PRE:
 * This kernel assumes one thread per element.
 *
 * Parameters:
 * @param d_X       the matrix to scale
 * @param alpha     the scaling coefficient
 */
void __global__ fica_scaleMatrix( float *d_X, float *alpha );

/**
 * Name: fica_tanh
 *
 * Description:
 * Kernel for finding the tanh(.) of each element of a matrix. Assumes that
 * each block operates on an entire column of the matrix at a time.
 *
 * POST:
 * The values in the given matrix are overwritten with their tanh(.) values.
 *
 * Parameters:
 * @param d_ws    location of the matrix on which to operate
 * @param ld      the 'leading dimension' of the matrix
 */
void __global__ fica_tanh( float *d_ws, int ld );

/**
 * Name: fica_tanhDer
 *
 * Description:
 * Kernel for the gpu_negent_tanh function. Performs the following on the given
 * matrix, WS (in MATLAB syntax):
 *
 *    WS      = 1 - WS.^2;
 *    WS(:,1) = sum(WS')';
 *
 * There is assumed to be one block per row of the given matrix. The function
 * name is based on the idea that WS holds the tanh(.) of values, which means
 * that (1 - WS.^2) will be the tanh'(.) (derivative of tanh()) of those values.
 *
 * PRE:
 * It is assumed that the given matrix is stored in column-major order, that
 * the number of columns in the matrix is a multiple of 256, and that there is
 * only one block of threads per row of the matrix.
 *
 * POST:
 * The first column of the given matrix is overwritten with the result of the
 * above calculation.
 *
 * Parameters:
 * @param d_ws    location of the matrix on which to operate
 * @param ld      the 'leading dimension' of the given matrix
 * @param n_cols  the number of columns of the given matrix
 */
void __global__ fica_tanhDer(float *d_ws, unsigned int ld, unsigned int n_cols);

/**
 * Name: fica_cubeRule
 *
 * Description:
 * Kernel for the gpu_negent_cube function. Performs the following on the given
 * matrix, WS (in MATLAB syntax):
 *
 *    d_wsum = sum( (3.0 * WS .^ 2)' );
 *    WS     = WS .^ 3;
 *
 * There is assumed to be one block per row of the given matrix, and 256 threads
 * per block.
 *
 * PRE:
 * It is assumed that the given matrix is stored in column-major format, that
 * the number of columns is a multiple of 256, and that there is only one block
 * of threads per row of the matrix.
 *
 * Parameters:
 * @param d_wsum    where to store the sum value mentioned above
 * @param d_ws      the location of the 'WS' matrix mentioned above
 * @param ld        the 'leading dimension' of the given matrix
 * @param n_cols    the number of columns in the given matrix
 */
void __global__ fica_cubeRule( float *d_wsum, float *d_ws,
                               unsigned int ld, unsigned int n_cols );

/**
 * Name: fica_gaussRule
 *
 * Description:
 * Kernel for the gpu_negent_gauss function. Performs the following on the given
 * matrix, WS (in MATLAB syntax):
 *
 *    d_wsum = sum( ((1.0 - WS.^2) .* exp( -(WS.^2) / 2 ))' );
 *    WS     = WS .* exp( -(WS.^2) / 2);
 *
 * There is assumed to be one block per row of the given matrix, and 256 threads
 * per block.
 *
 * PRE:
 * It is assumed that the given matrix is stored in column-major format, that
 * the number of columns is a multiple of 256, and that there is only one block
 * of threads per row of the matrix.
 *
 * Parameters:
 * @param d_wsum    where to store the sum value mentioned above
 * @param d_ws      the location of the 'WS' matrix mentioned above
 * @param ld        the 'leading dimension' of the given matrix
 * @param n_cols    the number of columns in the given matrix
 */
void __global__ fica_gaussRule( float *d_wsum, float *d_ws,
                                unsigned int ld, unsigned int n_cols );

/**
 * Name: fica_wnext
 *
 * Description:
 * Kernel for the negent functions. Performs the following on the given W, WX,
 * and SUMS matrices (in MATLAB syntax):
 *
 *    for i = 1:size(W,1)
 *      W(i,:) = (WX(i,:) - sums(i) * W(i,:)) / n_cols;
 *    end
 *
 * There is assumed to be one thread per element of the W matrix. No assumption
 * is made about the number of threads per block or the grid size.
 *
 * PRE:
 * The WX and W matrices are assumed to be identical in size and to have the
 * same leading dimension.
 *
 * POST:
 * The W matrix is overwritten according to the above equation.
 *
 * Parameters:
 * @param d_w     location of the W matrix
 * @param d_wx    location of the WX matrix
 * @param d_sums  location of the SUMS array/matrix
 * @param ld      the 'leading dimension' of the W matrix
 * @param n_cols  the number of columns in the observation matrix
 */
void __global__ fica_wnext( float *d_w, float *d_wx, float *d_sums,
                            unsigned int ld, unsigned int n_cols );

#endif
