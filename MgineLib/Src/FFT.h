#pragma once

// Thanks to kiss fft.
// And loic fonteneau, too

#include <math.h>
#include <string.h>

namespace FFT 
{
	const int MAXFACTORS = 32;

	typedef double SCALAR;

	typedef struct {
		SCALAR r;
		SCALAR i;
	} COMPLEX;

	typedef struct FFT_STATE 
	{
		int nfft;
		int inverse;
		int factors[2*MAXFACTORS];
		COMPLEX twiddles[1];
	} *FFTCFG;

	/* 
	 *  kiss_fft_alloc
	 *  
	 *  Initialize a FFT (or IFFT) algorithm's cfg/state buffer.
	 *
	 *  typical usage:      FFTCFG mycfg=kiss_fft_alloc(1024,0,NULL,NULL);
	 *
	 *  The return value from fft_alloc is a cfg buffer used internally
	 *  by the fft routine or NULL.
	 *
	 *  If lenmem is NULL, then kiss_fft_alloc will allocate a cfg buffer using malloc.
	 *  The returned value should be free()d when done to avoid memory leaks.
	 *  
	 *  The state can be placed in a user supplied buffer 'mem':
	 *  If lenmem is not NULL and mem is not NULL and *lenmem is large enough,
	 *      then the function places the cfg in mem and the size used in *lenmem
	 *      and returns mem.
	 *  
	 *  If lenmem is not NULL and ( mem is NULL or *lenmem is not large enough),
	 *      then the function returns NULL and places the minimum cfg 
	 *      buffer size in *lenmem.
	 * */

	FFTCFG kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem); 

	/*
	 * kiss_fft(cfg,in_out_buf)
	 *
	 * Perform an FFT on a complex input buffer.
	 * for a forward FFT,
	 * fin should be  f[0] , f[1] , ... ,f[nfft-1]
	 * fout will be   F[0] , F[1] , ... ,F[nfft-1]
	 * Note that each element is complex and can be accessed like
		f[k].r and f[k].i
	 * */
	void kiss_fft(FFTCFG cfg,const COMPLEX *fin,COMPLEX *fout);

	/*
	 A more generic version of the above function. It reads its input from every Nth sample.
	 * */
	void kiss_fft_stride(FFTCFG st,const COMPLEX *fin,COMPLEX *fout,int in_stride);

	/* If kiss_fft_alloc allocated a buffer, it is one contiguous 
	   buffer and can be simply free()d when no longer needed*/

	/*
	 * Returns the smallest integer k, such that k>=n and k has only "fast" factors (2,3,5)
	 */
	int kiss_fft_next_fast_size(int n);

	/* for real ffts, we need an even size */
	#define kiss_fftr_next_fast_size_real(n) \
			(kiss_fft_next_fast_size( ((n)+1)>>1)<<1)


	typedef struct RFFT_STATE
	{
	    FFTCFG substate;
	    COMPLEX * tmpbuf;
	    COMPLEX * super_twiddles;
	} *RFFT_CFG;


	RFFT_CFG kiss_fftr_alloc(int nfft,int inverse_fft,void * mem, size_t * lenmem);
	/*
		nfft must be even
		If you don't care to allocate space, use mem = lenmem = NULL 
	*/


	void kiss_fftr(RFFT_CFG cfg,const SCALAR *timedata,COMPLEX *freqdata);
	/*
		input timedata has nfft scalar points
		output freqdata has nfft/2+1 complex points
	*/

	void kiss_fftri(RFFT_CFG cfg,const COMPLEX *freqdata,SCALAR *timedata);
	/*
		input freqdata has  nfft/2+1 complex points
		output timedata has nfft scalar points
	*/

} 

namespace Mgine
{
	class FFTPlan
	{
	public:
		static void ResultToPowerSpectrum(FFT::COMPLEX *ArrComplex, FFT::SCALAR *ArrResult, int SpectrumCount);
		static void ResultToCorrelation(FFT::COMPLEX *ArrComplex, FFT::SCALAR *ArrResult, int SpectrumCount);

		FFT::RFFT_CFG pRFFT;
		Util::UnwindArray<FFT::COMPLEX> Result;

		inline FFTPlan(){ pRFFT = NULL; }
		~FFTPlan();

		void Init(int nFFT);
		void Uninit();

		void Process(FFT::SCALAR *Data);
		void PowerSpectrum(double *ArrResult);
		double IndexToFreq(unsigned int BaseFrequency, int Index); 

		DWORD GetSampleCount() const;
		DWORD GetSpectrumCount() const;
	};

}
