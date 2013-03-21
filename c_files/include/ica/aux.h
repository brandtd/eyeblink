#ifndef ICA_AUX_H
#define ICA_AUX_H

#include "matrix.h"
#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Name: remmean
 *
 * Description:
 * Removes the mean from a set of observation vectors, returning the resulting
 * zero-mean observations and the means of the original observations.
 *
 * Parameters:
 * @param means     where to store the calculated means
 * @param Z         where to store the zero-mean observations
 * @param X         the observations vectors
 *
 * PRE:
 * The X matrix is assumed to be stored in column-major format, and it is
 * expected that each row of X represents a variable, and each column of X
 * represents an observation of each variable.
 *
 * The Z matrix is expected to have already been initialized.
 *
 * POST:
 * The means and Z parameters will be filled with data. The Z matrix will be
 * stored in column-major format.
 */
void remmean( NUMTYPE *means, Matrix *Z, Matrix const *X );

/**
 * Name: remmeanTranspose
 *
 * Description:
 * Removes the mean from a set of observation vectors, returning the resulting
 * zero-mean observations and the means of the original observations.
 *
 * This function will compute the zero-mean observation matrix and will store
 * its transpose in the Z matrix (this is what the "Transpose" in the function
 * name refers to).
 *
 * Parameters:
 * @param means     where to store the calculated means
 * @param Z         where to store the zero-mean observations
 * @param X         the observations vectors
 *
 * PRE:
 * The X matrix is assumed to be stored in column-major format, and it is
 * expected that each row of X represents a variable, and each column of X
 * represents an observation of each variable.
 *
 * The Z matrix is expected to have already been initialized such that if X is
 * an NxT matrix then Z is a TxN matrix.
 *
 * POST:
 * The means and Z parameters will be filled with data. The Z matrix will be
 * stored in column-major format.
 */
void remmeanTranspose( NUMTYPE *means, Matrix *Z, Matrix const *X );

/**
 * Name: whiten
 *
 * Description:
 * Whitens a set of a zero-mean observations, returning the resulting whitened
 * observations and the whitening and dewhitening matrices.
 *
 * See computeWhiten() for an explanation of the whitening process and an
 * explanation of the transpose parameter.
 *
 * If transpose is zero, and we let W be the whitening matrix, then the white_Z
 * matrix will be equal to white_Z = W * Z. If transpose is nonzero, then the
 * white_Z matrix will equal white_Z = W * Z'.
 *
 * Parameters:
 * @param white_Z   where to store the whitened observations
 * @param whiten    where to store the whitening matrix
 * @param dewhiten  where to store the dewhitening matrix
 * @param Z         where to find the zero-mean observations
 * @param transpose   whether or not the Z matrix must be transposed
 *
 * PRE:
 * The same preconditions that apply to computeWhiten apply to this function.
 *
 * POST:
 * The white_Z, whiten, and dewhiten matrices will be filled with column-major
 * formatted data.
 */
void whiten( Matrix *white_Z, Matrix *whiten, Matrix *dewhiten,
             Matrix const *Z, int transpose );

/**
 * Name: computeWhiten
 *
 * Description:
 * Computes the whitening and dewhitening matrices for a set of zero-mean
 * observations.
 *
 * To make observations white, we find the eigenvalue decomposition of the
 * zero-mean observations' covariance matrix. This lets us compute whitening
 * and dewhitening matrices. The whitening matrix will convert our zero-mean
 * observations into uncorrelated, unit variance variables. The dewhitening
 * matrix can be used to undo the whitening effect, and, for ICA, is useful for
 * computing the unmixing matrix.
 *
 * The whitening and dewhitening matrices are equal to:
 *    White   = D^(-1/2) * E'
 *    Dewhite = E * D^(1/2)
 * where D is the diagonal matrix of eigenvalues and E' is the transpose of
 * the eigenvector matrix, both coming from the covariance matrix of the zero-
 * mean observations.
 *
 * The transpose parameter indicates whether the rows or columns of Z are
 * variables or observations. If transpose is zero, then each row of Z is
 * treated as a variable and each column as an observation of the variables. If
 * the transpose parameter is nonzero then it is assumed that Z is the transpose
 * of the observation matrix, meaning that each column of Z is a variable and
 * each row is an observation of those variables.
 *
 * Put another way, if the transpose parameter is zero, then if we let W be the
 * whitening matrix, then Y = W * Z will yield the whitened observations. If the
 * transpose parameter is nonzero, then Y = W * Z' will yield the whitened
 * observations, where .' represents the transpose operator.
 *
 * Parameters:
 * @param whiten      where to store the whitening matrix
 * @param dewhiten    where to store the dewhitening matrix
 * @param Z           where to find the zero-mean observations
 * @param transpose   whether or not the Z matrix must be transposed
 *
 * PRE:
 * The matrices are all assumed to be initialized, and the Z matrix is expected
 * to be stored in column-major format.
 *
 * The whitening/dewhitening matrices will be square matrices with the same
 * number of rows as Z (same number of columns if transpose is nonzero).
 *
 * POST:
 * The whiten, and dewhiten matrices will be filled with column-major formatted
 * data.
 */
void computeWhiten( Matrix *whiten, Matrix *dewhiten, Matrix const *Z,
                    int transpose );

#ifdef __cplusplus
}
#endif

#endif
