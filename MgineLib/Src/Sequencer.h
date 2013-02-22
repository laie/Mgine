#pragma once

namespace Mgine
{
	class Sequencer
	{
	public:
		enum PROC_COMMAND : BYTE
		{
			Stay,
			Finish
		};

		struct PLAN;
		struct PLANARG;
		struct PLAN
		{
			boost::function<void (PLANARG & Arg)> Proc;
			double MaxDuration;

			inline PLAN(
				const boost::function<void (PLANARG & Arg)> & Proc,
				const double & MaxDuration
				)
			{
				ATHROW(!Proc.empty());
				this->Proc = Proc;
				this->MaxDuration = MaxDuration;
			}
		};

		struct PLANARG
		{
			const bool & IsStarting;	// tells whether it is first call or not
			const bool & IsFinishing;	// tells whether it is last call or not

			const double & Progress;	// progress in ms
			const double & MaxDuration;	// max progress; duration

			const double & ElapsedTime;	// elapsed time by before frame

			PROC_COMMAND & Command; // command to caller

			inline PLANARG(
				const bool & IsStarting,
				const bool & IsFinishing,
				const double & Progress,
				const double & MaxDuration,
				const double & ElapsedTime,
				PROC_COMMAND & Command
				) :
				IsStarting(IsStarting),
				IsFinishing(IsFinishing),
				Progress(Progress),
				MaxDuration(MaxDuration),
				ElapsedTime(ElapsedTime),
				Command(Command)
			{ }

			inline double Degree() const { return Progress/MaxDuration; }; // Progress 0 to 1
			
			inline void Next()
			{ Command = Finish; }
		};

	private:
		boost::ptr_list<PLAN> ListProc;
		double CurrentTick;

	public:

		inline Sequencer()
		{
		}

		inline ~Sequencer()
		{
		}

		inline void Update(double ElapsedTime)
		{
			if ( ListProc.empty() ) return;
			bool isstarting = false;
			bool isfinishing = false;
			if ( !CurrentTick ) isstarting = true;

			CurrentTick += ElapsedTime;

			for ( ; !ListProc.empty(); )
			{
				isfinishing = false;

				PROC_COMMAND cmd = Stay;
				PLAN & plan = ListProc.front();

				if ( plan.MaxDuration != -1 && CurrentTick >= plan.MaxDuration )
				{
					isfinishing = true;

					PLANARG arg(isstarting, isfinishing,
						plan.MaxDuration, plan.MaxDuration,
						ElapsedTime, cmd);
					plan.Proc(arg);

					CurrentTick -= plan.MaxDuration;
					ListProc.erase(ListProc.begin());
					isstarting = true;
					continue;
				} else 
				{
					PLANARG arg(isstarting, isfinishing,
						CurrentTick, plan.MaxDuration,
						ElapsedTime, cmd);
					plan.Proc(arg);
					
					switch ( cmd )
					{
					case Finish:
						Next();
						break;
					}
					isstarting = false;
				}
				break;
			}
		}

		inline void Clear()
		{
			CurrentTick = 0;
			ListProc.clear();
		}

		inline void Next()
		{
			CurrentTick = 0;
			if ( !ListProc.empty() ) ListProc.erase(ListProc.begin());
		}

		inline void AddPlan(const boost::function<void (PLANARG & Arg)> & Proc)
		{ ListProc.push_back(new PLAN(Proc, -1)); }
		inline void AddPlan(const boost::function<void (PLANARG & Arg)> & Proc, const double & MaxDuration)
		{ ListProc.push_back(new PLAN(Proc, MaxDuration)); }
	};

	class SequencerHelper
	{
	private:
		std::list<Sequencer*> ListSequencer;

	public:
		inline void AddSequencer(Sequencer & SequencerMember)
		{ ListSequencer.push_back(&SequencerMember); }

		inline void Update(double ElapsedTime)
		{
			for ( auto i=ListSequencer.begin();
				i != ListSequencer.end();
				i++ )
			{
				(*i)->Update(ElapsedTime);
			}
		}
	};
}