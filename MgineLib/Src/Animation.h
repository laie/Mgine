#pragma once

namespace Mgine
{
	class AnimationManager;
	class AnimationPool;
	class AnimationBase;

	class AnimationManager
	{
	private:
		static Util::TriList<AnimationPool*> *iListAnimationPool;
		static bool IsAppFinished;

	public:
		static inline void RaiseBeginApp(){ IsAppFinished = false; }
		static inline void RaiseEndApp(){ IsAppFinished = true; delete iListAnimationPool; iListAnimationPool = NULL; }

		static inline Util::TriList<AnimationPool*> & ListAnimationPool()
		{
			if ( !IsAppFinished && !iListAnimationPool )
				iListAnimationPool = new Util::TriList<AnimationPool*>;
			return *iListAnimationPool;
		}

		//static void Update(double ElapsedTime);
	};


	class AnimationBase
	{
	private:
	protected:
		AnimationPool *Owner;

		double InitialDelay;
		double Duration;
		double CurrentProgress;
		int RepeatCount;
		int CurrentRepeat;

		VEC Weight; // total move distance estimated
		VEC LastValue;
		VEC Value;

		bool IsFinished;

		inline AnimationBase(VEC Weight, int InitialDelay, int Duration, int RepeatCount)
			: Weight(Weight), InitialDelay(InitialDelay), Duration(Duration), RepeatCount(RepeatCount)
		{
			this->Owner = NULL;
			this->CurrentProgress = 0;
			this->CurrentRepeat = 0;
			this->Value = (VEC)0;
			this->IsFinished = false;
		}

		virtual VEC OnUpdate(VEC Degree){ return 0; }

	public:
		virtual ~AnimationBase(){ }

		virtual void SetOwner(AnimationPool *Owner);

		virtual double GetDuration() const { return Duration; }  
		virtual double GetCurrentProgress() const { return CurrentProgress; }
		virtual VEC GetWeight() const { return Weight; }
		virtual VEC GetValue() const { return Value; }
		virtual VEC GetLastValue() const { return LastValue; }
		virtual bool GetIsFinished() const { return IsFinished; }
		virtual int GetRepeatCount() const { return RepeatCount; }

		virtual void RaiseUpdate(double ElapsedTime);
	};

	class AnimationLambda : public AnimationBase
	{
	protected:
		boost::function<VEC (VEC)> ProcUpdate;

	public:
		inline AnimationLambda
			(	int InitialDelay,
				int Duration,
				const boost::function<VEC (VEC)> & ProcUpdate
			) : AnimationBase(ProcUpdate((VEC)1), InitialDelay, Duration, 1)
		{ this->ProcUpdate = ProcUpdate; }
		
		inline AnimationLambda
			(	int Duration,
				const boost::function<VEC (VEC)> & ProcUpdate
			) : AnimationBase(ProcUpdate((VEC)1), 0, Duration, 1)
		{ this->ProcUpdate = ProcUpdate; }

		virtual VEC OnUpdate(VEC Degree);
	};

	class AnimationMoveSmooth : public AnimationBase
	{
	public:
		inline AnimationMoveSmooth(VEC Weight, int InitialDelay, int Duration)
			: AnimationBase(Weight, InitialDelay, Duration, 1)
		{ }
		inline AnimationMoveSmooth(VEC Weight)
			: AnimationBase(Weight, 0, 500, 1)
		{ }
		
		virtual VEC OnUpdate(VEC Degree);
	};

	class AnimationMoveRude : public AnimationBase
	{
	public:
		inline AnimationMoveRude(VEC Weight, int InitialDelay, int Duration)
			: AnimationBase(Weight, InitialDelay, Duration, 1)
		{ }
		inline AnimationMoveRude(VEC Weight)
			: AnimationBase(Weight, 0, 500, 1)
		{ }
		
		virtual VEC OnUpdate(VEC Degree);
	};

	class AnimationPool : boost::noncopyable
	{
	protected:
		int AnimationPoolIndex;
		boost::ptr_list<AnimationBase> ListAnimation;

	public:
		boost::function<void (VEC)> Set;
		boost::function<VEC ()> Get;
		boost::function<AnimationBase *(VEC Weight)> NewDefault;

		inline AnimationPool()
		{
			AnimationPoolIndex = AnimationManager::ListAnimationPool().Add(this);
			SetDefaultAnimation([](VEC Weight){ return new AnimationMoveSmooth(Weight); });
		}

		inline ~AnimationPool()
		{ AnimationManager::ListAnimationPool().Sub(AnimationPoolIndex); }

		inline void BindMember(VEC & BindTargetMember)
		{
			BindMember(
				[BindTargetMember](VEC AnimValue) mutable -> void
				{ BindTargetMember = AnimValue; },
				[BindTargetMember]() -> VEC
				{ return BindTargetMember; }
			);
		}

		inline void BindMember(
			const boost::function<void (VEC)> & SetProc,
			const boost::function<VEC ()> & GetProc)
		{
			Set = SetProc;
			Get = GetProc;
		}

		inline void SetDefaultAnimation(decltype(NewDefault) NewDefaultFunctor)
		{ this->NewDefault = NewDefaultFunctor; }

		// this will manage AnimationManaged's ownership
		inline void AddAnimation(AnimationBase *AnimationManaged)
		{ ListAnimation.push_back(AnimationManaged); }

		inline void Update(double ElapsedTime)
		{
			ATHROWR(!Set.empty() && !Get.empty(), "Set/Get is not set. Use BindMember() to Bind a variable.");

			VEC cumul = 0;
			for ( auto i = ListAnimation.begin();
				i != ListAnimation.end();
				)
			{
				i->RaiseUpdate(ElapsedTime);

				cumul += i->GetValue() - i->GetLastValue();

				if ( i->GetIsFinished() )
				{
					ListAnimation.erase(i++);
					continue;
				}
				i++;
			}
			
			if ( cumul ) Set(Get()+cumul);
		}

		inline AnimationPool & operator=(VEC Objective)
		{
			ATHROWR(!NewDefault.empty(), "Default animation is not set. Use SetDefaultAnimation().");
			VEC cumul = Get();
			for (	auto i=ListAnimation.begin();
					i != ListAnimation.end();
					i++ )
				cumul += i->GetWeight() - i->GetValue();

			AnimationBase *anim;
			ATHROWR(anim = NewDefault(Objective-cumul), "Default animation creation failed.");
			AddAnimation(anim);
			return *this;
		}
		inline AnimationPool & operator+=(AnimationBase *AnimationManaged)
		{ AddAnimation(AnimationManaged); return *this; }
		inline AnimationPool & operator+=(VEC Weight)
		{
			ATHROWR(!NewDefault.empty(), "Default animation is not set. Use SetDefaultAnimation().");
			AnimationBase *anim;
			ATHROWR(anim = NewDefault(Weight), "Default animation creation failed.");
			return *this += anim;
		}
	};

	// Manages Multiple Animation Pool
	class AnimationPoolHelper
	{
	private:
		std::list<AnimationPool*> ListPool;

	public:
		inline void AddPool(AnimationPool & Pool)
		{
			ListPool.push_back(&Pool);
		}

		inline void Update(double ElapsedTime)
		{
			std::for_each(ListPool.begin(), ListPool.end(),
				[&](AnimationPool *Pool){ Pool->Update(ElapsedTime); } );
		}
	};

}