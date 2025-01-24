#pragma once

#include "CoreTypes.h"
#include "TypeTraits/TypeTraits.h"
#include "Templates/Utility.h"
#include "Templates/Function.h"
#include "Containers/Array.h"
#include "Strings/StringView.h"
#include "Strings/String.h"

NAMESPACE_REDCRAFT_BEGIN
	NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(FileSystem)

/** The encoding of the text file. */
enum class EEncoding : uint8
{
	Default,
	Narrow,
	Wide,
	UTF8,
	UTF16BE,
	UTF16LE,
	UTF32BE,
	UTF32LE,
};

/** Loads the file at the specified path into the byte array. */
REDCRAFTUTILITY_API bool LoadFileToArray(TArray<uint8>& Result, FStringView Path);

/** Saves the byte array to the file at the specified path. */
REDCRAFTUTILITY_API bool SaveArrayToFile(TArrayView<const uint8> Data, FStringView Path);

/**
 * Loads the file at the specified path into the string.
 *
 * @param Result   - The string to load the file into.
 * @param Path     - The path to the file to load.
 * @param Encoding - The encoding of the file. The default value indicates automatic detection.
 * @param bVerify  - Whether to verify the character validity of the file.
 *
 * @return true if the file was successfully loaded, false otherwise.
 */
template <CCharType T>
REDCRAFTUTILITY_API bool LoadFileToString(TString<T>& Result, FStringView Path, FileSystem::EEncoding Encoding = FileSystem::EEncoding::Default, bool bVerify = false);

/**
 * Saves the string to the file at the specified path.
 *
 * @param String   - The string to save to the file.
 * @param Path     - The path to the file to save.
 * @param Encoding - The encoding of the file. The default value indicates the same as the string.
 * @param bWithBOM - Whether to write the BOM character at the beginning of the file. Not valid for narrow and wide encoding.
 *
 * @return true if the file was successfully saved, false otherwise.
 */
template <CCharType T>
REDCRAFTUTILITY_API bool SaveStringToFile(TStringView<T> String, FStringView Path, FileSystem::EEncoding Encoding = FileSystem::EEncoding::Default, bool bWithBOM = true);

/**
 * Saves the string to the file at the specified path.
 *
 * @param String   - The string to save to the file.
 * @param Path     - The path to the file to save.
 * @param Encoding - The encoding of the file. The default value indicates the same as the string.
 * @param bWithBOM - Whether to write the BOM character at the beginning of the file. Not valid for narrow and wide encoding.
 *
 * @return true if the file was successfully saved, false otherwise.
 */
template <typename T> requires (CConvertibleTo<T&&, FStringView> || CConvertibleTo<T&&, FWStringView>
	|| CConvertibleTo<T&&, FU8StringView> || CConvertibleTo<T&&, FU16StringView> || CConvertibleTo<T&&, FU32StringView>)
bool SaveStringToFile(T&& String, FStringView Path, FileSystem::EEncoding Encoding = FileSystem::EEncoding::Default, bool bWithBOM = true)
{
	if      constexpr (CConvertibleTo<T&&, FStringView>)    return SaveStringToFile(FStringView   (Forward<T>(String)), Path, Encoding, bWithBOM);
	else if constexpr (CConvertibleTo<T&&, FWStringView>)   return SaveStringToFile(FWStringView  (Forward<T>(String)), Path, Encoding, bWithBOM);
	else if constexpr (CConvertibleTo<T&&, FU8StringView>)  return SaveStringToFile(FU8StringView (Forward<T>(String)), Path, Encoding, bWithBOM);
	else if constexpr (CConvertibleTo<T&&, FU16StringView>) return SaveStringToFile(FU16StringView(Forward<T>(String)), Path, Encoding, bWithBOM);
	else if constexpr (CConvertibleTo<T&&, FU32StringView>) return SaveStringToFile(FU32StringView(Forward<T>(String)), Path, Encoding, bWithBOM);

	return false;
}

REDCRAFTUTILITY_API size_t FileSize(FStringView Path);

REDCRAFTUTILITY_API bool Delete(FStringView Path);

REDCRAFTUTILITY_API bool Exists(FStringView Path);

REDCRAFTUTILITY_API bool Copy(FStringView Destination, FStringView Source);

REDCRAFTUTILITY_API bool Rename(FStringView Destination, FStringView Source);

REDCRAFTUTILITY_API bool CreateDirectory(FStringView Path, bool bRecursive = false);

REDCRAFTUTILITY_API bool DeleteDirectory(FStringView Path, bool bRecursive = false);

REDCRAFTUTILITY_API bool ExistsDirectory(FStringView Path);

REDCRAFTUTILITY_API bool IterateDirectory(FStringView Path, TFunctionRef<bool(FStringView /* Path */, bool /* bIsDirectory */)> Visitor);

NAMESPACE_END(FileSystem)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
