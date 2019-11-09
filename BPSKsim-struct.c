//gcc -std=c99 BPSKsim-struct.c

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

//BPSK変調器(Modulator)
void ModulateBpsk(SIG_SEQ s, DATA_SEQ d)//s means bpsk symbol
{
    unsigned i ;
    if(s.len != d.len)
    {
        printf("ERROR: ModulateBpsk, signal length don't match data length.\n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < s.len; ++i) (s.sig+i) -> I = ( *(d.data + i) - 0.5) * 2.0 ;
    }
}

//AWGN発生
void MakeAwgn(SIG_SEQ n, double Pn)// n means noise
{
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

//BPSK復調器(Demodulator)
void DemodulateBPSK(DATA_SEQ rData, SIG_SEQ rs)//rData means receiving data, rs means receiving symbol
{
    unsigned i ;
    for(i = 0; i < rs.len; ++i)
    {
        if( (rs.sig + i) -> I > 0 ) *(rData.data + i) = 1 ;
        else                        *(rData.data + i) = 0 ;
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
    if( ( fp = fopen("BER.txt", "w") ) == NULL )
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

int main(void)
{
    DATA_SEQ txData, rxData ;
    SIG_SEQ txSymbol, rxSymbol, noise ;
    double BER[SNR_STEP], SNRdB[SNR_STEP] ;
    unsigned i ;
    txData.len = L ;
    rxData.len = L ;
    txSymbol.len = L ;
    rxSymbol.len = L ;
    noise.len = L ;

    txData.data = (unsigned short*)malloc( sizeof(unsigned short) * L ) ;
    rxData.data = (unsigned short*)malloc( sizeof(unsigned short) * L ) ;

    txSymbol.sig = (COMPLEX*)malloc( sizeof(COMPLEX) * L ) ;
    rxSymbol.sig = (COMPLEX*)malloc( sizeof(COMPLEX) * L ) ;
    noise.sig = (COMPLEX*)malloc( sizeof(COMPLEX) * L ) ;

    if( txData.data == NULL || rxData.data == NULL
      ||txSymbol.sig == NULL || rxSymbol.sig == NULL || noise.sig == NULL )
    {
        printf("ERROR: failed memory capture.\n") ;
        exit(1) ;
    }
    else
    {
        for(i = 0; i < SNR_STEP; ++i) SNRdB[i] = (double)i ;

        for(i = 0; i < SNR_STEP; ++i)
        {
            srand( time(NULL) ) ;
            MakeRandData( txData ) ;            
            ModulateBpsk( txSymbol, txData ) ;

            double Pn = CalculateSnrDb2NoisePower(SNRdB[i]) ;
            MakeAwgn( noise, Pn ) ;
            SumVector( rxSymbol, txSymbol, noise ) ;

            DemodulateBPSK( rxData, rxSymbol ) ;

            BER[i] = CalculateBER( txData, rxData ) ;
        }
    }
    PrintBER( SNR_STEP, SNRdB, BER) ;

    free(txData.data) ;
    free(rxData.data) ;
    free(txSymbol.sig) ;
    free(rxSymbol.sig) ;
    free(noise.sig) ;

    return 0 ;
}