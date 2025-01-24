#include <Miscellaneous/FileSystem.h>

#include "Numerics/Bit.h"
#include "Numerics/Math.h"
#include "Templates/ScopeHelper.h"
#include "Containers/StaticArray.h"

#include <cstdio>

#if PLATFORM_WINDOWS
#	undef TEXT
#	include <windows.h>
#	undef CreateDirectory
#elif PLATFORM_LINUX
#	include <unistd.h>
#	include <dirent.h>
#	include <sys/stat.h>
#endif

#pragma warning(push)
#pragma warning(disable: 4996)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(FileSystem)

bool LoadFileToArray(TArray<uint8>& Result, FStringView Path)
{
	if (!FileSystem::Exists(Path)) return false;

	FILE* File = std::fopen(*Path, "rb");

	if (File == nullptr) return false;

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fseek(File, 0, SEEK_END) != 0) return false;

	const long Length = std::ftell(File);

	if (!Math::IsWithin(Length, 0, TNumericLimits<long>::Max())) return false;

	if (std::fseek(File, 0, SEEK_SET) != 0) return false;

	Result.SetNum(Length);

	if (std::fread(Result.GetData(), sizeof(uint8), Length, File) != static_cast<size_t>(Length)) return false;

	FileGuard.Release();

	if (std::fclose(File) != 0) return false;

	return true;
}

bool SaveArrayToFile(TArrayView<const uint8> Data, FStringView Path)
{
	FILE* File = std::fopen(*Path, "wb");

	if (File == nullptr) return false;

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fwrite(Data.GetData(), sizeof(uint8), Data.Num(), File) != Data.Num()) return false;

	FileGuard.Release();

	if (std::fclose(File) != 0) return false;

	return true;
}

template <CCharType T>
bool LoadFileToString(TString<T>& Result, FStringView Path, FileSystem::EEncoding Encoding /* = FileSystem::EEncoding::Default */, bool bVerify /* = false */)
{
	if (!FileSystem::Exists(Path)) return false;

	FILE* File = std::fopen(*Path, "rb");

	if (File == nullptr) return false;

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fseek(File, 0, SEEK_END) != 0) return false;

	long Length = std::ftell(File);

	if (!Math::IsWithin(Length, 0, TNumericLimits<long>::Max())) return false;

	if (std::fseek(File, 0, SEEK_SET) != 0) return false;

	TStaticArray<uint8, 4> Buffer = { 0xAA, 0xAA, 0xAA, 0xAA };

	Ignore = std::fread(Buffer.GetData(), sizeof(uint8), Buffer.Num(), File);

	// Auto-detect the encoding if it is not specified.
	if (Encoding == FileSystem::EEncoding::Default)
	{
		// Check if the file is a UTF-32 encoded file.
		if      (Buffer[0] == 0x00 && Buffer[1] == 0x00 && Buffer[2] == 0xFE && Buffer[3] == 0xFF) Encoding = FileSystem::EEncoding::UTF32BE;
		else if (Buffer[0] == 0xFF && Buffer[1] == 0xFE && Buffer[2] == 0x00 && Buffer[3] == 0x00) Encoding = FileSystem::EEncoding::UTF32LE;

		// Check if the file is a UTF-16 encoded file.
		else if (Buffer[0] == 0xFF && Buffer[1] == 0xFE) Encoding = FileSystem::EEncoding::UTF16LE;
		else if (Buffer[0] == 0xFE && Buffer[1] == 0xFF) Encoding = FileSystem::EEncoding::UTF16BE;

		// Check if the file is a UTF-8 encoded file.
		else if (Buffer[0] == 0xEF && Buffer[1] == 0xBB && Buffer[2] == 0xBF) Encoding = FileSystem::EEncoding::UTF8;

		// Check if the file is a wide character encoded file.
		else if (Buffer[0] == 0x00 || Buffer[1] == 0x00 || Buffer[2] == 0x00 || Buffer[3] == 0x00) Encoding = FileSystem::EEncoding::Wide;

		// Check if the file is a narrow character encoded file.
		else Encoding = FileSystem::EEncoding::Narrow;
	}

	// Jump to the BOM character if the file is a UTF-8, UTF-16 or UTF-32 encoded file.
	switch (Encoding)
	{
	case FileSystem::EEncoding::Narrow:
	case FileSystem::EEncoding::Wide:                                                                                          { Length -= 0; if (std::fseek(File, 0, SEEK_SET) != 0) return false; } break;
	case FileSystem::EEncoding::UTF8:    if (Buffer[0] == 0xEF && Buffer[1] == 0xBB && Buffer[2] == 0xBF)                      { Length -= 3; if (std::fseek(File, 3, SEEK_SET) != 0) return false; } break;
	case FileSystem::EEncoding::UTF16BE: if (Buffer[0] == 0xFE && Buffer[1] == 0xFF)                                           { Length -= 2; if (std::fseek(File, 2, SEEK_SET) != 0) return false; } break;
	case FileSystem::EEncoding::UTF16LE: if (Buffer[0] == 0xFF && Buffer[1] == 0xFE)                                           { Length -= 2; if (std::fseek(File, 2, SEEK_SET) != 0) return false; } break;
	case FileSystem::EEncoding::UTF32BE: if (Buffer[0] == 0x00 && Buffer[1] == 0x00 && Buffer[2] == 0xFE && Buffer[3] == 0xFF) { Length -= 4; if (std::fseek(File, 4, SEEK_SET) != 0) return false; } break;
	case FileSystem::EEncoding::UTF32LE: if (Buffer[0] == 0xFF && Buffer[1] == 0xFE && Buffer[2] == 0x00 && Buffer[3] == 0x00) { Length -= 4; if (std::fseek(File, 4, SEEK_SET) != 0) return false; } break;
	default: check_no_entry();
	}

	check(Math::EEndian::Native == Math::EEndian::Big || Math::EEndian::Native == Math::EEndian::Little);

	const bool bByteSwap =
		Math::EEndian::Native == Math::EEndian::Big    ? Encoding == FileSystem::EEncoding::UTF16LE || Encoding == FileSystem::EEncoding::UTF32LE :
		Math::EEndian::Native == Math::EEndian::Little ? Encoding == FileSystem::EEncoding::UTF16BE || Encoding == FileSystem::EEncoding::UTF32BE : false;

	const auto LoadImpl = [File, Length, bByteSwap]<typename U>(TString<U>& String) -> bool
	{
		if (Length % sizeof(U) != 0) return false;

		String.Reserve(Length / sizeof(U));

		while (true)
		{
			U Char;

			const size_t ReadNum = std::fread(&Char, 1, sizeof(U), File);

			if (ReadNum == 0) break;

			if (ReadNum != sizeof(U)) return false;

			if (bByteSwap) Char = Math::ByteSwap(static_cast<TMakeUnsigned<U>>(Char));

#			if PLATFORM_WINDOWS
			{
				if (!String.IsEmpty() && String.Back() == LITERAL(U, '\r') && Char == LITERAL(U, '\n'))
				{
					String.PopBack();
				}
			}
#			endif

			String.PushBack(Char);
		}

		return true;
	};

	bool bCompatible = false;

	if      constexpr (CSameAs<T, char>)    bCompatible |= Encoding == FileSystem::EEncoding::Narrow;
	else if constexpr (CSameAs<T, wchar>)   bCompatible |= Encoding == FileSystem::EEncoding::Wide;
	else if constexpr (CSameAs<T, u8char>)  bCompatible |= Encoding == FileSystem::EEncoding::UTF8;
	else if constexpr (CSameAs<T, u16char>) bCompatible |= Encoding == FileSystem::EEncoding::UTF16BE || Encoding == FileSystem::EEncoding::UTF16LE;
	else if constexpr (CSameAs<T, u32char>) bCompatible |= Encoding == FileSystem::EEncoding::UTF32BE || Encoding == FileSystem::EEncoding::UTF32LE;

	else static_assert(sizeof(T) == -1, "Unsupported character type");

	if (!bCompatible || bVerify)
	{
		switch (Encoding)
		{
		case FileSystem::EEncoding::Narrow:  { FString    Temp; if (!LoadImpl(Temp)) return false; if (!Result.DecodeFrom(Temp)) return false; break; }
		case FileSystem::EEncoding::Wide:    { FWString   Temp; if (!LoadImpl(Temp)) return false; if (!Result.DecodeFrom(Temp)) return false; break; }
		case FileSystem::EEncoding::UTF8:    { FU8String  Temp; if (!LoadImpl(Temp)) return false; if (!Result.DecodeFrom(Temp)) return false; break; }
		case FileSystem::EEncoding::UTF16BE:
		case FileSystem::EEncoding::UTF16LE: { FU16String Temp; if (!LoadImpl(Temp)) return false; if (!Result.DecodeFrom(Temp)) return false; break; }
		case FileSystem::EEncoding::UTF32BE:
		case FileSystem::EEncoding::UTF32LE: { FU32String Temp; if (!LoadImpl(Temp)) return false; if (!Result.DecodeFrom(Temp)) return false; break; }
		default: check_no_entry();
		}
	}

	else if (!LoadImpl(Result)) return false;

	FileGuard.Release();

	if (std::fclose(File) != 0) return false;

	return true;
}

template REDCRAFTUTILITY_API bool LoadFileToString<char>   (FString&,    FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool LoadFileToString<wchar>  (FWString&,   FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool LoadFileToString<u8char> (FU8String&,  FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool LoadFileToString<u16char>(FU16String&, FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool LoadFileToString<u32char>(FU32String&, FStringView, FileSystem::EEncoding, bool);

template <CCharType T>
bool SaveStringToFile(TStringView<T> String, FStringView Path, FileSystem::EEncoding Encoding /* = FileSystem::EEncoding::Default */, bool bWithBOM /* = true */)
{
	bool bCompatible = Encoding == FileSystem::EEncoding::Default;

	if      constexpr (CSameAs<T, char>)    bCompatible |= Encoding == FileSystem::EEncoding::Narrow;
	else if constexpr (CSameAs<T, wchar>)   bCompatible |= Encoding == FileSystem::EEncoding::Wide;
	else if constexpr (CSameAs<T, u8char>)  bCompatible |= Encoding == FileSystem::EEncoding::UTF8;
	else if constexpr (CSameAs<T, u16char>) bCompatible |= Encoding == FileSystem::EEncoding::UTF16BE || Encoding == FileSystem::EEncoding::UTF16LE;
	else if constexpr (CSameAs<T, u32char>) bCompatible |= Encoding == FileSystem::EEncoding::UTF32BE || Encoding == FileSystem::EEncoding::UTF32LE;

	else static_assert(sizeof(T) == -1, "Unsupported character type");

	if (bCompatible)
	{
		FILE* File = std::fopen(*Path, "wb");

		if (File == nullptr) return false;

		auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

		if (bWithBOM)
		{
			if constexpr (CSameAs<T, u8char>)
			{
				if (std::fwrite(U8TEXT("\uFEFF"), 1, 3, File) != 3) return false;
			}

			else if constexpr (CSameAs<T, u16char>)
			{
				constexpr TStaticArray<uint8, 2> BufferBE = { 0xFE, 0xFF };
				constexpr TStaticArray<uint8, 2> BufferLE = { 0xFF, 0xFE };

				if      (Encoding == FileSystem::EEncoding::UTF16BE) { if (std::fwrite(BufferBE.GetData(), 1, BufferBE.Num(), File) != BufferBE.Num()) return false; }
				else if (Encoding == FileSystem::EEncoding::UTF16LE) { if (std::fwrite(BufferLE.GetData(), 1, BufferLE.Num(), File) != BufferLE.Num()) return false; }

				else if (std::fwrite(U16TEXT("\uFEFF"), 1, sizeof(T), File) != sizeof(T)) return false;
			}

			else if constexpr (CSameAs<T, u32char>)
			{
				constexpr TStaticArray<uint8, 4> BufferBE = { 0x00, 0x00, 0xFE, 0xFF };
				constexpr TStaticArray<uint8, 4> BufferLE = { 0xFF, 0xFE, 0x00, 0x00 };

				if      (Encoding == FileSystem::EEncoding::UTF32BE) { if (std::fwrite(BufferBE.GetData() , 1, BufferBE.Num(), File) != BufferBE.Num()) return false; }
				else if (Encoding == FileSystem::EEncoding::UTF32LE) { if (std::fwrite(BufferLE.GetData() , 1, BufferLE.Num(), File) != BufferLE.Num()) return false; }

				else if (std::fwrite(U32TEXT("\uFEFF"), 1, sizeof(T), File) != sizeof(T)) return false;
			}
		}

		check(Math::EEndian::Native == Math::EEndian::Big || Math::EEndian::Native == Math::EEndian::Little);

		const bool bByteSwap =
			Math::EEndian::Native == Math::EEndian::Big    ? Encoding == FileSystem::EEncoding::UTF16LE || Encoding == FileSystem::EEncoding::UTF32LE :
			Math::EEndian::Native == Math::EEndian::Little ? Encoding == FileSystem::EEncoding::UTF16BE || Encoding == FileSystem::EEncoding::UTF32BE : false;

		for (T Char : String)
		{
#			if PLATFORM_WINDOWS
			{
				if (Char == LITERAL(T, '\n'))
				{
					T Return = LITERAL(T, '\r');

					if (bByteSwap) Return = Math::ByteSwap(static_cast<TMakeUnsigned<T>>(Return));

					if (std::fwrite(&Return, 1, sizeof(T), File) != sizeof(T)) return false;
				}
			}
#			endif

			if (bByteSwap) Char = Math::ByteSwap(static_cast<TMakeUnsigned<T>>(Char));

			if (std::fwrite(&Char, 1, sizeof(T), File) != sizeof(T)) return false;
		}

		FileGuard.Release();

		if (std::fclose(File) != 0) return false;

		return true;
	}

	FString PathWithNull;

	PathWithNull.Reserve(Path.Num() + 1);

	PathWithNull += Path;
	PathWithNull += '\0';

	switch (Encoding)
	{
	case FileSystem::EEncoding::Narrow:  { FString    Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::Narrow,  bWithBOM)) return false; break; }
	case FileSystem::EEncoding::Wide:    { FWString   Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::Wide,    bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF8:    { FU8String  Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::UTF8,    bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF16BE: { FU16String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::UTF16BE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF16LE: { FU16String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::UTF16LE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF32BE: { FU32String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::UTF32BE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF32LE: { FU32String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, PathWithNull, FileSystem::EEncoding::UTF32LE, bWithBOM)) return false; break; }
	default: check_no_entry(); return false;
	}

	return true;
}

template REDCRAFTUTILITY_API bool SaveStringToFile<char>   (FStringView,    FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool SaveStringToFile<wchar>  (FWStringView,   FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool SaveStringToFile<u8char> (FU8StringView,  FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool SaveStringToFile<u16char>(FU16StringView, FStringView, FileSystem::EEncoding, bool);
template REDCRAFTUTILITY_API bool SaveStringToFile<u32char>(FU32StringView, FStringView, FileSystem::EEncoding, bool);

size_t FileSize(FStringView Path)
{
	if (!FileSystem::Exists(Path)) return static_cast<size_t>(-1);

	FILE* File = std::fopen(*Path, "rb");

	if (File == nullptr) return static_cast<size_t>(-1);

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fseek(File, 0, SEEK_END) != 0) return static_cast<size_t>(-1);

	const long Length = std::ftell(File);

	if (!Math::IsWithin(Length, 0, TNumericLimits<long>::Max())) return static_cast<size_t>(-1);

	FileGuard.Release();

	if (std::fclose(File) != 0) return static_cast<size_t>(-1);

	return Length;
}

bool Delete(FStringView Path)
{
	return std::remove(*Path) == 0;
}

bool Exists(FStringView Path)
{
#	if PLATFORM_WINDOWS
	{
		DWORD Attributes = GetFileAttributesA(*Path);

		if (Attributes == INVALID_FILE_ATTRIBUTES) return false;

		return !(Attributes & FILE_ATTRIBUTE_DIRECTORY);
	}
#	elif PLATFORM_LINUX
	{
		struct stat FileInfo;

		FileInfo.st_size = -1;

		if (stat(*Path, &FileInfo) != 0) return false;

		if (!S_ISREG(FileInfo.st_mode)) return false;

		return true;
	}
#	endif

	return false;
}

bool Copy(FStringView Destination, FStringView Source)
{
	if (!FileSystem::Exists(Source)) return false;

	FILE* FileA = std::fopen(*Source, "rb");

	if (FileA == nullptr) return false;

	auto FileGuardA = TScopeCallback([=] { Ignore = std::fclose(FileA); });

	FILE* FileB = std::fopen(*Destination, "wb");

	if (FileB == nullptr) return false;

	auto FileGuardB = TScopeCallback([=] { Ignore = std::fclose(FileB); });

	size_t ReadSize;

	constexpr size_t BufferSize = 4096;

	TStaticArray<uint8, BufferSize> Buffer;

	do
	{
		ReadSize = std::fread(Buffer.GetData(), 1, BufferSize, FileA);

		if (std::fwrite(Buffer.GetData(), 1, ReadSize, FileB) != ReadSize) return false;
	}
	while (ReadSize == BufferSize);

	FileGuardA.Release();

	if (std::fclose(FileA) != 0) return false;

	FileGuardB.Release();

	if (std::fclose(FileB) != 0) return false;

	return true;
}

bool Rename(FStringView Destination, FStringView Source)
{
	return std::rename(*Source, *Destination) == 0;
}

bool CreateDirectory(FStringView Path, bool bRecursive /* = false */)
{
	if (Path.Num() == 0) return false;

	if (bRecursive)
	{
		if (Path.Back() == '/' || Path.Back() == '\\') Path = Path.First(Path.Num() - 1);

		FStringView Parent = Path.First(Path.FindLastOf("/\\"));

		if (!FileSystem::ExistsDirectory(Parent) && !FileSystem::CreateDirectory(Parent, true)) return false;
	}

#	if PLATFORM_WINDOWS
	{
		return CreateDirectoryA(*Path, nullptr) != 0;
	}
#	elif PLATFORM_LINUX
	{
		return mkdir(*Path, 0755) == 0;
	}
#	endif

	return false;
}

bool DeleteDirectory(FStringView Path, bool bRecursive /* = false */)
{
	if (bRecursive)
	{
		FString Temp;

		bool bSuccessfully = FileSystem::IterateDirectory(Path, [&](FStringView File, bool bIsDirectory) -> bool
		{
			Temp.Reset(false);

			Temp += Path;
			Temp += '/';
			Temp += File;
			Temp += '\0';

			if (bIsDirectory)
			{
				if (!FileSystem::DeleteDirectory(Temp, true)) return false;
			}

			else
			{
				if (!FileSystem::Delete(Temp)) return false;
			}

			return true;
		});

		if (!bSuccessfully) return false;
	}

#	if PLATFORM_WINDOWS
	{
		return RemoveDirectoryA(*Path) != 0;
	}
#	elif PLATFORM_LINUX
	{
		return rmdir(*Path) == 0;
	}
#	endif

	return false;
}

bool ExistsDirectory(FStringView Path)
{
#	if PLATFORM_WINDOWS
	{
		DWORD Attributes = GetFileAttributesA(*Path);

		if (Attributes == INVALID_FILE_ATTRIBUTES) return false;

		return Attributes & FILE_ATTRIBUTE_DIRECTORY;
	}
#	elif PLATFORM_LINUX
	{
		DIR* Directory = opendir(*Path);

		if (Directory == nullptr) return false;

		Ignore = closedir(Directory);

		return true;
	}
#	endif

	return false;
}

bool IterateDirectory(FStringView Path, TFunctionRef<bool(FStringView /* Path */, bool /* bIsDirectory */)> Visitor)
{
#	if PLATFORM_WINDOWS
	{
		FString FindPath;

		FindPath.Reserve(Path.Num() + 3);

		FindPath += Path;
		FindPath += '\\';
		FindPath += '*';
		FindPath += '\0';

		WIN32_FIND_DATA FindData;

		HANDLE FindHandle = FindFirstFileA(*FindPath, &FindData);

		auto FindGuard = TScopeCallback([=] { Ignore = FindClose(FindHandle); });

		if (FindHandle == INVALID_HANDLE_VALUE) return false;

		do
		{
			const FStringView FilePath = FindData.cFileName;

			if (FilePath == "." || FilePath == "..") continue;

			const bool bIsDirectory = (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (!Visitor(FilePath, bIsDirectory)) return false;
		}
		while (FindNextFileA(FindHandle, &FindData) != 0);

		FindGuard.Release();

		if (!FindClose(FindHandle)) return false;

		return true;
	}
#	elif PLATFORM_LINUX
	{
		DIR* Directory = opendir(*Path);

		if (Directory == nullptr) return false;

		auto DirectoryGuard = TScopeCallback([=] { Ignore = closedir(Directory); });

		dirent* Entry;

		while ((Entry = readdir(Directory)) != nullptr)
		{
			const FStringView FilePath = Entry->d_name;

			if (FilePath == "." || FilePath == "..") continue;

			const bool bIsDirectory = Entry->d_type == DT_DIR;

			if (!Visitor(FilePath, bIsDirectory)) return false;
		}

		DirectoryGuard.Release();

		if (closedir(Directory) != 0) return false;

		return true;
	}
#	endif

	return false;
}

NAMESPACE_END(FileSystem)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
