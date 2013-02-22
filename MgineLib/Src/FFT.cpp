#include "StdAfx.hpp"
#include "FFT.h"
using namespace Mgine;
using namespace Util;

#   define KISS_FFT_S_MUL(a,b) ( (a)*(b) )
#   define C_MUL(m,a,b) \
    do{ (m).r = (a).r*(b).r - (a).i*(b).i;\
        (m).i = (a).r*(b).i + (a).i*(b).r; }while(0)
#   define KISS_FFT_C_FIXDIV(c,div) /* NOOP */
#   define KISS_FFT_C_MULBYSCALAR( c, s ) \
    do{ (c).r *= (s);\
        (c).i *= (s); }while(0)

#ifndef KISS_FFT_CHECK_OVERFLOW_OP
#  define KISS_FFT_CHECK_OVERFLOW_OP(a,op,b) /* noop */
#endif

#define  KISS_FFT_C_ADD( res, a,b)\
    do { \
	    KISS_FFT_CHECK_OVERFLOW_OP((a).r,+,(b).r)\
	    KISS_FFT_CHECK_OVERFLOW_OP((a).i,+,(b).i)\
	    (res).r=(a).r+(b).r;  (res).i=(a).i+(b).i; \
    }while(0)
#define  C_SUB( res, a,b)\
    do { \
	    KISS_FFT_CHECK_OVERFLOW_OP((a).r,-,(b).r)\
	    KISS_FFT_CHECK_OVERFLOW_OP((a).i,-,(b).i)\
	    (res).r=(a).r-(b).r;  (res).i=(a).i-(b).i; \
    }while(0)
#define KISS_FFT_C_ADDTO( res , a)\
    do { \
	    KISS_FFT_CHECK_OVERFLOW_OP((res).r,+,(a).r)\
	    KISS_FFT_CHECK_OVERFLOW_OP((res).i,+,(a).i)\
	    (res).r += (a).r;  (res).i += (a).i;\
    }while(0)

#define KISS_FFT_C_SUBFORM( res , a)\
    do {\
	    KISS_FFT_CHECK_OVERFLOW_OP((res).r,-,(a).r)\
	    KISS_FFT_CHECK_OVERFLOW_OP((res).i,-,(a).i)\
	    (res).r -= (a).r;  (res).i -= (a).i; \
    } while(0)


#  define KISS_FFT_COS(phase) (SCALAR) cos(phase)
#  define KISS_FFT_SIN(phase) (SCALAR) sin(phase)
#  define KISS_FFT_HALF_OF(x) ((x)*.5)

#define  KISS_FFT_KF_CEXP(x,phase) \
	do{ \
		(x)->r = KISS_FFT_COS(phase);\
		(x)->i = KISS_FFT_SIN(phase);\
	}while(0)


namespace FFT
{
	static void kf_bfly2(
			COMPLEX * Fout,
			const size_t fstride,
			const FFTCFG st,
			int m
			)
	{
		COMPLEX * Fout2;
		COMPLEX * tw1 = st->twiddles;
		COMPLEX t;
		Fout2 = Fout + m;
		do{
			KISS_FFT_C_FIXDIV(*Fout,2); KISS_FFT_C_FIXDIV(*Fout2,2);

			C_MUL (t,  *Fout2 , *tw1);
			tw1 += fstride;
			C_SUB( *Fout2 ,  *Fout , t );
			KISS_FFT_C_ADDTO( *Fout ,  t );
			++Fout2;
			++Fout;
		}while (--m);
	}

	static void kf_bfly4(
			COMPLEX * Fout,
			const size_t fstride,
			const FFTCFG st,
			const size_t m
			)
	{
		COMPLEX *tw1,*tw2,*tw3;
		COMPLEX scratch[6];
		size_t k=m;
		const size_t m2=2*m;
		const size_t m3=3*m;


		tw3 = tw2 = tw1 = st->twiddles;

		do {
			KISS_FFT_C_FIXDIV(*Fout,4); KISS_FFT_C_FIXDIV(Fout[m],4); KISS_FFT_C_FIXDIV(Fout[m2],4); KISS_FFT_C_FIXDIV(Fout[m3],4);

			C_MUL(scratch[0],Fout[m] , *tw1 );
			C_MUL(scratch[1],Fout[m2] , *tw2 );
			C_MUL(scratch[2],Fout[m3] , *tw3 );

			C_SUB( scratch[5] , *Fout, scratch[1] );
			KISS_FFT_C_ADDTO(*Fout, scratch[1]);
			KISS_FFT_C_ADD( scratch[3] , scratch[0] , scratch[2] );
			C_SUB( scratch[4] , scratch[0] , scratch[2] );
			C_SUB( Fout[m2], *Fout, scratch[3] );
			tw1 += fstride;
			tw2 += fstride*2;
			tw3 += fstride*3;
			KISS_FFT_C_ADDTO( *Fout , scratch[3] );

			if(st->inverse) {
				Fout[m].r = scratch[5].r - scratch[4].i;
				Fout[m].i = scratch[5].i + scratch[4].r;
				Fout[m3].r = scratch[5].r + scratch[4].i;
				Fout[m3].i = scratch[5].i - scratch[4].r;
			}else{
				Fout[m].r = scratch[5].r + scratch[4].i;
				Fout[m].i = scratch[5].i - scratch[4].r;
				Fout[m3].r = scratch[5].r - scratch[4].i;
				Fout[m3].i = scratch[5].i + scratch[4].r;
			}
			++Fout;
		}while(--k);
	}

	static void kf_bfly3(
			 COMPLEX * Fout,
			 const size_t fstride,
			 const FFTCFG st,
			 size_t m
			 )
	{
		 size_t k=m;
		 const size_t m2 = 2*m;
		 COMPLEX *tw1,*tw2;
		 COMPLEX scratch[5];
		 COMPLEX epi3;
		 epi3 = st->twiddles[fstride*m];

		 tw1=tw2=st->twiddles;

		 do{
			 KISS_FFT_C_FIXDIV(*Fout,3); KISS_FFT_C_FIXDIV(Fout[m],3); KISS_FFT_C_FIXDIV(Fout[m2],3);

			 C_MUL(scratch[1],Fout[m] , *tw1);
			 C_MUL(scratch[2],Fout[m2] , *tw2);

			 KISS_FFT_C_ADD(scratch[3],scratch[1],scratch[2]);
			 C_SUB(scratch[0],scratch[1],scratch[2]);
			 tw1 += fstride;
			 tw2 += fstride*2;

			 Fout[m].r = Fout->r - KISS_FFT_HALF_OF(scratch[3].r);
			 Fout[m].i = Fout->i - KISS_FFT_HALF_OF(scratch[3].i);

			 KISS_FFT_C_MULBYSCALAR( scratch[0] , epi3.i );

			 KISS_FFT_C_ADDTO(*Fout,scratch[3]);

			 Fout[m2].r = Fout[m].r + scratch[0].i;
			 Fout[m2].i = Fout[m].i - scratch[0].r;

			 Fout[m].r -= scratch[0].i;
			 Fout[m].i += scratch[0].r;

			 ++Fout;
		 }while(--k);
	}

	static void kf_bfly5(
			COMPLEX * Fout,
			const size_t fstride,
			const FFTCFG st,
			int m
			)
	{
		COMPLEX *Fout0,*Fout1,*Fout2,*Fout3,*Fout4;
		int u;
		COMPLEX scratch[13];
		COMPLEX * twiddles = st->twiddles;
		COMPLEX *tw;
		COMPLEX ya,yb;
		ya = twiddles[fstride*m];
		yb = twiddles[fstride*2*m];

		Fout0=Fout;
		Fout1=Fout0+m;
		Fout2=Fout0+2*m;
		Fout3=Fout0+3*m;
		Fout4=Fout0+4*m;

		tw=st->twiddles;
		for ( u=0; u<m; ++u ) {
			KISS_FFT_C_FIXDIV( *Fout0,5); KISS_FFT_C_FIXDIV( *Fout1,5); KISS_FFT_C_FIXDIV( *Fout2,5); KISS_FFT_C_FIXDIV( *Fout3,5); KISS_FFT_C_FIXDIV( *Fout4,5);
			scratch[0] = *Fout0;

			C_MUL(scratch[1] ,*Fout1, tw[u*fstride]);
			C_MUL(scratch[2] ,*Fout2, tw[2*u*fstride]);
			C_MUL(scratch[3] ,*Fout3, tw[3*u*fstride]);
			C_MUL(scratch[4] ,*Fout4, tw[4*u*fstride]);

			KISS_FFT_C_ADD( scratch[7],scratch[1],scratch[4]);
			C_SUB( scratch[10],scratch[1],scratch[4]);
			KISS_FFT_C_ADD( scratch[8],scratch[2],scratch[3]);
			C_SUB( scratch[9],scratch[2],scratch[3]);

			Fout0->r += scratch[7].r + scratch[8].r;
			Fout0->i += scratch[7].i + scratch[8].i;

			scratch[5].r = scratch[0].r + KISS_FFT_S_MUL(scratch[7].r,ya.r) + KISS_FFT_S_MUL(scratch[8].r,yb.r);
			scratch[5].i = scratch[0].i + KISS_FFT_S_MUL(scratch[7].i,ya.r) + KISS_FFT_S_MUL(scratch[8].i,yb.r);

			scratch[6].r =  KISS_FFT_S_MUL(scratch[10].i,ya.i) + KISS_FFT_S_MUL(scratch[9].i,yb.i);
			scratch[6].i = -KISS_FFT_S_MUL(scratch[10].r,ya.i) - KISS_FFT_S_MUL(scratch[9].r,yb.i);

			C_SUB(*Fout1,scratch[5],scratch[6]);
			KISS_FFT_C_ADD(*Fout4,scratch[5],scratch[6]);

			scratch[11].r = scratch[0].r + KISS_FFT_S_MUL(scratch[7].r,yb.r) + KISS_FFT_S_MUL(scratch[8].r,ya.r);
			scratch[11].i = scratch[0].i + KISS_FFT_S_MUL(scratch[7].i,yb.r) + KISS_FFT_S_MUL(scratch[8].i,ya.r);
			scratch[12].r = - KISS_FFT_S_MUL(scratch[10].i,yb.i) + KISS_FFT_S_MUL(scratch[9].i,ya.i);
			scratch[12].i = KISS_FFT_S_MUL(scratch[10].r,yb.i) - KISS_FFT_S_MUL(scratch[9].r,ya.i);

			KISS_FFT_C_ADD(*Fout2,scratch[11],scratch[12]);
			C_SUB(*Fout3,scratch[11],scratch[12]);

			++Fout0;++Fout1;++Fout2;++Fout3;++Fout4;
		}
	}

	/* perform the butterfly for one stage of a mixed radix FFT */
	static void kf_bfly_generic(
			COMPLEX * Fout,
			const size_t fstride,
			const FFTCFG st,
			int m,
			int p
			)
	{
		int u,k,q1,q;
		COMPLEX * twiddles = st->twiddles;
		COMPLEX t;
		int Norig = st->nfft;

		COMPLEX * scratch = (COMPLEX*)new COMPLEX[p];

		for ( u=0; u<m; ++u ) {
			k=u;
			for ( q1=0 ; q1<p ; ++q1 ) {
				scratch[q1] = Fout[ k ];
				KISS_FFT_C_FIXDIV(scratch[q1],p);
				k += m;
			}

			k=u;
			for ( q1=0 ; q1<p ; ++q1 ) {
				int twidx=0;
				Fout[ k ] = scratch[0];
				for (q=1;q<p;++q ) {
					twidx += fstride * k;
					if (twidx>=Norig) twidx-=Norig;
					C_MUL(t,scratch[q] , twiddles[twidx] );
					KISS_FFT_C_ADDTO( Fout[ k ] ,t);
				}
				k += m;
			}
		}
		delete[] scratch;
	}

	static
	void kf_work(
			COMPLEX * Fout,
			const COMPLEX * f,
			const size_t fstride,
			int in_stride,
			int * factors,
			const FFTCFG st
			)
	{
		COMPLEX * Fout_beg=Fout;
		const int p=*factors++; /* the radix  */
		const int m=*factors++; /* stage's fft length/p */
		const COMPLEX * Fout_end = Fout + p*m;

		if (m==1) {
			do{
				*Fout = *f;
				f += fstride*in_stride;
			}while(++Fout != Fout_end );
		}else{
			do{
				// recursive call:
				// DFT of size m*p performed by doing
				// p instances of smaller DFTs of size m, 
				// each one takes a decimated version of the input
				kf_work( Fout , f, fstride*p, in_stride, factors,st);
				f += fstride*in_stride;
			}while( (Fout += m) != Fout_end );
		}

		Fout=Fout_beg;

		// recombine the p smaller DFTs 
		switch (p) {
			case 2: kf_bfly2(Fout,fstride,st,m); break;
			case 3: kf_bfly3(Fout,fstride,st,m); break; 
			case 4: kf_bfly4(Fout,fstride,st,m); break;
			case 5: kf_bfly5(Fout,fstride,st,m); break; 
			default: kf_bfly_generic(Fout,fstride,st,m,p); break;
		}
	}

	/*  facbuf is populated by p1,m1,p2,m2, ...
		where 
		p[i] * m[i] = m[i-1]
		m0 = n                  */
	static 
	void kf_factor(int n,int * facbuf)
	{
		int p=4;
		double floor_sqrt;
		floor_sqrt = floor( sqrt((double)n) );

		/*factor out powers of 4, powers of 2, then any remaining primes */
		do {
			while (n % p) {
				switch (p) {
					case 4: p = 2; break;
					case 2: p = 3; break;
					default: p += 2; break;
				}
				if (p > floor_sqrt)
					p = n;          /* no more factors, skip to end */
			}
			n /= p;
			*facbuf++ = p;
			*facbuf++ = n;
		} while (n > 1);
	}

	/*
	 *
	 * User-callable function to allocate all necessary storage space for the fft.
	 *
	 * The return value is a contiguous block of memory, allocated with malloc.  As such,
	 * It can be freed with free(), rather than a kiss_fft-specific function.
	 * */
	FFTCFG FFT::kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem )
	{
		FFTCFG st=NULL;
		size_t memneeded = sizeof(struct FFT_STATE)
			+ sizeof(COMPLEX)*(nfft-1); /* twiddle factors*/

		if ( lenmem==NULL ) {
			st = ( FFTCFG )new char[memneeded];
		}else{
			if (mem != NULL && *lenmem >= memneeded)
				st = (FFTCFG)mem;
			*lenmem = memneeded;
		}
		if (st) {
			int i;
			st->nfft=nfft;
			st->inverse = inverse_fft;

			for (i=0;i<nfft;++i) {
				const double pi=3.141592653589793238462643383279502884197169399375105820974944;
				double phase = -2*pi*i / nfft;
				if (st->inverse)
					phase *= -1;
				KISS_FFT_KF_CEXP(st->twiddles+i, phase );
			}

			kf_factor(nfft,st->factors);
		}
		return st;
	}


	void FFT::kiss_fft_stride(FFTCFG st,const COMPLEX *fin,COMPLEX *fout,int in_stride)
	{
		if (fin == fout) {
			//NOTE: this is not really an in-place FFT algorithm.
			//It just performs an out-of-place FFT into a temp buffer
			COMPLEX * tmpbuf = new COMPLEX[st->nfft];
			kf_work(tmpbuf,fin,1,in_stride, st->factors,st);
			memcpy(fout,tmpbuf,sizeof(COMPLEX)*st->nfft);
			delete[] tmpbuf;
		}else{
			kf_work( fout, fin, 1,in_stride, st->factors,st );
		}
	}

	void FFT::kiss_fft(FFTCFG cfg,const COMPLEX *fin, COMPLEX *fout)
	{
		kiss_fft_stride(cfg,fin,fout,1);
	}



	int kiss_fft_next_fast_size(int n)
	{
		while(1) {
			int m=n;
			while ( (m%2) == 0 ) m/=2;
			while ( (m%3) == 0 ) m/=3;
			while ( (m%5) == 0 ) m/=5;
			if (m<=1)
				break; /* n is completely factorable by twos, threes, and fives */
			n++;
		}
		return n;
	}


	/* fft real starts */

	RFFT_CFG FFT::kiss_fftr_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem)
	{
		int i;
		RFFT_CFG st = NULL;
		size_t subsize, memneeded;

		if (nfft & 1) {
			// Read FFT optimization must be power of 2
			return NULL;
		}
		nfft >>= 1;

		kiss_fft_alloc (nfft, inverse_fft, NULL, &subsize);
		memneeded = sizeof(RFFT_STATE) + subsize + sizeof(COMPLEX) * ( nfft * 3 / 2);

		if (lenmem == NULL) {
			st = (RFFT_CFG) new BYTE[memneeded];
		} else {
			if (*lenmem >= memneeded)
				st = (RFFT_CFG) mem;
			*lenmem = memneeded;
		}
		if (!st)
			return NULL;

		st->substate = (FFTCFG) (st + 1); /*just beyond RFFT_STATE struct */
		st->tmpbuf = (COMPLEX *) (((char *) st->substate) + subsize);
		st->super_twiddles = st->tmpbuf + nfft;
		kiss_fft_alloc(nfft, inverse_fft, st->substate, &subsize);

		for (i = 0; i < nfft/2; ++i) {
			double phase =
				-3.14159265358979323846264338327 * ((double) (i+1) / nfft + .5);
			if (inverse_fft)
				phase *= -1;
			KISS_FFT_KF_CEXP (st->super_twiddles+i,phase);
		}
		return st;
	}

	void FFT::kiss_fftr(RFFT_CFG st,const SCALAR *timedata,COMPLEX *freqdata)
	{
		/* input buffer timedata is stored row-wise */
		int k,ncfft;
		COMPLEX fpnk,fpk,f1k,f2k,tw,tdc;

		if ( st->substate->inverse ) {
			//fprintf(stderr,"kiss fft usage error: improper alloc\n");
			//exit(1);
			// Improper alloc
			__asm int 3;
		}

		ncfft = st->substate->nfft;

		/*perform the parallel fft of two real signals packed in real,imag*/
		kiss_fft( st->substate , (const COMPLEX*)timedata, st->tmpbuf );
		/* The real part of the DC element of the frequency spectrum in st->tmpbuf
		 * contains the sum of the even-numbered elements of the input time sequence
		 * The imag part is the sum of the odd-numbered elements
		 *
		 * The sum of tdc.r and tdc.i is the sum of the input time sequence. 
		 *      yielding DC of input time sequence
		 * The difference of tdc.r - tdc.i is the sum of the input (dot product) [1,-1,1,-1... 
		 *      yielding Nyquist bin of input time sequence
		 */
 
		tdc.r = st->tmpbuf[0].r;
		tdc.i = st->tmpbuf[0].i;
		KISS_FFT_C_FIXDIV(tdc,2);
		KISS_FFT_CHECK_OVERFLOW_OP(tdc.r ,+, tdc.i);
		KISS_FFT_CHECK_OVERFLOW_OP(tdc.r ,-, tdc.i);
		freqdata[0].r = tdc.r + tdc.i;
		freqdata[ncfft].r = tdc.r - tdc.i;
		freqdata[ncfft].i = freqdata[0].i = 0;

		for ( k=1;k <= ncfft/2 ; ++k ) {
			fpk    = st->tmpbuf[k]; 
			fpnk.r =   st->tmpbuf[ncfft-k].r;
			fpnk.i = - st->tmpbuf[ncfft-k].i;
			KISS_FFT_C_FIXDIV(fpk,2);
			KISS_FFT_C_FIXDIV(fpnk,2);

			KISS_FFT_C_ADD( f1k, fpk , fpnk );
			C_SUB( f2k, fpk , fpnk );
			C_MUL( tw , f2k , st->super_twiddles[k-1]);

			freqdata[k].r = KISS_FFT_HALF_OF(f1k.r + tw.r);
			freqdata[k].i = KISS_FFT_HALF_OF(f1k.i + tw.i);
			freqdata[ncfft-k].r = KISS_FFT_HALF_OF(f1k.r - tw.r);
			freqdata[ncfft-k].i = KISS_FFT_HALF_OF(tw.i - f1k.i);
		}
	}

	void FFT::kiss_fftri(RFFT_CFG st,const COMPLEX *freqdata,SCALAR *timedata)
	{
		/* input buffer timedata is stored row-wise */
		int k, ncfft;

		if (st->substate->inverse == 0) {
			//fprintf (stderr, "kiss fft usage error: improper alloc\n");
			//exit (1);
			// Improper alloc
			__asm int 3;
		}

		ncfft = st->substate->nfft;

		st->tmpbuf[0].r = freqdata[0].r + freqdata[ncfft].r;
		st->tmpbuf[0].i = freqdata[0].r - freqdata[ncfft].r;
		KISS_FFT_C_FIXDIV(st->tmpbuf[0],2);

		for (k = 1; k <= ncfft / 2; ++k) {
			COMPLEX fk, fnkc, fek, fok, tmp;
			fk = freqdata[k];
			fnkc.r = freqdata[ncfft - k].r;
			fnkc.i = -freqdata[ncfft - k].i;
			KISS_FFT_C_FIXDIV( fk , 2 );
			KISS_FFT_C_FIXDIV( fnkc , 2 );

			KISS_FFT_C_ADD (fek, fk, fnkc);
			C_SUB (tmp, fk, fnkc);
			C_MUL (fok, tmp, st->super_twiddles[k-1]);
			KISS_FFT_C_ADD (st->tmpbuf[k],     fek, fok);
			C_SUB (st->tmpbuf[ncfft - k], fek, fok);
			st->tmpbuf[ncfft - k].i *= -1;
		}
		kiss_fft (st->substate, st->tmpbuf, (COMPLEX *) timedata);
	}
	/* fft real ends */
}

void FFTPlan::ResultToPowerSpectrum(FFT::COMPLEX *ArrComplex, FFT::SCALAR *ArrResult, int SpectrumCount)
{
	ATHROW(ArrResult);
	ATHROW(ArrComplex);

	for ( int i=0; i < SpectrumCount; i++ )
		ArrResult[i] = sqrt(ArrComplex[i].r*ArrComplex[i].r+ArrComplex[i].i*ArrComplex[i].i);
}

void FFTPlan::ResultToCorrelation(FFT::COMPLEX *ArrComplex, FFT::SCALAR *ArrResult, int SpectrumCount)
{
	ATHROW(ArrResult);
	ATHROW(ArrComplex);

	for ( int i=0; i < SpectrumCount; i++ )
	{
		if ( !i ) ArrResult[i] = 0;
		else
		{
			double re1 = ArrComplex[i].r, re2 = ArrComplex[i+1].r,
				im1 = ArrComplex[i].i, im2 = ArrComplex[i+1].i;
			double re = (re1*re2 + im1*im2),
				im = (re2*im1 - re1*im2);
			//double out = sqrt(re*re+im*im);
			//re /= out;
			//im /= out;
			ArrResult[i] = (re*re+im*im)/**0.01*/;
		}
	}
}

FFTPlan::~FFTPlan()
{ Uninit(); }

void FFTPlan::Init(int SampleCount)
{
	Uninit();
	try
	{
		ATHROW(SampleCount % 2 == 0);
		pRFFT = FFT::kiss_fftr_alloc(SampleCount, 0, 0, 0);
		ATHROW(pRFFT);
		Result.Set(new FFT::COMPLEX[SampleCount/2+1]);
	}
	catch ( BaseException & )
	{
		Uninit();
	}
}

void FFTPlan::Uninit()
{
	if ( pRFFT ) delete[] (BYTE*) pRFFT;
	pRFFT = NULL;
	Result = NULL;
}

void FFTPlan::Process(FFT::SCALAR *Data)
{
	ATHROW(pRFFT && pRFFT->substate);
	ATHROW(Data);
	ATHROW(Result.Arr);

	FFT::kiss_fftr(pRFFT, Data, Result.Arr);
}

void FFTPlan::PowerSpectrum(double *ArrResult)
{
	ATHROW(pRFFT);
	ATHROW(ArrResult);
	return ResultToPowerSpectrum(Result.Arr, ArrResult, GetSpectrumCount());
}

double FFTPlan::IndexToFreq(unsigned int BaseFrequency, int Index)
{
	ATHROW(pRFFT && pRFFT->substate);

	if ( Index >= pRFFT->substate->nfft || Index < 0 ) return 0.0;
	else return ( (double)Index / ((double)GetSpectrumCount()*2) * BaseFrequency );
		//return ( -(double)(pRFFT->substate->nfft/2-Index) / (double)pRFFT->substate->nfft/2 * BaseFrequency );
}

/*DWORD FFTPlan::GetNFFT() const
{
	if ( !(pRFFT && pRFFT->substate) ) return 0;
	return pRFFT->substate->nfft*2;
}*/

DWORD FFTPlan::GetSampleCount() const
{
	ATHROW(pRFFT && pRFFT->substate);
	return pRFFT->substate->nfft*2;
}
DWORD FFTPlan::GetSpectrumCount() const
{
	ATHROW(pRFFT && pRFFT->substate);
	return pRFFT->substate->nfft;
}

