#ifndef TRANSVERSALFILTER_H_
#define TRANSVERSALFILTER_H_

#include "complex.h"
#include <stdlib.h>

//delete following
typedef struct
{
    COMPLEX *sig ;
    unsigned len ;
}SIG_SEQ ;//signal-sequence


COMPLEX MakeCorrelation(SIG_SEQ s, SIG_SEQ w, unsigned short conjFlag)
{
    unsigned i ;
    COMPLEX tmp, corr = {0.0, 0.0} ;
    //if(conjfla == 1), weight coefficient is conj

    if(s.len != w.len)
    {
        printf("ERROR: MakeCorrelation, don't match length.\n") ;
        exit(1) ;
    }
    else
    {
        if(conjFlag == 1)
        {
            for(i = 0; i < s.len; ++i)
            {
                tmp = ProCom( *(s.sig + i), MakeConjugate( *(w.sig + i))) ;
                corr = SumCom(tmp, corr) ;
            }
        }
    }
    return corr ;
}

void TvFliter(SIG_SEQ fltOut, SIG_SEQ fltIn, SIG_SEQ w, unsigned short conjFlag)
{
    unsigned i, k, tapCount ;
    SIG_SEQ tap, fltIn_0add ;
    if( fltOut.len != fltIn.len + (w.len - 1) )
    {
        printf("ERROR: TvFilter, channel input length don't match output\n") ;
        printf("It needs to be (channel out len) = (channel input len) + (tap num - 1).\n") ;
        exit(1) ;
    }
    else
    {
        tap.len = w.len ;
        tap.sig = (COMPLEX *)calloc(tap.len, sizeof(COMPLEX)) ;
        fltIn_0add.len = fltIn.len + (w.len - 1) * 2 ;
        fltIn_0add.sig = (COMPLEX *)calloc(fltIn_0add.len, sizeof(COMPLEX)) ;

        if(tap.sig == NULL || fltIn_0add.sig == NULL)
        {
            printf("ERROR: TvFilter, failed memory capture.\n") ;
            exit(1) ;
        }
        else
        {
            for(i = 0; i < fltIn.len; ++i)
            {
                *(fltIn_0add.sig + (w.len - 1) + i) = *(fltIn.sig + i) ;
            }
            for(k = 0; k < fltOut.len; ++k)
            {
                for(i = 0; i < tap.len; ++i)
                {
                    *(tap.sig + i) = *(fltIn_0add.sig + (tap.len - 1 + k) - i) ;
                    *(fltOut.sig + k) = MakeCorrelation(tap, w, conjFlag) ;
                }
                free(tap.sig) ;
                free(fltIn_0add.sig) ;
            }
        }
    }
}

#endif//TRANSVERSALFILTER_H_