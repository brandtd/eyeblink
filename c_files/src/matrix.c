#include "matrix.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LENGTH   1000

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int mat_newFromFile( Matrix *mat, char const *filename )
{
  int rows, cols, index, col_diff;
  char *line, *value;

  mat->elem = NULL;
  mat->rows = 0;
  mat->cols = 0;

  FILE *matfile = fopen( filename, "r" );

  // Verify that we succesfully opened the given file.
  if (matfile == NULL) {
    return 0;
  }

  // Allocate memory for reading in lines from the file and begin allocating
  // space for storing the matrix as an array.
  line = (char*) malloc( sizeof(char) * LINE_LENGTH );
  mat->elem = (NUMTYPE*) malloc( sizeof(NUMTYPE) );

  // Parse the first line to figure out how many rows are in the matrix.
  line  = fgets(line, LINE_LENGTH, matfile);
  value = strtok( line, "," );

  rows = 1; cols = 1; index = 0;
  mat->elem[index++] = (NUMTYPE) atof( value );

  while ((value = strtok(NULL, ",")) != NULL) {
    rows++;
    mat->elem = (NUMTYPE*) realloc( mat->elem, sizeof(NUMTYPE) * rows );
    mat->elem[index++] = (NUMTYPE) atof( value );
  }

  // Parse the rest of the file one line at a time. Each line represents a new
  // column in the matrix.
  while (fgets(line, LINE_LENGTH, matfile)) {
    cols++;

    mat->elem = (NUMTYPE*) realloc( mat->elem, sizeof(NUMTYPE) * rows * cols );
    value = strtok( line, "," );
    mat->elem[index++] = (NUMTYPE) atof( value );

    while ((value = strtok( NULL, "," )) != NULL) {
      mat->elem[index++] = (NUMTYPE) atof( value );
    }
  }

  // TODO: this looks like something I threw in and forgot about. I don't know
  //       if it's still needed.
  // Make sure the number of allocated columns is a multiple of 256 so that the
  // GPU functions are happy (hello, code coupling!).
  col_diff = 256 - (cols % 256);
  if (col_diff == 256) {
    col_diff = 0;
  } else {
    mat->elem = (NUMTYPE*) realloc( mat->elem,
                                    sizeof(NUMTYPE) * rows * (cols + col_diff));

    // Set the extra space to 0.
    memset( mat->elem + rows * cols, 0, sizeof(NUMTYPE) * col_diff * rows );
  }

  // Make sure to finish initializing the Matrix fields.
  mat->ld   = rows;
  mat->rows = rows;
  mat->cols = cols;
  mat->lag  = cols + col_diff;

  // Free the space we allocated just for this function.
  free( line );
  fclose(matfile);

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void mat_freeMatrix( Matrix *mat )
{
  if (mat->elem) {
    free(mat->elem);
    mat->elem = NULL;
    mat->rows = 0;
    mat->cols = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int mat_similar( Matrix const *mat1, Matrix const *mat2, NUMTYPE epsilon )
{
  int i;

  // Verify both matrices are the same size.
  if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
    return 0;
  }

  // Verify all elements are within one epsilon of each other.
  for (i = 0; i < mat1->rows * mat1->cols; i++) {
    if (fabs(mat1->elem[i] - mat2->elem[i]) > epsilon) {
      return 0;
    }
  }

  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int mat_printToFile( char const *filename, Matrix const *matrix,
                     MajorFormat major )
{
  int row, col;
  FILE *out_file = fopen( filename, "w" );

  // Make sure we managed to open the file.
  if (out_file == NULL) {
    return 0;
  }

  // Switch between printing in column or row major format.
  if (major == COL_MAJOR) {
    for (col = 0; col < matrix->cols; col++) {
      fprintf( out_file, "%.14g", matrix->elem[col * matrix->ld] );
      for (row = 1; row < matrix->rows; row++) {
        fprintf( out_file, ",%.14g", matrix->elem[col * matrix->ld + row] );
      }
      fprintf( out_file, "\n" );
    }
  } else { // ROW_MAJOR
    for (row = 0; row < matrix->rows; row++) {
      fprintf( out_file, "%.14g", matrix->elem[row] );
      fflush(out_file);
      for (col = 1; col < matrix->cols; col++) {
        fprintf( out_file, ",%.14g", matrix->elem[row + col * matrix->ld] );
        fflush(out_file);
      }
      fprintf( out_file, "\n" );
      fflush(out_file);
    }
  }

  fclose( out_file );
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Initialize variables used when calling the BLAS and LAPACK routines.
char _not_transpose = 'n';
char _transpose = 't';
NUMTYPE _beta  = 0.0;
NUMTYPE _alpha = 1.0;

char _jobz = 'V';
char _uplo = 'U';
int _one = 1;
int _n1 = -1;
int _lwork = 0;
int _info = 0;
NUMTYPE *_work = NULL;
