#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "R.h"
#include "Rdefines.h"

struct idxStruct
{
   int  from;
   int  to;
};

int lowerBound(double x,double *val,int first, int length){
int half,mid; 
  while (length > 0) {
    half = length >> 1;
    mid = first;
    mid += half;
    if ( val[mid] < x){
      first = mid;
      first ++;
      length = length - half -1;
    }
    else length = half;
  }
  return(first);
}

int upperBound(double x,double *val,int first, int length){
int half,mid;  
  while (length > 0) {
    half = length >> 1;
    mid = first;
    mid += half;
    if (x < val[mid]){
      length = half;
    }
    else {
      first = mid;
      first ++;
      length = length - half -1;
    }
  }
  return(first);
}

SEXP fastMatch(SEXP x, SEXP y, SEXP xidx, SEXP yidx, SEXP xolength, SEXP tol) {
    double *px, *py, dtol;
    int nx, ny, yi, xi, lb, ub, txi, from, to, *pxidx, *pyidx, xoLength;
    SEXP ans, residx;

    px = REAL(x);
    py = REAL(y);
    pxidx = INTEGER(xidx);
    pyidx = INTEGER(yidx);
    xoLength = INTEGER(xolength)[0];
    dtol = REAL(tol)[0];
    nx = length(x);
    ny = length(y);
    
    struct idxStruct * pidxS = calloc(nx,  sizeof(struct idxStruct));
    if (pidxS == NULL)
        error("fastMatch/calloc: memory could not be allocated ! (%d bytes)\n", nx  * sizeof(struct idxStruct) );
    for (xi=0;xi < nx;xi++) 
         pidxS[xi].from = nx+1;
    
    for (yi=0;yi < ny;yi++) {
       lb = lowerBound(py[yi] - dtol, px, 0, nx); 
       
       if (lb >= nx-1){
            lb=nx-1;
            ub=nx-1;
       } else
            ub = upperBound(py[yi] + dtol, px, lb, nx-lb);
            
       if (ub > nx-1)
            ub = nx -1;
            
       for (xi=lb;xi <= ub;xi++) {
            if (fabs(py[yi] - px[xi]) <= dtol) {
                if (yi < pidxS[xi].from)
                    pidxS[xi].from = yi;
                if (yi > pidxS[xi].to)    
                    pidxS[xi].to = yi;
            }
       }
    }
    
    PROTECT(ans = allocVector(VECSXP, xoLength));
        
    for (xi=0;xi < nx;xi++) {
        
        // no match 
        if (pidxS[xi].from == nx +1 && pidxS[xi].to == 0)
            continue; 
           
        txi = pxidx[xi] -1;   
        from = pidxS[xi].from; 
        to = pidxS[xi].to;
           
        // single match
        if (pidxS[xi].from == nx +1)
            from=pidxS[xi].to;
        if (pidxS[xi].to == 0)
            to=pidxS[xi].from;    
            
        PROTECT(residx = NEW_INTEGER(to-from+1));  
        
        int p=0;
        for (yi=from;yi <= to;yi++) {
            INTEGER_POINTER(residx)[p] = pyidx[yi];
            p++;
        }
        
        SET_VECTOR_ELT(ans, txi, residx);
        UNPROTECT(1); // residx
  }   

    UNPROTECT(1); // ans
    free(pidxS);
    return(ans);
}

