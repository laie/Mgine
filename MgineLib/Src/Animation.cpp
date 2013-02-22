#include "StdAfx.hpp"
#include "Animation.h"
#include "Function.h"

using namespace Mgine;

decltype(AnimationManager::iListAnimationPool) AnimationManager::iListAnimationPool;
decltype(AnimationManager::IsAppFinished) AnimationManager::IsAppFinished;

class MonitorAppBeginEnd
{
public:
	MonitorAppBeginEnd()
	{ AnimationManager::RaiseBeginApp(); }
	~MonitorAppBeginEnd()
	{ AnimationManager::RaiseEndApp(); }
} MonitorAppBeginEndInstance;


void AnimationBase::RaiseUpdate(double ElapsedTime)
{
	ATHROW(Duration);

	LastValue = Value;
	if ( IsFinished ) return;

	if ( InitialDelay )
	{
		InitialDelay -= ElapsedTime;
		if ( InitialDelay < 0 )
		{
			ElapsedTime = -InitialDelay;
			InitialDelay = 0;
		} else return;
	}

	CurrentProgress += ElapsedTime;
	while ( CurrentProgress >= Duration )
	{
		CurrentProgress -= Duration;
		if ( RepeatCount >= 0 ) // limitness repeat
		{
			CurrentRepeat++;
			if ( RepeatCount <= CurrentRepeat )
			{
				IsFinished = true;
				Value = Weight;//OnUpdate((VEC)1);
				return;
			}
		} else if ( RepeatCount == -1 ) // infinite repeat
		{

		} else NRTHROW();
	}

	if ( ElapsedTime ) Value = OnUpdate((VEC)(CurrentProgress / Duration));
}

void AnimationBase::SetOwner(AnimationPool *Owner)
{ this->Owner = Owner; }


VEC AnimationLambda::OnUpdate(VEC Degree)
{ return ProcUpdate(Degree); }

VEC AnimationMoveSmooth::OnUpdate(VEC Degree)
{ return MF::CurveIncreaseA(Degree, (VEC)1.5)*Weight; }

VEC AnimationMoveRude::OnUpdate(VEC Degree)
{
	return sin(MF::CurveIncreaseA(Degree, (VEC)4)*MF::PI_F)*Weight*(VEC)0.5
			+MF::CurveIncreaseA(Degree, (VEC)2)*Weight;
}
