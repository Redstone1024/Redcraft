#include "Miscellaneous/Console.h"

#include <cstdio>

#if PLATFORM_WINDOWS
#	undef TEXT
#	include <windows.h>
#elif PLATFORM_LINUX
#	include <sys/ioctl.h>
#	include <unistd.h>
#endif

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

#if PLATFORM_WINDOWS

NODISCARD bool InitANSIConsole()
{
	static bool bResult = []
	{
		HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return false;

		DWORD ConsoleMode = 0;

		if (!GetConsoleMode(Console, &ConsoleMode)) return false;

		ConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(Console, ConsoleMode)) return false;

		return true;
	}
	();

	return bResult;
}

#endif

EColor GForegroundColor = EColor::Default;
EColor GBackgroundColor = EColor::Default;

EColor GetForegroundColor()
{
#	if PLATFORM_WINDOWS
	{
		if (InitANSIConsole()) return GForegroundColor;

		const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return EColor::Default;

		CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

		if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return EColor::Default;

		const WORD Color = ConsoleInfo.wAttributes;

		EColor Result = EColor::Black;

		if (Color & FOREGROUND_RED)       Result |= EColor::Red;
		if (Color & FOREGROUND_GREEN)     Result |= EColor::Green;
		if (Color & FOREGROUND_BLUE)      Result |= EColor::Blue;
		if (Color & FOREGROUND_INTENSITY) Result |= EColor::Intensity;

		return Result;
	}
#	endif

	return GForegroundColor;
}

EColor GetBackgroundColor()
{
#	if PLATFORM_WINDOWS
	{
		if (InitANSIConsole()) return GBackgroundColor;

		const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return EColor::Default;

		CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

		if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return EColor::Default;

		const WORD Color = ConsoleInfo.wAttributes;

		EColor Result = EColor::Black;

		if (Color & BACKGROUND_RED)       Result |= EColor::Red;
		if (Color & BACKGROUND_GREEN)     Result |= EColor::Green;
		if (Color & BACKGROUND_BLUE)      Result |= EColor::Blue;
		if (Color & BACKGROUND_INTENSITY) Result |= EColor::Intensity;

		return Result;
	}
#	endif

	return GBackgroundColor;
}

EColor SetForegroundColor(EColor InColor)
{
	if (IsOutputRedirected()) return GetForegroundColor();

#	if PLATFORM_WINDOWS
	{
		if (!InitANSIConsole())
		{
			const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

			if (Console == INVALID_HANDLE_VALUE) return GetForegroundColor();

			CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

			if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return GetForegroundColor();

			WORD Color = ConsoleInfo.wAttributes & ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

			if (InColor == EColor::Default) InColor = EColor::White;

			if (!!(InColor & EColor::Red))       Color |= FOREGROUND_RED;
			if (!!(InColor & EColor::Green))     Color |= FOREGROUND_GREEN;
			if (!!(InColor & EColor::Blue))      Color |= FOREGROUND_BLUE;
			if (!!(InColor & EColor::Intensity)) Color |= FOREGROUND_INTENSITY;

			if (!SetConsoleTextAttribute(Console, Color)) return GetForegroundColor();

			return InColor;
		}
	}
#	endif

#	if PLATFORM_WINDOWS || PLATFORM_LINUX
	{
		int Result;

		switch (InColor)
		{
		case EColor::Black:         Result = std::fputs("\033[30m", stdout); break;
		case EColor::Red:           Result = std::fputs("\033[31m", stdout); break;
		case EColor::Green:         Result = std::fputs("\033[32m", stdout); break;
		case EColor::Yellow:        Result = std::fputs("\033[33m", stdout); break;
		case EColor::Blue:          Result = std::fputs("\033[34m", stdout); break;
		case EColor::Magenta:       Result = std::fputs("\033[35m", stdout); break;
		case EColor::Cyan:          Result = std::fputs("\033[36m", stdout); break;
		case EColor::White:         Result = std::fputs("\033[37m", stdout); break;
		case EColor::BrightBlack:   Result = std::fputs("\033[90m", stdout); break;
		case EColor::BrightRed:     Result = std::fputs("\033[91m", stdout); break;
		case EColor::BrightGreen:   Result = std::fputs("\033[92m", stdout); break;
		case EColor::BrightYellow:  Result = std::fputs("\033[93m", stdout); break;
		case EColor::BrightBlue:    Result = std::fputs("\033[94m", stdout); break;
		case EColor::BrightMagenta: Result = std::fputs("\033[95m", stdout); break;
		case EColor::BrightCyan:    Result = std::fputs("\033[96m", stdout); break;
		case EColor::BrightWhite:   Result = std::fputs("\033[97m", stdout); break;
		default:                    Result = std::fputs("\033[39m", stdout); break;
		}

		if (Result == EOF) return GetForegroundColor();

		return GForegroundColor = InColor;
	}
#	endif

	return GetForegroundColor();
}

EColor SetBackgroundColor(EColor InColor)
{
	if (IsOutputRedirected()) return GetBackgroundColor();

#	if PLATFORM_WINDOWS
	{
		if (!InitANSIConsole())
		{
			const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

			if (Console == INVALID_HANDLE_VALUE) return GetBackgroundColor();

			CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

			if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return GetBackgroundColor();

			WORD Color = ConsoleInfo.wAttributes & ~(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);

			if (InColor == EColor::Default) InColor = EColor::Black;

			if (!!(InColor & EColor::Red))       Color |= BACKGROUND_RED;
			if (!!(InColor & EColor::Green))     Color |= BACKGROUND_GREEN;
			if (!!(InColor & EColor::Blue))      Color |= BACKGROUND_BLUE;
			if (!!(InColor & EColor::Intensity)) Color |= BACKGROUND_INTENSITY;

			if (!SetConsoleTextAttribute(Console, Color)) return GetBackgroundColor();

			return InColor;
		}
	}
#	endif

#	if PLATFORM_WINDOWS || PLATFORM_LINUX
	{
		int Result;

		switch (InColor)
		{
		case EColor::Black:         Result = std::fputs("\033[40m",  stdout); break;
		case EColor::Red:           Result = std::fputs("\033[41m",  stdout); break;
		case EColor::Green:         Result = std::fputs("\033[42m",  stdout); break;
		case EColor::Yellow:        Result = std::fputs("\033[43m",  stdout); break;
		case EColor::Blue:          Result = std::fputs("\033[44m",  stdout); break;
		case EColor::Magenta:       Result = std::fputs("\033[45m",  stdout); break;
		case EColor::Cyan:          Result = std::fputs("\033[46m",  stdout); break;
		case EColor::White:         Result = std::fputs("\033[47m",  stdout); break;
		case EColor::BrightBlack:   Result = std::fputs("\033[100m", stdout); break;
		case EColor::BrightRed:     Result = std::fputs("\033[101m", stdout); break;
		case EColor::BrightGreen:   Result = std::fputs("\033[102m", stdout); break;
		case EColor::BrightYellow:  Result = std::fputs("\033[103m", stdout); break;
		case EColor::BrightBlue:    Result = std::fputs("\033[104m", stdout); break;
		case EColor::BrightMagenta: Result = std::fputs("\033[105m", stdout); break;
		case EColor::BrightCyan:    Result = std::fputs("\033[106m", stdout); break;
		case EColor::BrightWhite:   Result = std::fputs("\033[107m", stdout); break;
		default:                    Result = std::fputs("\033[49m",  stdout); break;
		}

		if (Result == EOF) return GetBackgroundColor();

		return GBackgroundColor = InColor;
	}
#	endif

	return GetBackgroundColor();
}

uint GetWindowWidth()
{
#	if PLATFORM_WINDOWS
	{
		const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return static_cast<uint>(-1);

		CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

		if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return static_cast<uint>(-1);

		return static_cast<uint>(ConsoleInfo.srWindow.Right - ConsoleInfo.srWindow.Left + 1);
	}
#	elif PLATFORM_LINUX
	{
		winsize Size;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &Size) == -1) return static_cast<uint>(-1);

		return static_cast<uint>(Size.ws_col);
	}
#	endif

	return static_cast<uint>(-1);
}

uint GetWindowHeight()
{
#	if PLATFORM_WINDOWS
	{
		const HANDLE Console = GetStdHandle(STD_OUTPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return static_cast<uint>(-1);

		CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;

		if (!GetConsoleScreenBufferInfo(Console, &ConsoleInfo)) return static_cast<uint>(-1);

		return static_cast<uint>(ConsoleInfo.srWindow.Bottom - ConsoleInfo.srWindow.Top + 1);
	}
#	elif PLATFORM_LINUX
	{
		winsize Size;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &Size) == -1) return static_cast<uint>(-1);

		return static_cast<uint>(Size.ws_row);
	}
#	endif

	return static_cast<uint>(-1);
}

bool IsInputRedirected()
{
#	if PLATFORM_WINDOWS
	{
		const HANDLE StandardInput = GetStdHandle(STD_INPUT_HANDLE);

		if (StandardInput == INVALID_HANDLE_VALUE) return false;

		DWORD FileType = GetFileType(StandardInput);

		return FileType != FILE_TYPE_CHAR;
	}
#	elif PLATFORM_LINUX
	{
		return isatty(fileno(stdin)) == 0;
	}
#	endif

	return false;
}

bool IsOutputRedirected()
{
#	if PLATFORM_WINDOWS
	{
		const HANDLE StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		if (StandardOutput == INVALID_HANDLE_VALUE) return false;

		DWORD FileType = GetFileType(StandardOutput);

		return FileType != FILE_TYPE_CHAR;
	}
#	elif PLATFORM_LINUX
	{
		return isatty(fileno(stdout)) == 0;
	}
#	endif

	return false;
}

bool IsErrorRedirected()
{
#	if PLATFORM_WINDOWS
	{
		const HANDLE StandardError = GetStdHandle(STD_ERROR_HANDLE);

		if (StandardError == INVALID_HANDLE_VALUE) return false;

		DWORD FileType = GetFileType(StandardError);

		return FileType != FILE_TYPE_CHAR;
	}
#	elif PLATFORM_LINUX
	{
		return isatty(fileno(stderr)) == 0;
	}
#	endif

	return false;
}

void Clear()
{
	if (IsOutputRedirected()) return;

#	if PLATFORM_WINDOWS
	{
		std::system("cls");
	}
#	elif PLATFORM_LINUX
	{
		Ignore = std::fputs("\033[2J\033[1;1H", stdout);
	}
#	endif
}

char Input(bool bEcho)
{
	if (bEcho || IsOutputRedirected())
	{
		const int Result = std::getchar();

		if (Result == EOF) return static_cast<char>(-1);

		return static_cast<char>(Result);
	}

#	if PLATFORM_WINDOWS
	{
		const HANDLE Console = GetStdHandle(STD_INPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return static_cast<char>(-1);

		DWORD ConsoleMode = 0;

		GetConsoleMode(Console, &ConsoleMode);

		SetConsoleMode(Console, ConsoleMode & ~ENABLE_ECHO_INPUT);

		const char Result = Input();

		SetConsoleMode(Console, ConsoleMode);

		return Result;
	}
#	elif PLATFORM_LINUX
	{ }
#	endif

	return static_cast<char>(-1);
}

FString InputLn(bool bEcho)
{
	if (bEcho || IsOutputRedirected())
	{
		FString Result;

		while (true)
		{
			const int Char = std::getchar();

			if (Char == EOF || Char == '\n') break;

			Result.PushBack(static_cast<char>(Char));
		}

		return Result;
	}

#	if PLATFORM_WINDOWS
	{
		const HANDLE Console = GetStdHandle(STD_INPUT_HANDLE);

		if (Console == INVALID_HANDLE_VALUE) return "";

		DWORD ConsoleMode = 0;

		GetConsoleMode(Console, &ConsoleMode);

		SetConsoleMode(Console, ConsoleMode & ~ENABLE_ECHO_INPUT);

		const FString Result = InputLn();

		SetConsoleMode(Console, ConsoleMode);

		return Result;
	}
#	elif PLATFORM_LINUX
	{ }
#	endif

	return "";
}

bool Print(char Char)
{
	return std::putchar(Char) != EOF;
}

bool Error(char Char)
{
	return std::fputc(Char, stderr) != EOF;
}

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
