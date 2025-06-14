#ifndef MBASE_TIMER_LOOP_H
#define MBASE_TIMER_LOOP_H

#include <mbase/common.h>
#include <mbase/list.h>
#include <mbase/framework/timers.h>
#include <mbase/framework/thread_pool.h>

#ifdef MBASE_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifdef MBASE_PLATFORM_UNIX
#include <time.h>
#endif

MBASE_BEGIN

static const U32 gDefaultTimerLimit = 2048;
static SIZE_T gTimerLoopIdCounter = 0;

class timer_loop : public non_copymovable {
public:
	using timer_container = mbase::list<timer_base*>;

	enum class flags : U32 {
		TIMER_SUCCESS = 0,
		TIMER_ERR_LIMIT_REACHED = MBASE_TIMER_LOOP_FLAGS_MIN,
		TIMER_ERR_INVALID_DATA,
		TIMER_WARN_TIMER_WILL_EXECUTE_IMM,
		TIMER_ERR_BELONGS_TO_FOREIGN_LOOP,
		TIMER_ERR_ALREADY_REGISTERED = MBASE_TIMER_LOOP_FLAGS_MAX
	};

	/* ===== BUILDER METHODS BEGIN ===== */
	MBASE_INLINE timer_loop();
	MBASE_INLINE ~timer_loop();
	/* ===== BUILDER METHODS END ===== */

	/* ===== OBSERVATION METHODS BEGIN ===== */
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE SIZE_T get_active_timer_count() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE U32 get_delta_seconds() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::tpool& get_thread_pool() noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::list<timer_base*>& get_timer_list() noexcept;
	/* ===== OBSERVATION METHODS END ===== */

	/* ===== STATE-MODIFIER METHODS BEGIN ===== */
	MBASE_INLINE flags register_timer(timer_base& in_timer) noexcept;
	MBASE_INLINE flags register_timer(timer_base& in_timer, PTRGENERIC in_usr_data) noexcept;
	MBASE_INLINE flags unregister_timer(timer_base& in_timer) noexcept;
	MBASE_INLINE GENERIC run_timers() noexcept;
	MBASE_INLINE GENERIC run_timer_loop() noexcept;
	MBASE_INLINE GENERIC reset_timer() noexcept;
	MBASE_INLINE GENERIC set_timer_limit(U32 in_limit) noexcept;
	MBASE_INLINE GENERIC halt() noexcept;
	MBASE_INLINE GENERIC clear_timers() noexcept;
	/* ===== STATE-MODIFIER METHODS END ===== */

protected:
	F64 mDeltaTime;
	#ifdef MBASE_PLATFORM_WINDOWS
	F64 mFrequency;
	#endif
	U32 mTimerLimit;
	U32 mTimerIdCounter;
	U64 mPrevTime;
	SIZE_T mTimerLoopId;
	bool mIsRunning;
	timer_container mRegisteredTimers;
	mbase::tpool mThreadPool;
};

MBASE_INLINE timer_loop::timer_loop() : mTimerLimit(gDefaultTimerLimit), mTimerIdCounter(0), mPrevTime(0), mTimerLoopId(0), mIsRunning(false)
{
	mTimerLoopId = ++gTimerLoopIdCounter;
	#ifdef MBASE_PLATFORM_WINDOWS
	LARGE_INTEGER performanceFrequency = {};
	QueryPerformanceFrequency(&performanceFrequency);
	mFrequency = 1000 / (F64)performanceFrequency.QuadPart; // MS ACCURACY
	mDeltaTime = mFrequency;
	#endif	

	#ifdef MBASE_PLATFORM_UNIX
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	mPrevTime = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	#endif
}

MBASE_INLINE timer_loop::~timer_loop()
{
	clear_timers();
}

MBASE_INLINE timer_loop::flags timer_loop::register_timer(timer_base& in_timer) noexcept
{
	if (in_timer.is_registered())
	{
		return flags::TIMER_ERR_ALREADY_REGISTERED;
	}

	if (mRegisteredTimers.size() > mTimerLimit)
	{
		return flags::TIMER_ERR_LIMIT_REACHED;
	}

	in_timer.mHandlerId = ++mTimerIdCounter;
	flags terr = flags::TIMER_SUCCESS;

	if (in_timer.get_target_time() <= 0)
	{
		terr = flags::TIMER_WARN_TIMER_WILL_EXECUTE_IMM;
	}

	in_timer.mLoopId = mTimerLoopId;
	in_timer.mStatus = mbase::timer_base::flags::TIMER_STATUS_REGISTERED;
	mRegisteredTimers.push_back(&in_timer);
	in_timer.mSelfIter = mRegisteredTimers.end_node();
	in_timer.on_register();

	return terr;
}

MBASE_INLINE timer_loop::flags timer_loop::register_timer(timer_base& in_timer, PTRGENERIC in_usr_data) noexcept
{
	if (in_timer.is_registered())
	{
		return flags::TIMER_ERR_ALREADY_REGISTERED;
	}

	if (mRegisteredTimers.size() > mTimerLimit)
	{
		return flags::TIMER_ERR_LIMIT_REACHED;
	}

	in_timer.mHandlerId = ++mTimerIdCounter; // Timer ids will start from 1
	flags terr = flags::TIMER_SUCCESS;

	if (in_timer.get_target_time() <= 0)
	{
		// WARN THE USER THAT THE TIMER WILL BE EXECUTED IMMEDIATELY
		terr = flags::TIMER_WARN_TIMER_WILL_EXECUTE_IMM;
	}

	in_timer.mSuppliedData = in_usr_data;
	in_timer.mLoopId = mTimerLoopId;
	in_timer.mStatus = mbase::timer_base::flags::TIMER_STATUS_REGISTERED;
	mRegisteredTimers.push_back(&in_timer);
	in_timer.mSelfIter = mRegisteredTimers.end_node();
	in_timer.on_register();

	return terr;
}

MBASE_INLINE timer_loop::flags timer_loop::unregister_timer(timer_base& in_timer) noexcept
{
	if (!in_timer.is_registered())
	{
		return flags::TIMER_SUCCESS;
	}

	if (in_timer.mLoopId != mTimerLoopId)
	{
		return flags::TIMER_ERR_BELONGS_TO_FOREIGN_LOOP;
	}

	MBASE_NULL_CHECK_RETURN_VAL(in_timer.mSelfIter.get(), flags::TIMER_ERR_INVALID_DATA); // SERIOUS PROBLEM IF THIS OCCURS
	timer_base* tb = *in_timer.mSelfIter;

	tb->mLoopId = 0;
	tb->mSuppliedData = nullptr;
	tb->mStatus = mbase::timer_base::flags::TIMER_STATUS_UNREGISTERED;
	tb->on_unregister();
	mRegisteredTimers.erase(in_timer.mSelfIter);

	return flags::TIMER_SUCCESS;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE SIZE_T timer_loop::get_active_timer_count() const noexcept
{
	return mRegisteredTimers.size();
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE U32 timer_loop::get_delta_seconds() const noexcept
{
	return static_cast<U32>(mDeltaTime);
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::tpool& timer_loop::get_thread_pool() noexcept
{
	return mThreadPool;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::list<timer_base*>& timer_loop::get_timer_list() noexcept
{
	return mRegisteredTimers;
}

MBASE_INLINE GENERIC timer_loop::run_timers() noexcept
{
	#ifdef MBASE_PLATFORM_WINDOWS
	LARGE_INTEGER queryTime;
	QueryPerformanceCounter(&queryTime);
	U64 currentTime = queryTime.QuadPart;
	#endif

	#ifdef MBASE_PLATFORM_UNIX
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	U64 currentTime = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	#endif

	if(!mPrevTime)
	{
		mPrevTime = currentTime; // only the first case
	}

	#ifdef MBASE_PLATFORM_WINDOWS
	mDeltaTime = (currentTime - mPrevTime) * mFrequency;
	#endif

	#ifdef MBASE_PLATFORM_UNIX
	mDeltaTime = (currentTime - mPrevTime);
	#endif


	timer_container::iterator It = mRegisteredTimers.begin();
	while (It != mRegisteredTimers.end())
	{
		timer_base* tmpTimerBase = *It;
		tmpTimerBase->mCurrentTime += mDeltaTime;
		++It;
		if (tmpTimerBase->mCurrentTime >= tmpTimerBase->mTargetTime)
		{
			if (tmpTimerBase->get_execution_policy() == mbase::timer_base::flags::TIMER_POLICY_ASYNC)
			{
				mThreadPool.execute_job(*tmpTimerBase);
			}
			else
			{
				tmpTimerBase->on_call(tmpTimerBase->get_user_data());
			}

			if (!tmpTimerBase->is_registered())
			{
				// which means that the handler in on_call method unregistered itself
				continue;
			}

			if (tmpTimerBase->get_timer_type() == mbase::timer_base::flags::TIMER_TYPE_TIMEOUT)
			{
				tmpTimerBase->mLoopId = 0;
				tmpTimerBase->on_unregister();
				It = mRegisteredTimers.erase(tmpTimerBase->mSelfIter);
			}
			else
			{
				time_interval* ti = static_cast<time_interval*>(tmpTimerBase);
				ti->reset_time();
				ti->mTickCount++;
				if (ti->mTickLimit != 0)
				{
					if (ti->mTickCount >= ti->mTickLimit)
					{
						ti->mLoopId = 0;
						ti->mStatus = mbase::timer_base::flags::TIMER_STATUS_UNREGISTERED;
						ti->on_unregister();
						It = mRegisteredTimers.erase(ti->mSelfIter);
					}
				}
			}
		}
	}
	#ifdef MBASE_PLATFORM_WINDOWS
	QueryPerformanceCounter(&queryTime);
	#endif

	#ifdef MBASE_PLATFORM_UNIX
	clock_gettime(CLOCK_MONOTONIC, &ts);
	#endif
	mPrevTime = currentTime;
}

MBASE_INLINE GENERIC timer_loop::run_timer_loop() noexcept
{
	if (mIsRunning)
	{
		return;
	}

	mIsRunning = true;

	while (mIsRunning)
	{
		run_timers();
	}
}

MBASE_INLINE GENERIC timer_loop::reset_timer() noexcept
{
	mPrevTime = 0;
}

MBASE_INLINE GENERIC timer_loop::set_timer_limit(U32 in_limit) noexcept
{
	mTimerLimit = in_limit;
}

MBASE_INLINE GENERIC timer_loop::halt() noexcept
{
	mIsRunning = false;
}

MBASE_INLINE GENERIC timer_loop::clear_timers() noexcept
{
	for (timer_container::iterator It = mRegisteredTimers.begin(); It != mRegisteredTimers.end();)
	{
		timer_base* ti = *It;
		ti->mLoopId = 0;
		ti->mStatus = mbase::timer_base::flags::TIMER_STATUS_ABANDONED;
		ti->on_unregister();
		It = mRegisteredTimers.erase(ti->mSelfIter);
	}
}

MBASE_END

#endif // !MBASE_TIMER_LOOP_H
