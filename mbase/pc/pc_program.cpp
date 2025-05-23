#include <mbase/pc/pc_program.h>
#include <mbase/pc/pc_config.h>
#include <mbase/pc/pc_diagnostics.h>
#include <mbase/pc/pc_io_manager.h>
#include <mbase/pc/pc_net_manager.h>
#include <mbase/pc/pc_state.h>

MBASE_BEGIN

PcProgramBase::PcProgramBase() :
	mConfig(NULL),
	mDiagnostics(NULL),
	mIoManager(NULL),
	mNetManager(NULL),
	mProgramState(NULL),
	mIsRunning(false),
	mIsAuthorized(false),
	mIsInitialized(false)
{

}

PcProgramBase::~PcProgramBase()
{

}

PcConfig* PcProgramBase::get_config() noexcept
{
	return mConfig;
}

PcDiagnostics* PcProgramBase::get_diagnostics_manager() noexcept
{
	return mDiagnostics;
}

PcIoManager* PcProgramBase::get_io_manager() noexcept
{
	return mIoManager;
}

PcNetManager* PcProgramBase::get_net_manager() noexcept
{
	return mNetManager;
}

PcProgramInformation PcProgramBase::get_program_info() const noexcept
{
	return mProgramInfo;
}

timer_loop* PcProgramBase::get_timer_loop() noexcept
{
	return &mTimerLoop;
}

bool PcProgramBase::is_running() const noexcept
{
	return mIsRunning;
}

bool PcProgramBase::is_initialized() const noexcept
{
	return mIsInitialized;
}

GENERIC PcProgramBase::initialize_system(
	PcProgramInformation in_program_info,
	PcConfig* in_configurator,
	PcDiagnostics* in_diagnostics,
	PcIoManager* in_io_manager,
	PcNetManager* in_net_manager,
	PcState* in_program_state
)
{
	mProgramInfo = in_program_info;
	mConfig = in_configurator;
	mDiagnostics = in_diagnostics;
	mIoManager = in_io_manager;
	mNetManager = in_net_manager;
	mProgramState = in_program_state;
}

GENERIC PcProgramBase::update_defaults()
{
	mConfig->update();
	mProgramState->update();
	mNetManager->update();
	//mIoManager->update();
	mTimerLoop.run_timers();
}

bool PcProgramBase::halt()
{
	return false;
}

bool PcProgramBase::exit(I32 in_code, mbase::string in_message)
{
	printf("%s\n", in_message.c_str());
	::exit(in_code);
	return false;
}

MBASE_END