#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PI acos(-1)
#define MSEQ_POL_LEN 4
#define MSEQ_POL_COEFF 1,0,0,1
#define L_MSEQ ((1 << MSEQ_POL_LEN) - 1)
#define N_FADING_CHANNEL_TAP 20
#define N_DELAY 4
#define DELAY 0,4,8,15
#define TAUI 15
#define MITIGATION_DB 10

typedef struct
{
    double I ;
    double Q ;
}COMPLEX ;

typedef struct
{
    COMPLEX *sig ;
    unsigned len ;
}SIG_SEQ ;//signal-sequence

typedef struct
{
    unsigned short *data ;
    unsigned len ;
}DATA_SEQ ;

typedef struct
{
    SIG_SEQ testSeq ;
}TX_SIGNALS ;

typedef struct
{
    SIG_SEQ testSeq ;
    SIG_SEQ noise ;
}RX_SIGNALS ;

typedef struct
{
    double taul ;
    double mtgn_dB ;
    SIG_SEQ coeff ;
    SIG_SEQ output ;
}CHANNEL ;

void InitTx(TX_SIGNALS *a)
{
    a->testSeq.len = L_MSEQ ;
}

void InitRX(RX_SIGNALS *a)
{
    a->noise.len = L_MSEQ + (N_FADING_CHANNEL_TAP - 1) ;
    a->testSeq.len = L_MSEQ + (N_FADING_CHANNEL_TAP - 1) ;
}

void InitCh(CHANNEL *a)
{
    a->coeff.len = N_FADING_CHANNEL_TAP ;
    a->taul = TAUI ;
    a->mtgn_dB = MITIGATION_DB ;
    a->output.len = L_MSEQ + (N_FADING_CHANNEL_TAP - 1) ;
}

void AllocateMemTx(TX_SIGNALS *a)
{
    a->testSeq.sig = (COMPLEX *)calloc( a->testSeq.len, sizeof(COMPLEX) ) ;

    if(a->testSeq.sig == NULL)
    {
        printf("ERROR: AllocateMemTx, failed memory capture.\n") ;
        exit(1) ;
    }
}

void AllocateMemRx(RX_SIGNALS *a)
{
    a->testSeq.sig = (COMPLEX *)calloc(a->testSeq.len, sizeof(COMPLEX)) ;
    a->noise.sig = (COMPLEX *)calloc(a->noise.len, sizeof(COMPLEX)) ;

    if(a->testSeq.sig == NULL || a->noise.sig == NULL)
    {
        printf("ERROR: AllocateMemRx, failed memory capture.\n") ;
        exit(1) ;
    }
}

void AllocateMemCh(CHANNEL *a)
{
    a->output.sig = (COMPLEX *)calloc(a->output.len, sizeof(COMPLEX)) ;
    a->coeff.sig = (COMPLEX *)calloc(a->coeff.len, sizeof(COMPLEX)) ;

    if(a->output.sig == NULL || a->coeff.sig == NULL)
    {
        printf("ERROR: AllocateMemCh, failed memory capture.\n") ;
        exit(1) ;
    }
}

