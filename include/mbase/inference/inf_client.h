#ifndef MBASE_INF_CLIENT_H
#define MBASE_INF_CLIENT_H

#include <mbase/common.h>
#include <mbase/behaviors.h>
#include <mbase/string.h>
#include <mbase/unordered_map.h>
#include <mbase/framework/handler_base.h>

MBASE_BEGIN

// inheriting from InfClientBase seemed unneccesary, so I removed it.

class InfProcessorBase;

class MBASE_API InfClientBase {
public:
	using size_type = U64;

	/* ===== BUILDER METHODS BEGIN ===== */
	virtual ~InfClientBase();
	/* ===== BUILDER METHODS END ===== */
	
	/* ===== INTERFACE METHODS BEGIN ===== */
	virtual GENERIC on_register(InfProcessorBase* out_processor) = 0;
	virtual GENERIC on_unregister(InfProcessorBase* out_processor) = 0;
	/* ===== INTERFACE METHODS END ===== */
};

MBASE_END

#endif // !MBASE_INF_CLIENT_H
