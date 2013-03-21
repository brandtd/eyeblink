#ifndef MATRIX_H
#define MATRIX_H

#include "numtype.h"

#ifdef __cplusplus
extern "C" {
#endif

// The general matrix struct used in all matrix functions. When using this
// struct, elements should be stored internally in column-major format. The
// leading dimension refers to the size of a matrix in memory. It is useful for
// either padding a matrix (e.g. when needed to make for an efficient CUDA
// algorithm) or when blocking the matrix. For example, suppose we allocate a
// matrix of size:
//
//   * * * * * *      x x x x x *      * * * * * *
//   * * * * * *      x x x x x *      * + + + + *
//   * * * * * *  ->  x x x x x *  ->  * + + + + *
//   * * * * * *      x x x x x *      * + + + + *
//   * * * * * *      * * * * * *      * * * * * *
//
// and we only want to put elements in the 'x' spots. Then the leading dimension
// of this matrix is 5, the number of rows is 4, and the number of columns is 5.
// Similarly, if we only want the '+' elements, the leading dimension stays the
// same, but the number of rows would become 3, and the number of columns
// becomes 4.
//
// The need to know the leading dimension becomes apparent when accessing
// elements in the matrix. Assuming the matrix is stored in column-major order,
// accessing the element at row and column (r,c) (assuming zero-based indexing)
// is done by:
//    Matrix x;
//    ...
//    x.elem[ c * x.ld + r ];
//
// The 'lag' parameter indicates how many columns were allocated for the matrix.
// This is used by the GPU functions that need zero-padding to run efficiently.
typedef struct Matrix {
  NUMTYPE *elem;
  int rows;
  int cols;
  int ld;
  int lag;
} Matrix;

// Enum used to specify column or row major format (used, for example, in the
// matrix print function).
typedef enum MajorFormat {
  COL_MAJOR,
  ROW_MAJOR
} MajorFormat;

/**
 * Name: CUBLAS_GEMM
 * Name: GEMM
 *
 * Description:
 * Multiplies two matrices, A and B, storing the result in C. The parameters to
 * this macro should be of type struct Matrix. For the CUBLAS_ macros, the
 * parameters should hold device pointers.
 *
 * Mathematically, this macro results in:
 *    C = A * B;
 *
 * PRE:
 * It is assumed that A, B, and C are all of the correct dimensions and that
 * they have been fully initialized.
 *
 * The C parameter must not be equal to either A or B.
 *
 * Parameters:
 * @param C     where to store the final product
 * @param A     the left matrix in the product
 * @param B     the right matrix in the product
 */
#define CUBLAS_GEMM( C, A, B )              /* defined later in the file */
#define GEMM( C, A, B )                     /* defined later in the file */

/**
 * Name: CUBLAS_GEMM_NT
 * Name: GEMM_NT
 *
 * Same as GEMM, but computes:
 *    C = A * B';
 * where B' is the transpose of B.
 *
 * PRE:
 * It is assumed that A, B, and C are all of the correct dimensions and that
 * they have been fully initialized.
 *
 * The C parameter must not be equal to either A or B.
 *
 * Parameters:
 * @param C   where to store the final product
 * @param A   the left matrix in the product
 * @param B   the right matrix in the product
 */
#define CUBLAS_GEMM_NT( C, A, B )           /* defined later in the file */
#define GEMM_NT( C, A, B )                  /* defined later in the file */

/**
 * Name: CUBLAS_GEMM_TN
 * Name: GEMM_TN
 *
 * Same as GEMM, but computes:
 *    C = A' * B;
 * where A' is the transpose of A.
 *
 * PRE:
 * It is assumed that A, B, and C are all of the correct dimensions and that
 * they have been fully initialized.
 *
 * The C parameter must not be equal to either A or B.
 *
 * Parameters:
 * @param C   where to store the final product
 * @param A   the left matrix in the product
 * @param B   the right matrix in the product
 */
#define CUBLAS_GEMM_TN( C, A, B )           /* defined later in the file */
#define GEMM_TN( C, A, B )                  /* defined later in the file */

/**
 * Name: GEMV
 *
 * Description:
 * Left-multiplies a vector, x, by a matrix, A, storing the result in vector, y.
 * The x and y parameters to this macro should be of type NUMTYPE*, and the A
 * parameter should be of type struct Matrix (not a pointer).
 * 
 * PRE:
 * It is assumed that all parameters are fully initialized and addressable
 * within the space need to compute and store the result of a matrix vector
 * operation (each must have enough element allocated for it).
 *
 * Parameters:
 * @param y   where to store the final product
 * @param A   the matrix in the calculation
 * @param x   the vector in the calculation
 */
#define GEMV( y, A, x )                     /* defined later in the file */

/**
 * Name: CUBLAS_COVARIANCE
 * Name: COVARIANCE
 *
 * Description:
 * Calculate the covariance matrix for a zero-mean observation matrix, where
 * each row of the observation matrix is a variable and each column is an
 * observation of that variable.
 *
 * The parameters of this macro should be of type struct Matrix.
 *
 * PRE:
 * It is assumed that the C and Z matrices are already initialized and that
 * the C matrix is of the correct dimension for storing the covariance matrix
 * of matrix Z (i.e., C is expected to be a square matrix with the same number
 * of rows as the Z matrix).
 *
 * Parameters:
 * @param C     where to store the covariance matrix
 * @param Z     the matrix to operate on
 */
#define CUBLAS_COVARIANCE( C, Z )         /* defined later in the file */
#define COVARIANCE( C, Z )                /* defined later in the file */

/**
 * Name: CUBLAS_COVARIANCE_T
 * Name: COVARIANCE_T
 *
 * Description:
 * Same as COVARIANCE/CUBLAS_COVARIANCE, but the 'Z' parameter is expected to
 * need to be transposed.
 *
 * The parameters of this macro should be of type struct Matrix.
 *
 * PRE:
 * It is assumed that the C and Z matrices are already initialized and that
 * the C matrix is of the correct dimension for storing the covariance matrix
 * of matrix Z (i.e., C is expected to be a square matrix with the same number
 * of columns as the Z matrix [columns because Z will be transposed]).
 *
 * Parameters:
 * @param C     where to store the covariance matrix
 * @param Z     the matrix to operate on
 */
#define CUBLAS_COVARIANCE_T( C, Z )        /* defined later in the file */
#define COVARIANCE_T( C, Z )               /* defined later in the file */

/**
 * Name: SYEV
 *
 * Description:
 * Finds the eigenvalue decomposition of the symmetric matrix E, storing the
 * resulting eigenvalues in d and the orthonormal eigenvectors in E.
 *
 * The E parameter should be of type struct Matrix (N.B. not a pointer to a
 * Matrix) and the d parameter should be of type NUMTYPE*.
 *
 * PRE:
 * Assumptions:
 *   + E is square and symmetric.
 *   + d is already allocated and has at least as many slots as E has rows.
 *
 * POST:
 * The E matrix is overwritten with the eigenvectors for the matrix.
 *
 * Parameters:
 * @param E     the matrix to process and where to store the resulting
 *              eigenvectors
 * @param d     where to store the resulting eigenvalues
 */
#define SYEV( E, d )                      /* defined later in the file */

/**
 * Name: mat_newFromFile
 *
 * Description:
 * Creates a new matrix (allocating space for it) by reading in values for the
 * matrix from the specified file. The file is expected to be a comma-separated-
 * value file specifying the matrix in column-major order.
 *
 * If the file cannot be opened or any other error occurs, this function returns
 * zero. If everything works, this function returns nonzero.
 *
 * Parameters:
 * @param mat         where to store the matrix data
 * @param filename    the CSV file storing that matrix' values
 *
 * Returns:
 * @return int        0 on error, nonzero otherwise
 */
int mat_newFromFile( Matrix *mat, char const *filename );

/**
 * Name: mat_freeMatrix
 *
 * Description:
 * Frees up space used by the passed in matrix.
 *
 * This function simply free()s the element array within the Matrix struct and
 * resets the fields within the Matrix struct. It does not free() the pointer to
 * the matrix.
 *
 * If the given matrix has already been free'd (it's 'elem' parameter is NULL),
 * then this function does nothing.
 *
 * Parameters:
 * @param mat     the Matrix struct on which to operate
 */
void mat_freeMatrix( Matrix *mat );

/**
 * Name: mat_similar
 *
 * Description:
 * Compares two matrices, verifying that they are element by element similar
 * within some epsilon (this is meant to be an operator== method that takes
 * into account quantization errors introduced by floating point).
 *
 * If the matrices are different sizes, or any elements of the same index in the
 * two matrices differ by more than epsilon, this function returns false.
 * Otherwise, it returns true.
 *
 * Parameters:
 * @param mat1      matrix to compare
 * @param mat2      matrix to compare
 * @param epsilon   how close matching elements must be
 *
 * Returns:
 * @return int      0 if the two matrices are not similar, nonzero otherwise
 */
int mat_similar( Matrix const *mat1, Matrix const *mat2, NUMTYPE epsilon );

/**
 * Name: mat_printToFile
 *
 * Description:
 * Prints a matrix to file in either column-major or row-major format.
 *
 * Parameters:
 * @param filename      the file to print to
 * @param matrix        the matrix to print
 * @param major         specification to print in row or column major
 *
 * Returns:
 * @return int          0 if file could not be opened, nonzero otherwise
 */
int mat_printToFile( char const *filename, Matrix const *matrix,
                     MajorFormat major );

/*******************************************************************************
 * Implementation details for using the BLAS and LAPACK routines. The BLAS
 * routines take lots of parameters that result in a lot of boiler plate code.
 * The macros 'declared' above help alleviate that problem.
 *
 * The BLAS routines are written in fortran, whose parameters are all 'pass by
 * reference', which, as far as C goes, means everything's a pointer. Because of
 * that, default arguments cannot just be compile-time constants--they must be
 * pointers to actual, valid memory locations, and there is no header file
 * contain the declaration of the fortran routines.
 *
 * Below, routines are declared, and the macros are setup to use some global
 * variables, saving the user from having to create a bunch of local variables
 * everytime they want to use a BLAS/LAPACK routine. These variables should not
 * be directly modified by user programs.
 *
 * We don't use the CBLAS library for this, because the CBLAS library just does
 * exactly what we're doing here, except it add a function call overhead.
 ******************************************************************************/

// This is where we actually define the macros.
#undef CUBLAS_GEMM
#undef CUBLAS_GEMM_NT
#undef CUBLAS_GEMM_TN
#undef CUBLAS_COVARIANCE
#undef CUBLAS_COVARIANCE_T

#undef GEMM
#undef GEMM_NT
#undef GEMM_TN
#undef GEMV

#undef COVARIANCE
#undef COVARIANCE_T
#undef SYEV

// Which functions we use depends on whether or we are using single or double
// precision floating point values.
#ifdef USE_SINGLE
  #define xGEMM sgemm_
  #define xGEMV sgemv_
  #define xSYEV ssyev_
  #define cublasXgemm   cublasSgemm
  #define cublasXgemv   cublasSgemv
#else
  #define xGEMM dgemm_
  #define xGEMV dgemv_
  #define xSYEV dsyev_
  #define cublasXgemm   cublasDgemm
  #define cublasXgemv   cublasDgemv
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CUBLAS_GEMM( C, A, B ) cublasXgemm( 'N', 'N',\
                                            (A).rows, (B).cols, (A).cols,\
                                            1.0,\
                                            (A).elem, (A).ld,\
                                            (B).elem, (B).ld,\
                                            0.0,\
                                            (C).elem, (C).ld )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CUBLAS_GEMM_NT( C, A, B ) cublasXgemm( 'N', 'T',\
                                               (A).rows, (B).rows, (A).cols,\
                                               1.0,\
                                               (A).elem, (A).ld,\
                                               (B).elem, (B).ld,\
                                               0.0,\
                                               (C).elem, (C).ld )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CUBLAS_GEMM_TN( C, A, B ) cublasXgemm( 'T', 'N',\
                                               (A).cols, (B).cols, (A).rows,\
                                               1.0,\
                                               (A).elem, (A).ld,\
                                               (B).elem, (B).ld,\
                                               0.0,\
                                               (C).elem, (C).ld )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CUBLAS_COVARIANCE( C, Z ) cublasXgemm( 'N', 'T',\
                                               (Z).rows, (Z).rows, (Z).cols,\
                                               1.0 / (NUMTYPE) ((Z).cols - 1),\
                                               (Z).elem, (Z).ld,\
                                               (Z).elem, (Z).ld,\
                                               0.0,\
                                               (C).elem, (C).ld )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define CUBLAS_COVARIANCE_T( C, Z ) cublasXgemm( 'T', 'N',\
                                                 (Z).cols, (Z).cols, (Z).rows,\
                                                 1.0 / (NUMTYPE)((Z).rows - 1),\
                                                 (Z).elem, (Z).ld,\
                                                 (Z).elem, (Z).ld,\
                                                 0.0,\
                                                 (C).elem, (C).ld )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define GEMM( C, A, B ) xGEMM( &_not_transpose, &_not_transpose,\
                               &((A).rows), &((B).cols), &((A).cols),\
                               &_alpha,\
                               (A).elem, &((A).ld),\
                               (B).elem, &((B).ld),\
                               &_beta,\
                               (C).elem, &((C).ld) )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define GEMM_NT( C, A, B ) xGEMM( &_not_transpose, &_transpose,\
                                  &((A).rows), &((B).rows), &((A).cols),\
                                  &_alpha,\
                                  (A).elem, &((A).ld),\
                                  (B).elem, &((B).ld),\
                                  &_beta,\
                                  (C).elem, &((C).ld) )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define GEMM_TN( C, A, B ) xGEMM( &_transpose, &_not_transpose,\
                                  &((A).cols), &((B).cols), &((A).rows),\
                                  &_alpha,\
                                  (A).elem, &((A).ld),\
                                  (B).elem, &((B).ld),\
                                  &_beta,\
                                  (C).elem, &((C).ld) )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define GEMV( y, A, x ) xGEMV( &_not_transpose,\
                               &((A).rows), &((A).cols),\
                               &_alpha,\
                               (A).elem, &((A).ld),\
                               x, &_one,\
                               &_beta, y, &_one )

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define COVARIANCE( C, Z ) _alpha = 1.0 / (NUMTYPE) ((Z).cols - 1);\
                           xGEMM( &_not_transpose, &_transpose,\
                                  &((Z).rows), &((Z).rows), &((Z).cols),\
                                  &_alpha,\
                                  (Z).elem, &((Z).ld),\
                                  (Z).elem, &((Z).ld),\
                                  &_beta,\
                                  (C).elem, &((C).ld) );\
                           _alpha = 1.0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define COVARIANCE_T( C, Z ) _alpha = 1.0 / (NUMTYPE) ((Z).rows - 1);\
                             xGEMM( &_transpose, &_not_transpose,\
                                    &((Z).cols), &((Z).cols), &((Z).rows),\
                                    &_alpha,\
                                    (Z).elem, &((Z).ld),\
                                    (Z).elem, &((Z).ld),\
                                    &_beta,\
                                    (C).elem, &((C).ld) );\
                             _alpha = 1.0;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define SYEV( E, d ) if (_work == NULL) {\
                       _work = (NUMTYPE*) malloc(sizeof(NUMTYPE));\
                       _lwork = 1;\
                     }\
                     xSYEV( &_jobz,&_uplo,&((E).rows),(E).elem,&((E).ld),\
                            (d), _work, &_n1, &_info );\
                     if ((int) _work[0] > _lwork) {\
                       _lwork = (int) _work[0];\
                       if (_work) {\
                         free( _work );\
                       }\
                       _work = (NUMTYPE*) malloc( sizeof(NUMTYPE) * _lwork );\
                     }\
                     xSYEV( &_jobz,&_uplo,&((E).rows),(E).elem,&((E).ld),\
                            (d), _work, &_lwork, &_info )

// These are the fortran functions that we will need to link with at compile
// time.
#ifdef USE_SINGLE
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void sgemm_( char const *transA, char const *transB,
               int const *m, int const *n, int const *k,
               float const *alpha,
               float const *A, int const *ldA,
               float const *B, int const *ldB,
               float const *beta,
               float *C, int const *ldC );

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void sgemv_( char const *trans, int const *m, int const *n,
               float const *alpha,
               float const *A, int const *ldA,
               float const *x, int const *incx,
               float const *beta,
               float *y, int const *incy );

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void ssyev_( char const *jobz, char const *uplo, int const *n,
               float *A, int const *ldA,
               float *w,
               float *work, int const *lwork,
               int *info );
#else
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void dgemm_( char const *transA, char const *transB,
               int const *m, int const *n, int const *k,
               double const *alpha,
               double const *A, int const *ldA,
               double const *B, int const *ldB,
               double const *beta,
               double *C, int const *ldC );

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void dgemv_( char const *trans, int const *m, int const *n,
               double const *alpha,
               double const *A, int const *ldA,
               double const *x, int const *incx,
               double const *beta,
               double *y, int const *incy );

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  void dsyev_( char const *jobz, char const *uplo, int const *n,
               double *A, int const *ldA,
               double *w,
               double *work, int const *lwork,
               int *info );
#endif

// These are the placeholder variables we need to use the BLAS routines.
extern char _not_transpose;
extern char _transpose;
extern NUMTYPE _alpha;
extern NUMTYPE _beta;

extern char _jobz;
extern char _uplo;
extern int _one;
extern int _n1;
extern int _lwork;
extern int _info;
extern NUMTYPE *_work;

#ifdef __cplusplus
}
#endif

#endif
