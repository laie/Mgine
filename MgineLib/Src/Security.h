#pragma once

#pragma warning( disable : 4102 )
#pragma warning( disable : 4748 )

#ifdef _DEBUG
#define OBF(Expr) { Expr }
#define TWIST(Expr) { Expr }
#else

#define TWIST_COUNT(COUNT, Expr) \
	(Mgine::TwistCode<COUNT, (5000+__LINE__*0x54 +__COUNTER__*0x83)> \
	( \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
		[&](){ OBF_SPAG(;) OBF_DB(;) \
			Expr \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		}(); \
		} \
	) \
	);
// Hides the Expr deeply, useful for hide large instructions. Be sure that Expr ends within 5 seconds
#define TWIST(Expr) \
	TWIST_COUNT(10, TWIST_COUNT(10, OBF(Expr)) )


#define _STRIZE(Text) #Text
#define STRIZE(Text) _STRIZE(Text)
#define _WSTRIZE(Text) L ## #Text
#define WSTRIZE(Text) _WSTRIZE(Text)
#define __COUNTER_WSTR__ WSTRIZE(__COUNTER__)

#define _JUST(Text) Text
#define JUST(Text) _JUST(Text)

#define _JOIN(A, B) A ## B
#define JOIN(A, B) _JOIN(A, B)

#define _OBF_DB_COUNTER(C, ...) \
	{ \
	{ \
		__asm jmp JOIN(___dbobj_label___, C) \
		__asm __emit 0xf0 \
		__asm __emit 0x3e \
		__asm __emit 0x81 \
		__asm __emit 0x05 \
		__asm { JOIN(___dbobj_label___, C): } \
	} \
	{ __VA_ARGS__ } \
	}
#define OBF_DB(...) _OBF_DB_COUNTER(__COUNTER__, __VA_ARGS__)
#define _OBF_JMP_COUNTER(C, Expr) \
	{ \
	{ \
	__asm jmp JOIN(___jmpobf_label___, C)-0x4 \
	__asm call dword ptr [eax+0xffff03eb] \
	__asm { JOIN(___jmpobf_label___, C) : } \
	__asm or al,0x90 \
	} \
	{ OBF_DB(Expr) } \
	}
#define OBF_JMP(Expr) _OBF_JMP_COUNTER(__COUNTER__, Expr)
#define _OBF_CALL_COUNTER(C, Expr) \
	{ \
	{ \
	__asm push eax \
	__asm call JOIN(___callobf_label___, C) \
	__asm { JOIN(___callobf_label___, C): } \
	__asm xor eax,eax \
	__asm pop eax \
	__asm pop eax \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, Expr) \
	}
#define OBF_CALL(Expr) _OBF_CALL_COUNTER(__COUNTER__, Expr)
#define _OBF_CALL2_COUNTER(C, Expr) \
	{ \
	{ \
	__asm push JOIN(___callobf2_label___, C)+1+2 \
	__asm call JOIN(___callobf2_label___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm ret \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___callobf2_label___, C): } \
	__asm ret \
	__asm jmp dword ptr [eax+0xffff03eb] \
	__asm and al,0x90 \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, Expr) \
	}
#define OBF_CALL2(Expr) _OBF_CALL2_COUNTER(__COUNTER__, Expr)

#define _OBF_SPAG_COUNTER(C, Expr) \
	{ \
	{ \
	__asm jmp JOIN(___spagobf_label8___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label1___, C): } \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label7___, C) \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label2___, C): } \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label6___, C) \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label3___, C): } \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label5___, C) \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label4___, C): } \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	OBF_DB(Expr) \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_labelout___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label5___, C): } \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label4___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label6___, C): } \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label3___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label7___, C): } \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm align 0x8 \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label2___, C) \
	} \
	_OBF_JMP_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_label8___, C): } \
	} \
	_OBF_CALL_COUNTER(__COUNTER__, ) \
	{ \
	__asm jmp JOIN(___spagobf_label1___, C) \
	} \
	_OBF_CALL2_COUNTER(__COUNTER__, ) \
	{ \
	__asm { JOIN(___spagobf_labelout___, C): } \
	} \
	}
#define OBF_SPAG(Expr) _OBF_SPAG_COUNTER(__COUNTER__, Expr)

// General Obfuscation. Use TWIST instead
#define OBF(Expr) OBF_SPAG(OBF_SPAG(Expr))
#endif // #ifdef _DEBUG

namespace Mgine
{
	class Sec
	{
	private:
	public:
		static const decltype(&GetTickCount) pGetTickCount;
	};

	class BaseTwistCode
	{
	public:
		int FakeFloatA, FakeFloatB;
		int FakeIntA;

		boost::function<void ()> Lambda;

		inline BaseTwistCode(){ }

		inline int MethodDummyA(int A, int B, int C)
		{
			int dum = 45;
			int i, dummy;
			for ( i=0; i < dum; i++ )
				dum -= i;
			dum += (A+B+C)/3 + i+(int)this;
			if ( dum != 0 ) 
				dummy = i;
			else dummy = B;

			FakeIntA = C ^ A ^ B;

			return FakeFloatA = A*B*C*dum;
		}

		inline int MethodDummyB(void *A)
		{
			int dummy = (int)((int)this+(int)A+0x408);
			int i;

			for ( i=1; i < 10; i++ )
			{
				if ( i == 5 )
				{
					if (
						((FakeFloatA*FakeFloatB) == 0 && FakeFloatA != 0 && FakeFloatB != 0 )
						|| ( ((int)A & 0x10) >> 4 == 1 )
						)
						Lambda();
				}

				if ( dummy % i == ((int)A%10)
					&& 6 < i )
					break;
			}
			if ( i == 43 || (((dummy ^ i ^ (int)A) << 16) | (dummy*i*(int)A)) )
			{
				return FakeFloatB = 2222*(dummy & (int)A & i & (int)(dummy*i*(int)A));
			} else FakeFloatB = FakeFloatA;

			RECT rct = { };
			POINT pt = { (int)FakeFloatA, (int)FakeFloatB };
			//PtInRect(&rct, pt);

			return FakeFloatA;
		}

		inline bool MethodDummyC(void *A)
		{
			if ( !A ) return false;
			if ( (FakeFloatA*FakeFloatB)/100 != 0 )
				MethodDummyA(0xADC4F27B, 400, (int)A);
			else MethodDummyB(0x00000000);

			RECT rct = { };
			POINT pt = { (int)A, (int)FakeFloatB };

			return (int)A % 8 == 3;
		}

	};

	template<int TCurrentTwistCount, int TSeed>
	class TwistCode
		: public BaseTwistCode
	{
		static_assert(TCurrentTwistCount > 0, "Invalid TwistCount");

		TwistCode<TCurrentTwistCount-1, TSeed> TwisterChainDown;
		int Rand;
		int MaxTwist;

		inline void Constructor(const decltype(Lambda) & ManagedLambdaParam, int MaxTwist)
		{
			Rand = ((long long)TSeed ^ 0x7be93caf * ((long long)(TSeed+TCurrentTwistCount)*0xf7) % 0x100000000ul) ^ 0xf7e7 ^ TSeed*(1+TCurrentTwistCount*10);
			this->MaxTwist = MaxTwist;
			this->Lambda = ManagedLambdaParam;
		}

	public:
		inline TwistCode(const decltype(Lambda) & ManagedLambdaParam, int A)
			: TwisterChainDown(ManagedLambdaParam, A)
		{
			Constructor(ManagedLambdaParam, A);
		}

		inline TwistCode(const decltype(Lambda) & ManagedLambdaParam)
			: TwisterChainDown(ManagedLambdaParam, TCurrentTwistCount)
		{
			static_assert(TCurrentTwistCount > 6, "should be larger enough");
			Constructor(ManagedLambdaParam, TCurrentTwistCount);
		}

		inline ~TwistCode()
		{
			RECT rct = { };
			POINT pt = { (int)pLambda, (int)FakeFloatB };

			//PtInRect(&rct, pt);

			TwisterChainDown.MethodDummyA(Rand ^ 400*TCurrentTwistCount*TCurrentTwistCount*(MaxTwist+1)+TSeed*0x177, Rand^0xA82C5*TCurrentTwistCount, Rand^(int)pLambda | (0xf2a2c2fd^TSeed*0x273+TSeed*0x572));
			int b = 
				(int)
				(
					(
						(long long)Rand
						^ 0x152EEul
						| ((long long)TCurrentTwistCount+41)
						*((long long)TCurrentTwistCount+1)
						*0x32A0Cul
						+(long long)TSeed*0x173ul
					)
					& (~0x10ul)
				);
			OBF(
				b |= ((MaxTwist/3 == TCurrentTwistCount/* && (CurrentTwistCount % 3 == 0)*/) ? 0x10 : TSeed << 9);
			)

			TwisterChainDown.MethodDummyB((void*)b);
			TwisterChainDown.MethodDummyC((void*)Rand);
		}
	};

	template<int TSeed>
	class TwistCode<0, TSeed>
		: public BaseTwistCode
	{
	public:
		int StartTick;
		inline TwistCode(const decltype(Lambda) & ManagedLambdaParam, int A)
		{
			StartTick = Sec::pGetTickCount();
			this->pLambda = ManagedLambdaParam;
		}
		inline ~TwistCode()
		{
			int tick = Sec::pGetTickCount();
			if ( (tick - StartTick) > 5000 ) __asm int 3;
			delete pLambda;
		}
	};

}

