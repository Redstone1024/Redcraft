#pragma once

#include "CoreTypes.h"

#include <cstdlib>
#include <utility>
#include <csignal>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#pragma warning(push)
#pragma warning(disable : 4996)

using FAtexitHandler = void();
using FSignalHandler = void(int);;

/** Indicates program execution status. */
enum class EExitCode : int
{
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE,
};

/** Defines signal types. */
enum class ESignalType : int
{
	SoftwareTermination = SIGTERM, // Termination request, sent to the program.
	SegmentationFault   = SIGSEGV, // Invalid memory access as segmentation fault.
	ExternalInterrupt   = SIGINT,  // External interrupt, usually initiated by the user.
	IllegalInstruction  = SIGILL,  // Invalid program image, such as invalid instruction.
	AbnormalTermination = SIGABRT, // Abnormal termination condition, as is e.g. initiated by Abort().
	ArithmeticException = SIGFPE,  // Erroneous arithmetic operation such as divide by zero.
};

inline static FSignalHandler* GSignalDefault = SIG_DFL; // Defines default signal handling strategies.
inline static FSignalHandler* GSignalIgnored = SIG_IGN; // Defines Signal is ignored strategies.
inline static FSignalHandler* GSignalError   = SIG_ERR; // Return value of signal specifying that an error was encountered.

/** The integer type that can be accessed as an atomic entity from an asynchronous signal handler. */
using FSignalAtomic = NAMESPACE_STD::sig_atomic_t;

/** Causes abnormal program termination without cleaning up. */
NORETURN FORCEINLINE void Abort()
{
	NAMESPACE_STD::abort();
}

/** Causes normal program termination with cleaning up. */
NORETURN FORCEINLINE void Exit(EExitCode ExitCode)
{
	NAMESPACE_STD::exit(static_cast<int>(ExitCode));
};

/** Causes quick program termination without completely cleaning up. */
NORETURN FORCEINLINE void QuickExit(EExitCode ExitCode)
{
	NAMESPACE_STD::quick_exit(static_cast<int>(ExitCode));
};

/** Causes normal program termination without cleaning up. */
NORETURN FORCEINLINE void QuickExitWithoutCleaning(EExitCode ExitCode)
{
	NAMESPACE_STD::_Exit(static_cast<int>(ExitCode));
};

/** Registers a function to be called on Exit() invocation. */
FORCEINLINE bool AtExit(FAtexitHandler* InFunc)
{
	return NAMESPACE_STD::atexit(InFunc) == 0;
}

/** Registers a function to be called on QuickExit invocation. */
FORCEINLINE bool AtQuickExit(FAtexitHandler* InFunc)
{
	return NAMESPACE_STD::at_quick_exit(InFunc) == 0;
}

/** Marks unreachable point of execution. */
NORETURN FORCEINLINE void Unreachable()
{
#	ifdef __cpp_lib_unreachable
	{
		NAMESPACE_STD::unreachable();
	}
#	endif
}

/** Calls the host environment's command processor. */
FORCEINLINE EExitCode System(const char* InCommand)
{
	return static_cast<EExitCode>(NAMESPACE_STD::system(InCommand));
}

/** Access to the list of environment variables. */
NODISCARD FORCEINLINE const char* GetEnv(const char* InEnv)
{
	return NAMESPACE_STD::getenv(InEnv);
}

/** Sets a signal handler for particular signal. */
FORCEINLINE FSignalHandler* Signal(ESignalType InType, FSignalHandler* InFunc)
{
	return NAMESPACE_STD::signal(static_cast<int>(InType), InFunc);
}

/** Runs the signal handler for particular signal. */
FORCEINLINE bool Raise(ESignalType InType)
{
	return NAMESPACE_STD::raise(static_cast<int>(InType)) == 0;
}

#pragma warning(pop)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
