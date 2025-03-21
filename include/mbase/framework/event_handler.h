#ifndef MBASE_EVENT_HANDLER_H
#define MBASE_EVENT_HANDLER_H

#include <mbase/common.h>
#include <mbase/list.h>
#include <mbase/string.h>
#include <mbase/framework/handler_base.h>

MBASE_BEGIN

class event_handler : public handler_base {
public:
	enum class flags : U32 {
		EVENT_HANDLER_SUCCESS = 0,
		EVENT_ONCE = MBASE_EVENT_HANDLER_FLAGS_CONTROL_START,
		EVENT_ON,
		EVENT_REGISTERED,
		EVENT_UNREGISTERED,
		EVENT_ERR_UNKNOWN = MBASE_EVENT_HANDLER_FLAGS_MAX
	};

	/* ===== BUILDER METHODS BEGIN ===== */
	MBASE_INLINE event_handler();
	/* ===== BUILDER METHODS END ===== */

	/* ===== OBSERVATION METHODS BEGIN ===== */
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::string get_event_name() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE flags get_event_type() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE flags get_event_status() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE SIZE_T get_manager_id() const noexcept;
	MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE bool is_registered() const noexcept;
	/* ===== OBSERVATION METHODS END ===== */

	virtual GENERIC on_register() { /* DEFAULT IMPL */ }
	virtual GENERIC on_unregister() { /* DEFAULT IMPL */ }

	friend class event_manager;
private:
	using event_element = mbase::list<event_handler*>::iterator;
	
	event_element mSelfIter;
	mbase::string mEventName;
	flags mEventType;
	flags mStatus;
	SIZE_T mManagerId;
};

MBASE_INLINE event_handler::event_handler() : handler_base(), mSelfIter(nullptr), mEventName(""), mEventType(flags::EVENT_ON), mStatus(flags::EVENT_UNREGISTERED), mManagerId(0)
{
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE mbase::string event_handler::get_event_name() const noexcept
{
	return mEventName;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE event_handler::flags event_handler::get_event_type() const noexcept
{
	return mEventType;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE event_handler::flags event_handler::get_event_status() const noexcept
{
	return mStatus;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE SIZE_T event_handler::get_manager_id() const noexcept
{
	return mManagerId;
}

MBASE_ND(MBASE_OBS_IGNORE) MBASE_INLINE bool event_handler::is_registered() const noexcept
{
	return mStatus == flags::EVENT_REGISTERED;
}


MBASE_END

#endif // !MBASE_EVENT_HANDLER_H
