/*
 * dbn.clamplayer.c -- For a given output configuration, computes the execpted input layer.
 *
 */
#include "pthread.support.h"
 
#include <R.h>
#include <Rdefines.h>
#include <Rmath.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Applic.h>
#include "dbn.h"
#include "rbm.h"
#include "matrix_functions.h"

#ifdef _POSIX_THREADS 
#include <pthread.h>
#endif

/****************************************************************************
 *
 * Compute.
 * 
 * For a given output, computes the input layer.
 */

double *dbn_clamplayer(dbn_t *dbn, double *output, int layer) {
  int rbm_indx = layer -1-1; // For clarity ... likely optimized out.  
							 // First RBM is 1 below layer.  An extra -1 for 0-based indices in C.

  double *layer_output= vector_copy(output, dbn->rbms[rbm_indx].n_outputs);
  double *current_input;
  
  // Daydream for a few iterations to imporve the output sample.
/*  double *current_input= get_layer_inputs(dbn, rbm_indx, output, 1);
  double *input_recon= daydream(&(dbn->rbms[rbm_indx]), 10, current_input);
  double *layer_output= get_layer_outputs(dbn, rbm_indx, input_recon, 1);
  Free(current_input); Free(input_recon);*/
    
  // Walk down the network.
  for(int i=(rbm_indx);i>=0;i--) { 
    current_input= get_layer_inputs(dbn, i, layer_output, 1);
    Free(layer_output);  // Careful of memory leaks when switching around these pointers!!
    layer_output= current_input;
  }

  return(current_input);
}



/*
 *  Sets the input, and returns the output ...
 */ 
SEXP clamplayer_dbn_R(SEXP dbn_r, SEXP output_r, SEXP layer_r, SEXP n_threads_r) {
  dbn_t *dbn= dbn_r_to_c(dbn_r); // Get values from R function.
  
  int n_threads= INTEGER(n_threads_r)[0];
  int layer= INTEGER(layer_r)[0];

  double *output= REAL(output_r);
  int n_examples= 1;//Rf_nrows(input_r)/dbn[0].n_inputs;
 
  SEXP input_r;
  protect(input_r= allocMatrix(REALSXP, dbn->n_inputs, n_examples));
  double *input= REAL(input_r);
  
  double *input_recon= dbn_clamplayer(dbn, output, layer);
  
  for(int i=0;i<dbn->n_inputs;i++)
    input[i]= input_recon[i];
	
  Free(input_recon);
/*  
  #ifdef _POSIX_THREADS 
    run_batch_compute_clamplayer(dbn, input, n_examples, n_threads, output);
  #else 
    run_batch_clamplayer(dbn, input, n_examples, n_threads, output);
  #endif
*/
  unprotect(1);
  return(input_r);
}


