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

void FreeMemTx(TX_SIGNALS *a)
{
    free(a->testSeq.sig) ;
}

void FreeMemRx(RX_SIGNALS *a)
{
    free(a->testSeq.sig) ;
    free(a->noise.sig) ;
}

void FreeMemCh(CHANNEL *a)
{
    free(a->output.sig) ;
    free(a->coeff.sig) ;
}


COMPLEX SumCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I + b.I ;
    c.Q = a.Q + b.Q ;
    return c ;
}

COMPLEX SubCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I - b.I ;
    c.Q = a.Q - b.Q ;
    return c ;
}

COMPLEX ProCom(COMPLEX a, COMPLEX b)
{
    COMPLEX c ;
    c.I = a.I * b.I - a.Q * b.Q ;
    c.Q = a.I * b.Q + a.Q * b.I ;
    return c ;
}

COMPLEX MakeConjugate(COMPLEX a)
{
    a.Q = (-1) * a.Q ;
    return a ;
}

double MakeAbs(COMPLEX a)
{
    return ( sqrt(pow(a.I, 2) + pow(a.Q, 2)) ) ;
}

COMPLEX ConvertByEuler(double A, double phi)
{
    COMPLEX c ;
    c.I = A * cos(phi) ;
    c.Q = A * sin(phi) ;
    return c ;
}

//return一様乱数[0.0,1.0]
double GenerateRateU(void)
{
    return ( (double)rand() / (double)RAND_MAX ) ;
}

//return正規乱数
double GenerateRateN(void)
{
    double s,r,t ;
    s = GenerateRateU() ;
    if(s == 0.0) s = 0.000000001 ;
    r = sqrt( -2.0 * log(s) ) ;
    t = 2.0 * PI * GenerateRateU() ;
    return ( r * sin(t) ) ;
}

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

void MakeMSEQ(SIG_SEQ mseq)
{
    unsigned long i ;
    int k, tmp ;
    int tap[MSEQ_POL_LEN] = {0.0} ;
    int mseqPol[] = {MSEQ_POL_COEFF} ;
    tap[0] = 1;

    for(i = 0; i < mseq.len; ++i)
    {
        (mseq.sig + i) -> I = (double)tap[MSEQ_POL_LEN - 1] ;
        tmp = 0;
        for(i = 0; i < MSEQ_POL_LEN; ++k)
        {
            tmp += tap[k] * mseqPol[k] ;
            tmp = tmp % 2 ;
        }

        for(k = (MSEQ_POL_LEN - 1); k > 0; --k)
        {
            tap[k] = tap[k-1] ;
        }
        tap[0] = tmp ;
    }
    for(i = 0; i < mseq.len; ++i)
    {
        if( (mseq.sig + i) -> I == 0.0) (mseq.sig + i) -> I = - 1.0 ;
    }
}

//AWGN発生
void MakeAwgn(SIG_SEQ n, double Pn)// n means noise
{
    double r, t ;
    unsigned i ;
    for(int i = 0; i < n.len; ++i)
    {
        (n.sig + i) -> I = GenerateRateN() * sqrt(Pn/2) ;
        (n.sig + i) -> Q = GenerateRateN() * sqrt(Pn/2) ;
    }
}

double CalculateRayleigh(void)
{
    double n = GenerateRateN() ;
    double m = GenerateRateN() ;
    double c = 1/sqrt(2.0) ;
    return sqrt( pow(n, 2.0) + pow(m, 2.0) ) * c ;
}

void MakeChCoeff(CHANNEL ch)
{
    unsigned i ;
    unsigned delay[] = {DELAY} ;
    double mtgn, delta, p_tau, rayleighAmp, randPhase ;

    mtgn = pow(10.0, ch.mtgn_dB/10.0) ;
    delta = (-1.0) * ch.taul/log(mtgn) ;

    for(i = 0; i < N_DELAY; ++i)
    {
        p_tau = exp( (double)delay[i]/delta ) ;
        rayleighAmp = CalculateRayleigh() * sqrt(p_tau) ;
        randPhase = GenerateRateU() * 2 * PI ;
        *(ch.coeff.sig + delay[i]) = ConvertByEuler(rayleighAmp, randPhase) ;
    }
}

void