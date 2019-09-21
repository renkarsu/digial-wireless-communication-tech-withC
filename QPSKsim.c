//gcc -std=c99 QPSKsim.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define PI acos(-1) //3.1415926535
#define L 10000     //送信データビット数
#define SNR_STEP 10 //誤り率を計算するSNRの値の数. SNR(signal-noise ratio): SN比

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
    DATA_SEQ data ;
    SIG_SEQ symbol ;
}TX_SIGNALS ;

typedef struct
{
    DATA_SEQ data ;
    SIG_SEQ symbol ;
    SIG_SEQ noise ;
}RX_SIGNALS ;

void InitTx(TX_SIGNALS *a)
{
    a->data.len = L ;
    a->symbol.len = L/2 ;
}

void InitRx(RX_SIGNALS *a)
{
    a->data.len = L ;
    a->symbol.len = L/2 ;
    a->noise.len = L/2 ;
}

void AllocateMemTx(TX_SIGNALS *a)
{
    a->data.data = (unsigned short*)malloc( sizeof(unsigned short) * a->data.len ) ;

    a->symbol.sig = (COMPLEX *)malloc( sizeof(COMPLEX) * a->symbol.len) ;

    if(a->data.data == NULL || a->symbol.sig == NULL)
    {
        printf("ERROR: AllocateMemTx, failed memory capture\n") ;
        exit(1) ; 
    }
}

void AllocateMemRx(RX_SIGNALS *a)
{
    a->data.data = (unsigned short*)malloc( sizeof(unsigned short) * a->data.len ) ;

    a->symbol.sig = (COMPLEX *)malloc( sizeof(COMPLEX) * a->symbol.len) ;

    a->noise.sig = (COMPLEX *)malloc( sizeof(COMPLEX) * a->symbol.len) ;

    if(a->data.data == NULL || a->symbol.sig == NULL || a->noise.sig == NULL)
    {
        printf("ERROR: AllocateMemRx, failed memory capture\n") ;
        exit(1) ; 
    }
}

void FreeMemTx(TX_SIGNALS *a)
{
    free(a->data.data) ;
    free(a->symbol.sig) ;
}

void FreeMemRx(RX_SIGNALS *a)
{
    free(a->data.data) ;
    free(a->symbol.sig) ;
    free(a->noise.sig) ;
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

//ランダムデータ発生
void MakeRandData(DATA_SEQ d)//d means input data
{
    unsigned i ;
    double x ;
    for(i = 0; i < d.len; ++i)
    {
        x = GenerateRateU() ;
        if(x >= 0.5) *(d.data+i) = 1.0 ;
        else         *(d.data+i) = 0.0 ;
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

//雑音電力計算
double CalculateSnrDb2NoisePower(double c_dB)
{
    return pow( 10, (-1) * c_dB / 10 ) ;
}

//ベクトル加算（系列同士の加算）
void SumVector(SIG_SEQ a, SIG_SEQ b, SIG_SEQ c)
{
    unsigned i ;
    if(a.len != b.len || b.len != c.len)
    {
        printf("ERROR: SumVector, don't match length.\n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < a.len; ++i)
        {
            (a.sig + i) -> I = (b.sig + i) -> I + (c.sig + i) -> I ;
            (a.sig + i) -> Q = (b.sig + i) -> Q + (c.sig + i) -> Q ;
        }
    }
}

//BER計算
double CalculateBER(DATA_SEQ d, DATA_SEQ rData)
{
    unsigned i ;
    unsigned sum = 0 ;
    double BER ;
    if(d.len != rData.len)
    {
        printf("ERROR: CalculateBer, don't match length.\n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < d.len; ++i)
        {
            sum += abs( *(d.data + i) - *(rData.data + i) ) ;
        }
        BER = (double)sum / d.len ;
    }
    return (BER) ;
}

//BERをファイルへ書き出す
void PrintBER(unsigned snr_step, double *SNRdB, double *BER)
{
    unsigned i ;
    FILE *fp ;
    if( ( fp = fopen("QPSKBER.txt", "w") ) == NULL )
    {
        printf("Error: cannot open file \n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < snr_step; ++i)
        {
            printf("%f [dB] BER = %f\n", *(SNRdB + i), *(BER + i) ) ;
            fprintf(fp, "%f %f\n", *(SNRdB + i), *(BER + i) ) ;
        }
        fclose(fp) ;
    }
}

void ModulateQPSK(SIG_SEQ s, DATA_SEQ d)
{
    unsigned i,co ;
    double c = 1/sqrt(2.0) ;
    if(s.len * 2 != d.len)
    {
        printf("ERROR: ModulateQPSK, signal length don't match data length.\n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < s.len; ++i)
        {
            co = 2*i ;
            (s.sig + i) -> I = c * ( (double) *(d.data + co) - 0.5 ) * 2.0 ;
            (s.sig + i) -> Q = c * ( (double) *(d.data + co + 1) - 0.5 ) * 2.0 ;
        }
    }
}

void DemodulateQPSK(DATA_SEQ rData, SIG_SEQ rs)
{
    unsigned i, co ;

    for(i = 0; i < rData.len; ++i) *(rData.data + i) = 0 ;

    for(i = 0; i < rs.len; ++i)
    {
        co = 2*i ;

        if( (rs.sig + i) -> I >= 0.0 ) *(rData.data + co) = 1 ;

        if( (rs.sig + i) -> Q >= 0.0 ) *(rData.data + co + 1) = 1 ; 
    }
}

int main(void)
{
    TX_SIGNALS tx ;
    RX_SIGNALS rx ;
    double BER[SNR_STEP], SNRdB[SNR_STEP] ;
    unsigned i ;

    InitTx(&tx) ;
    InitRx(&rx) ;
    AllocateMemTx(&tx) ;
    AllocateMemRx(&rx) ;

    for(i = 0; i < SNR_STEP; ++i) SNRdB[i] = (double)i ;

    for(i = 0; i < SNR_STEP; ++i)
    {
        srand( time(NULL) ) ;
        MakeRandData( tx.data ) ;            
        ModulateQPSK( tx.symbol, tx.data ) ;

        double Pn = CalculateSnrDb2NoisePower(SNRdB[i]) ;
        MakeAwgn( rx.noise, Pn ) ;
        SumVector( rx.symbol, tx.symbol, rx.noise ) ;

        DemodulateQPSK( rx.data, rx.symbol ) ;

        BER[i] = CalculateBER( tx.data, rx.data ) ;
    }

    PrintBER( SNR_STEP, SNRdB, BER) ;

    FreeMemTx(&tx) ;
    FreeMemRx(&rx) ;

    return 0 ;
}