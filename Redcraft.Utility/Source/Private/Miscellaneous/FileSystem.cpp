#include <Miscellaneous/FileSystem.h>

#include "Numerics/Bit.h"
#include "Templates/ScopeHelper.h"
#include "Containers/StaticArray.h"

#include <cstdio>

#pragma warning(push)
#pragma warning(disable: 4996)

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(FileSystem)

bool LoadFileToArray(TArray<uint8>& Result, FStringView Path)
{
	FILE* File = std::fopen(*Path, "rb");

	if (File == nullptr) return false;

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fseek(File, 0, SEEK_END) != 0) return false;

	const long Length = std::ftell(File);

	if (Length == -1) return false;

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
	FILE* File = std::fopen(*Path, "rb");

	if (File == nullptr) return false;

	auto FileGuard = TScopeCallback([=] { Ignore = std::fclose(File); });

	if (std::fseek(File, 0, SEEK_END) != 0) return false;

	long Length = std::ftell(File);

	if (Length == -1) return false;

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

			if constexpr (sizeof(U) > 1) if (bByteSwap) Char = Math::ByteSwap(Char);

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

					if constexpr (sizeof(T) > 1) if (bByteSwap) Return = Math::ByteSwap(Return);

					if (std::fwrite(&Return, 1, sizeof(T), File) != sizeof(T)) return false;
				}
			}
#			endif

			if constexpr (sizeof(T) > 1) if (bByteSwap) Char = Math::ByteSwap(Char);

			if (std::fwrite(&Char, 1, sizeof(T), File) != sizeof(T)) return false;
		}

		FileGuard.Release();

		if (std::fclose(File) != 0) return false;

		return true;
	}

	switch (Encoding)
	{
	case FileSystem::EEncoding::Narrow:  { FString    Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::Narrow,  bWithBOM)) return false; break; }
	case FileSystem::EEncoding::Wide:    { FWString   Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::Wide,    bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF8:    { FU8String  Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::UTF8,    bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF16BE: { FU16String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::UTF16BE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF16LE: { FU16String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::UTF16LE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF32BE: { FU32String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::UTF32BE, bWithBOM)) return false; break; }
	case FileSystem::EEncoding::UTF32LE: { FU32String Temp; if (!Temp.DecodeFrom(String)) return false; if (!FileSystem::SaveStringToFile(Temp, Path, FileSystem::EEncoding::UTF32LE, bWithBOM)) return false; break; }
	default: check_no_entry(); return false;
	}

	return true;
}

NAMESPACE_END(FileSystem)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END

#pragma warning(pop)
