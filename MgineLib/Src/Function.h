#pragma once

//#define ABS(a) ((a) < 0 ? -(a) : (a))
//#define SGN(a) ((a) < 0 ? -1 : ( (a) > 0 ? 1 : 0 ))

namespace Mgine
{
	class Func
	{
	private:
		template<typename T> static bool RCSortLarge(T *Arr, int *ArrIndex, int Left, int Count, int Depth);

	public:
		typedef DWORD (__stdcall *pfZwDelayExecution)(BOOLEAN, __int64*); 

		static const double	SQRT2;	// = 1.4142135623730950488016887242096980785696718753769480;
		static const double PI;		// = 3.14159265358979323846;
		static const float PI_F;	// = (float)PI;

		static inline double GetDoubleTick(){ return GetDoubleTick(0); }
		static inline double GetDoubleTick(LONGLONG Frequency)
		{
			LONGLONG Freq;
			if ( Frequency ) Freq = Frequency;
			else QueryPerformanceFrequency((LARGE_INTEGER*)&Freq);
			if ( !Freq ) return 0;
			LONGLONG counter = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&counter);
			double ret = 1000.;
			ret *= counter;
			ret /= Freq;
			return ret;
		}

		static inline ULONGLONG GetLongTick()
		{
			static DWORD LastTick = timeGetTime();//GetTickCount();
			static DWORD Stacked = 0;
			ULONGLONG ret = timeGetTime();
			if ( LastTick+(ULONGLONG)Stacked*4294967296 > ret ) Stacked ++;
			return ret + (ULONGLONG)Stacked*4294967296;
		}


		template<typename T> inline static T Theta(T X, T Y);
		template<typename T> inline static T Theta(T X, T Y, T CenterX, T CenterY);
		template<typename T> inline static T Distance(T X, T Y);
		template<typename T> inline static T Distance(T X1, T Y1, T X2, T Y2);
		template<typename T> inline static T NearestCycle(T Value, T Target, T Mod);

		template<typename T> inline static T CurveIncreaseA(const T& Progress, const T& Multiply);
		template<typename T> inline static T CurveCycleA(T Progress, T Multiply);

		template<typename T> inline static T Min(T Value, T Min)
		{ if ( Value < Min ) return (T)Value; else return (T)Min; }
		template<typename T> inline static T Max(T Value, T Max)
		{ if ( Value > Max ) return (T)Value; else return (T)Max; }
		template<typename T> inline static T MinMax(T Value, T Min, T Max)
		{ if ( Max < Value ) Value = (T)Max; if ( Value < Min ) Value = (T)Min; return (T)Value; }
		template<typename T> inline static T Clamp(T Value, T Min, T Max)
		{ if ( Max < Value ) Value = Max; if ( Value < Min ) Value = Min; return Value; }
		template<typename T> inline static T Abs(T Value)
		{ if ( Value < 0 ) return -Value; else return Value; }
		template<typename T> inline static T Sgn(T Value)
		{ if ( Value < 0 ) return -1; else if ( Value == 0 ) return 0; else return 1; }

		template<typename T> inline static void Excg(T &A, T &B) // Exchange
		{ T temp; temp = A; A = B; B = temp; }

		template<typename T> inline static T Log(T Base, T X)
		{ return log10(X)/log10(Base); }

		template<typename T> static void SortLarge(T *Arr, int *ArrIndex, int Size);

		static inline void BreakExecution()
		{
			#ifdef _DEBUG
			__asm int 3;
			#endif
		}

		static inline bool DelayExecution(float Period)
		{
			static pfZwDelayExecution ZwDelayExecution;
			if ( !ZwDelayExecution ) 
				ZwDelayExecution = (pfZwDelayExecution) GetProcAddress(GetModuleHandle(L"ntdll.dll"), "ZwDelayExecution");
			if ( !ZwDelayExecution ) BreakExecution(); //return false;
			LONGLONG timedelay = (LONGLONG)(-10000*Period);
			return ZwDelayExecution(true, &timedelay) == 0;
		}
	};
	typedef Func MF;


	template<typename T> bool MF::RCSortLarge(T *Arr, int *ArrIndex, int Left, int Count, int Depth)
	{
	start:
		int i = Left, p = Left + Count - 1;
		T pivot = Arr[Left + Count/2];
		int indexPivot = Left + Count/2;
		register bool sorted = true;

		if ( Depth >= 100 ) return false; // Prevent for stackoverflow
		if ( Left < 0 ) return true;
		if ( Count <= 1 ) return true;

		for (;;){
			for ( ; i <= p && pivot <= Arr[p]; p-- ){ // 오른쪽부터 pivot보다 작은값을 찾아
				if ( p-1 >= Left && Arr[p-1] > Arr[p] ){
					MF::Excg(Arr[p-1], Arr[p]);
					if ( ArrIndex ) MF::Excg(ArrIndex[p-1], ArrIndex[p]);

					if ( p-1 == indexPivot ) indexPivot = p;
					else if ( p == indexPivot ) indexPivot = p-1;

					sorted = false;
					if ( !(pivot <= Arr[p]) ){
						break;
					} else {
						p--;
						continue;
					}
				}
			}
			if ( p-1 >= Left && Arr[p-1] > Arr[p] ) sorted = false; 

			for ( ; i <= p && pivot >= Arr[i]; i++ ){ // 왼쪽부터 pivot보다 큰값 찾아
				if ( sorted && i+1 < Left+Count && Arr[i] > Arr[i+1] ){
					sorted = false;
				}
			}
			if ( sorted && i+1 < Left+Count && Arr[i] > Arr[i+1] ) sorted = false;

			if ( i < p ){
				//교차 안했으면 바꾼다.
				MF::Excg(Arr[i], Arr[p]);
				if ( ArrIndex ) MF::Excg(ArrIndex[i], ArrIndex[p]);

				if ( i == indexPivot ) indexPivot = p;
				else if ( p == indexPivot ) indexPivot = i;

			} else {
				if ( Left > p ){
					//교차했는데 p와 피봇이 다름. 피봇을 교환하고 break
					MF::Excg(Arr[Left], Arr[indexPivot]);
					if ( ArrIndex ) MF::Excg(ArrIndex[Left], ArrIndex[indexPivot]);
				
					indexPivot = Left;

					Left++;
					Count--;
					goto start;
				} else if ( i >= Left + Count ){
					MF::Excg(Arr[Left+Count-1], Arr[indexPivot]);
					if ( ArrIndex ) MF::Excg(ArrIndex[Left+Count-1], ArrIndex[indexPivot]);

					indexPivot = Left + Count - 1;

					Count--;
					goto start;

				}

				if ( sorted ) return true;
				if ( indexPivot < p ) {
					MF::Excg(Arr[p], Arr[indexPivot]);
					if ( ArrIndex ) MF::Excg(ArrIndex[p], ArrIndex[indexPivot]);
					indexPivot = p;

					if ( !RCSortLarge(Arr, ArrIndex, Left, p - Left, Depth + 1) ) return false;
					if ( !RCSortLarge(Arr, ArrIndex, p + 1, Left+Count - p-1, Depth + 1) ) return false;
				} else {
					MF::Excg(Arr[p+1], Arr[indexPivot]);
					if ( ArrIndex ) MF::Excg(ArrIndex[p+1], ArrIndex[indexPivot]);
					indexPivot = p+1;

					if ( !RCSortLarge(Arr, ArrIndex, Left, p - Left+1, Depth + 1) ) return false;
					if ( !RCSortLarge(Arr, ArrIndex, p + 1+1, Left+Count - p-1-1, Depth + 1) ) return false;
				}
				return true;
				break;
			}
		}
	}

	template<typename T> inline void MF::SortLarge(T *Arr, int *ArrIndex, int Size)
	{
		while ( !RCSortLarge(Arr, ArrIndex, 0, Size, 0) );
	}


	template<typename T> inline T MF::Theta(T X, T Y)
	{
		return Theta(X, Y, (T)0, (T)0);
	}

	template<typename T> T MF::Theta(T X, T Y, T CenterX, T CenterY)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");

		//
		//      3 PI/2
		//        │
		//  PI <─┼─> 0
		//        │
		//      1 PI/2
		//      

		T theta;
		if ( X-CenterX )
		{
			theta = atan((Y-CenterY)/(X-CenterX));
			if ( X < CenterX ) theta += (T)PI;
		} else {
			theta = 0;
			if ( Y < CenterY ) theta = (T)(3./2*PI);
			else theta = (T)(1./2*PI);
		}

		return theta;
	}

	template<typename T> inline T MF::Distance(T X, T Y)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");
		return (T)sqrt((double)X*X + (double)Y*Y);
	}

	template<typename T> inline T MF::Distance(T X1, T Y1, T X2, T Y2)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");
		return (T)sqrt((double)(X1-X2)*(X1-X2)+(double)(Y1-Y2)*(Y1-Y2));
	}

	template<typename T> T MF::NearestCycle(T Value, T Target, T Mod)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");
		int d = 0;
		double min = 0;
		double mt = (Target - fmod(Target, Mod))/Mod;
	
		Value = fmod(Value, Mod);
		min = Abs((mt-2)*Mod+Value-Target);
		d = -2;
		if ( min > Abs((mt-1)*Mod+Value-Target) ){
			min = Abs((mt-1)*Mod+Value-Target);
			d = -1;
		}
		if ( min > Abs((mt)*Mod+Value-Target) ){
			min = Abs((mt)*Mod+Value-Target);
			d = 0;
		}
		if ( min > Abs((mt+1)*Mod+Value-Target) ){
			min = Abs((mt+1)*Mod+Value-Target);
			d = 1;
		}
		if ( min > Abs((mt+2)*Mod+Value-Target) ){
			min = Abs((mt+2)*Mod+Value-Target);
			d = 2;
		}
	
		return (T)((mt+d)*Mod + Value);
	}

	template<typename T> T MF::CurveIncreaseA(const T& Progress, const T& Madness)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");
		T ret;
		if ( Progress < 0 ) return (T)0.;
		if ( Progress > 1 ) return (T)1.;
		if ( Progress < 0.5 )
		{
			ret = (T)pow((double)Progress*2, (double)Madness*2)/2;
		} 
		else 
		{
			ret = (T)(
				1. +
				-pow(1-(Progress-0.5)*2, (double)Madness*2)/2
				);
		}
		return ret;
	}

	template<typename T> T MF::CurveCycleA(T Progress, T Multiply)
	{
		//static_assert((T)0.5 == 0.5, "T should not be integer type.");
		T ret;
		if ( Progress < 0 ) return 0;
		if ( Progress > 1 ) return 0;

		if ( Progress < 0.5 )
			ret = CurveIncreaseA(Progress*2, Multiply);
		else
			ret = CurveIncreaseA(1-(Progress-0.5f)*2, Multiply);
		return ret;
	}
}

