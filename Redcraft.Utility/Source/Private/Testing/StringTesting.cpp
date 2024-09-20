#include "Testing/StringTesting.h"

#include "String/Char.h"
#include "Memory/Memory.h"
#include "String/CString.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestString()
{
	TestChar();
	TestCString();
}

void TestChar()
{
	{
		always_check(!CCharType<int>);
		always_check(CCharType<char>);
		always_check(CCharType<wchar_t>);
		always_check(CCharType<char8_t>);
		always_check(CCharType<char16_t>);
		always_check(CCharType<char32_t>);
	}

	{
		always_check(FChar::IsAlnum(TEXT('0')));
		always_check(FChar::IsAlpha(TEXT('A')));
		always_check(FChar::IsLower(TEXT('a')));
		always_check(FChar::IsUpper(TEXT('A')));
		always_check(FChar::IsDigit(TEXT('0')));
		always_check(FChar::IsCntrl(TEXT('\n')));
		always_check(FChar::IsGraph(TEXT('!')));
		always_check(FChar::IsSpace(TEXT('\t')));
		always_check(FChar::IsBlank(TEXT(' ')));
		always_check(FChar::IsPrint(TEXT('#')));
		always_check(FChar::IsPunct(TEXT('[')));
	}

	{
		always_check(FWChar::IsAlnum(WTEXT('0')));
		always_check(FWChar::IsAlpha(WTEXT('A')));
		always_check(FWChar::IsLower(WTEXT('a')));
		always_check(FWChar::IsUpper(WTEXT('A')));
		always_check(FWChar::IsDigit(WTEXT('0')));
		always_check(FWChar::IsCntrl(WTEXT('\n')));
		always_check(FWChar::IsGraph(WTEXT('!')));
		always_check(FWChar::IsSpace(WTEXT('\t')));
		always_check(FWChar::IsBlank(WTEXT(' ')));
		always_check(FWChar::IsPrint(WTEXT('#')));
		always_check(FWChar::IsPunct(WTEXT('[')));
	}

	{
		always_check(FU8Char::IsAlnum(U8TEXT('0')));
		always_check(FU8Char::IsAlpha(U8TEXT('A')));
		always_check(FU8Char::IsLower(U8TEXT('a')));
		always_check(FU8Char::IsUpper(U8TEXT('A')));
		always_check(FU8Char::IsDigit(U8TEXT('0')));
		always_check(FU8Char::IsCntrl(U8TEXT('\n')));
		always_check(FU8Char::IsGraph(U8TEXT('!')));
		always_check(FU8Char::IsSpace(U8TEXT('\t')));
		always_check(FU8Char::IsBlank(U8TEXT(' ')));
		always_check(FU8Char::IsPrint(U8TEXT('#')));
		always_check(FU8Char::IsPunct(U8TEXT('[')));
	}

	{
//		always_check(FU16Char::IsAlnum(U16TEXT('0')));
//		always_check(FU16Char::IsAlpha(U16TEXT('A')));
//		always_check(FU16Char::IsLower(U16TEXT('a')));
//		always_check(FU16Char::IsUpper(U16TEXT('A')));
		always_check(FU16Char::IsDigit(U16TEXT('0')));
		always_check(FU16Char::IsCntrl(U16TEXT('\n')));
//		always_check(FU16Char::IsGraph(U16TEXT('!')));
		always_check(FU16Char::IsSpace(U16TEXT('\t')));
		always_check(FU16Char::IsBlank(U16TEXT(' ')));
//		always_check(FU16Char::IsPrint(U16TEXT('#')));
//		always_check(FU16Char::IsPunct(U16TEXT('[')));
	}

	{
//		always_check(FU32Char::IsAlnum(U32TEXT('0')));
//		always_check(FU32Char::IsAlpha(U32TEXT('A')));
//		always_check(FU32Char::IsLower(U32TEXT('a')));
//		always_check(FU32Char::IsUpper(U32TEXT('A')));
		always_check(FU32Char::IsDigit(U32TEXT('0')));
		always_check(FU32Char::IsCntrl(U32TEXT('\n')));
//		always_check(FU32Char::IsGraph(U32TEXT('!')));
		always_check(FU32Char::IsSpace(U32TEXT('\t')));
		always_check(FU32Char::IsBlank(U32TEXT(' ')));
//		always_check(FU32Char::IsPrint(U32TEXT('#')));
//		always_check(FU32Char::IsPunct(U32TEXT('[')));
	}

	{
		always_check(!FChar::IsAlnum(TEXT('$')));
		always_check(!FChar::IsAlpha(TEXT('0')));
		always_check(!FChar::IsLower(TEXT('A')));
		always_check(!FChar::IsUpper(TEXT('a')));
		always_check(!FChar::IsDigit(TEXT('I')));
		always_check(!FChar::IsCntrl(TEXT('_')));
		always_check(!FChar::IsGraph(TEXT(' ')));
		always_check(!FChar::IsSpace(TEXT('=')));
		always_check(!FChar::IsBlank(TEXT('\r')));
		always_check(!FChar::IsPrint(TEXT('\n')));
		always_check(!FChar::IsPunct(TEXT('H')));
	}

	{
		always_check(!FWChar::IsAlnum(WTEXT('$')));
		always_check(!FWChar::IsAlpha(WTEXT('0')));
		always_check(!FWChar::IsLower(WTEXT('A')));
		always_check(!FWChar::IsUpper(WTEXT('a')));
		always_check(!FWChar::IsDigit(WTEXT('I')));
		always_check(!FWChar::IsCntrl(WTEXT('_')));
		always_check(!FWChar::IsGraph(WTEXT(' ')));
		always_check(!FWChar::IsSpace(WTEXT('=')));
		always_check(!FWChar::IsBlank(WTEXT('\r')));
		always_check(!FWChar::IsPrint(WTEXT('\n')));
		always_check(!FWChar::IsPunct(WTEXT('H')));
	}

	{
		always_check(!FU8Char::IsAlnum(U8TEXT('$')));
		always_check(!FU8Char::IsAlpha(U8TEXT('0')));
		always_check(!FU8Char::IsLower(U8TEXT('A')));
		always_check(!FU8Char::IsUpper(U8TEXT('a')));
		always_check(!FU8Char::IsDigit(U8TEXT('I')));
		always_check(!FU8Char::IsCntrl(U8TEXT('_')));
		always_check(!FU8Char::IsGraph(U8TEXT(' ')));
		always_check(!FU8Char::IsSpace(U8TEXT('=')));
		always_check(!FU8Char::IsBlank(U8TEXT('\r')));
		always_check(!FU8Char::IsPrint(U8TEXT('\n')));
		always_check(!FU8Char::IsPunct(U8TEXT('H')));
	}

	{
//		always_check(!FU16Char::IsAlnum(U16TEXT('$')));
//		always_check(!FU16Char::IsAlpha(U16TEXT('0')));
//		always_check(!FU16Char::IsLower(U16TEXT('A')));
//		always_check(!FU16Char::IsUpper(U16TEXT('a')));
		always_check(!FU16Char::IsDigit(U16TEXT('I')));
		always_check(!FU16Char::IsCntrl(U16TEXT('_')));
//		always_check(!FU16Char::IsGraph(U16TEXT(' ')));
		always_check(!FU16Char::IsSpace(U16TEXT('=')));
		always_check(!FU16Char::IsBlank(U16TEXT('\r')));
//		always_check(!FU16Char::IsPrint(U16TEXT('\n')));
//		always_check(!FU16Char::IsPunct(U16TEXT('H')));
	}

	{
//		always_check(!FU32Char::IsAlnum(U32TEXT('$')));
//		always_check(!FU32Char::IsAlpha(U32TEXT('0')));
//		always_check(!FU32Char::IsLower(U32TEXT('A')));
//		always_check(!FU32Char::IsUpper(U32TEXT('a')));
		always_check(!FU32Char::IsDigit(U32TEXT('I')));
		always_check(!FU32Char::IsCntrl(U32TEXT('_')));
//		always_check(!FU32Char::IsGraph(U32TEXT(' ')));
		always_check(!FU32Char::IsSpace(U32TEXT('=')));
		always_check(!FU32Char::IsBlank(U32TEXT('\r')));
//		always_check(!FU32Char::IsPrint(U32TEXT('\n')));
//		always_check(!FU32Char::IsPunct(U32TEXT('H')));
	}

	{
		always_check( FChar::IsDigit(TEXT('F'), 16));
		always_check(!FChar::IsDigit(TEXT('G'), 16));
		always_check( FWChar::IsDigit(WTEXT('F'), 16));
		always_check(!FWChar::IsDigit(WTEXT('G'), 16));
		always_check( FU8Char::IsDigit(U8TEXT('F'), 16));
		always_check(!FU8Char::IsDigit(U8TEXT('G'), 16));
		always_check( FU16Char::IsDigit(U16TEXT('F'), 16));
		always_check(!FU16Char::IsDigit(U16TEXT('G'), 16));
		always_check( FU32Char::IsDigit(U32TEXT('F'), 16));
		always_check(!FU32Char::IsDigit(U32TEXT('G'), 16));
	}

	{
		always_check(FChar::ToLower(TEXT('i')) == TEXT('i'));
		always_check(FChar::ToUpper(TEXT('l')) == TEXT('L'));
		always_check(FWChar::ToUpper(WTEXT('l')) == WTEXT('L'));
		always_check(FWChar::ToLower(WTEXT('i')) == WTEXT('i'));
		always_check(FU8Char::ToLower(U8TEXT('i')) == U8TEXT('i'));
		always_check(FU8Char::ToUpper(U8TEXT('l')) == U8TEXT('L'));
//		always_check(FU16Char::ToLower(U16TEXT('i')) == U16TEXT('i'));
//		always_check(FU16Char::ToUpper(U16TEXT('l')) == U16TEXT('L'));
//		always_check(FU32Char::ToLower(U32TEXT('i')) == U32TEXT('i'));
//		always_check(FU32Char::ToUpper(U32TEXT('l')) == U32TEXT('L'));
	}

	{
		always_check(0x0 == FChar::ToDigit(TEXT('0')));
		always_check(0xF == FChar::ToDigit(TEXT('f')));
		always_check(0xF == FChar::ToDigit(TEXT('F')));
		always_check(0x0 == FWChar::ToDigit(WTEXT('0')));
		always_check(0xF == FWChar::ToDigit(WTEXT('f')));
		always_check(0xF == FWChar::ToDigit(WTEXT('F')));
		always_check(0x0 == FU8Char::ToDigit(U8TEXT('0')));
		always_check(0xF == FU8Char::ToDigit(U8TEXT('f')));
		always_check(0xF == FU8Char::ToDigit(U8TEXT('F')));
		always_check(0x0 == FU16Char::ToDigit(U16TEXT('0')));
		always_check(0xF == FU16Char::ToDigit(U16TEXT('f')));
		always_check(0xF == FU16Char::ToDigit(U16TEXT('F')));
		always_check(0x0 == FU16Char::ToDigit(U16TEXT('0')));
		always_check(0xF == FU16Char::ToDigit(U16TEXT('f')));
		always_check(0xF == FU16Char::ToDigit(U16TEXT('F')));
	}

	{
		always_check(TEXT('0') == FChar::FromDigit(0x0));
		always_check(TEXT('f') != FChar::FromDigit(0xF));
		always_check(TEXT('F') == FChar::FromDigit(0xF));
		always_check(WTEXT('0') == FWChar::FromDigit(0x0));
		always_check(WTEXT('f') != FWChar::FromDigit(0xF));
		always_check(WTEXT('F') == FWChar::FromDigit(0xF));
		always_check(U8TEXT('0') == FU8Char::FromDigit(0x0));
		always_check(U8TEXT('f') != FU8Char::FromDigit(0xF));
		always_check(U8TEXT('F') == FU8Char::FromDigit(0xF));
		always_check(U16TEXT('0') == FU16Char::FromDigit(0x0));
		always_check(U16TEXT('f') != FU16Char::FromDigit(0xF));
		always_check(U16TEXT('F') == FU16Char::FromDigit(0xF));
		always_check(U16TEXT('0') == FU16Char::FromDigit(0x0));
		always_check(U16TEXT('f') != FU16Char::FromDigit(0xF));
		always_check(U16TEXT('F') == FU16Char::FromDigit(0xF));
	}
}

void TestCString()
{
	auto TestTCString = []<typename T>(TInPlaceType<T>)
	{
		constexpr size_t BUFFER_SIZE = 64;

		T StrA[BUFFER_SIZE];
		T StrB[BUFFER_SIZE];
		T StrC[BUFFER_SIZE];
		T StrD[BUFFER_SIZE];

		always_check(TCString<T>::Copy(StrA, IGNORE_SIZE, LITERAL(T, "Hello"), IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Copy(StrB, IGNORE_SIZE, LITERAL(T, "Hello"), IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Copy(StrC, IGNORE_SIZE, LITERAL(T, "World"), IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Copy(StrD, IGNORE_SIZE, LITERAL(T, "     "), IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::Length(StrA,           4) == 4);
		always_check(TCString<T>::Length(StrA, BUFFER_SIZE) == 5);
		always_check(TCString<T>::Length(StrA, IGNORE_SIZE) == 5);

		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrB, IGNORE_SIZE) == TCString<T>::Compare(StrA, BUFFER_SIZE, StrB, BUFFER_SIZE));
		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrC, IGNORE_SIZE) == TCString<T>::Compare(StrA, BUFFER_SIZE, StrC, BUFFER_SIZE));
		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrC, IGNORE_SIZE) < 0);

		Memory::Memzero(StrD);

		always_check(TCString<T>::Compare(StrA, BUFFER_SIZE, StrD, BUFFER_SIZE) > 0);
		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrD, IGNORE_SIZE) > 0);

		always_check(TCString<T>::Copy(StrD, IGNORE_SIZE, StrA, IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::Compare(StrA, BUFFER_SIZE, StrD, BUFFER_SIZE) == 0);
		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrD, IGNORE_SIZE) == 0);

		Memory::Memzero(StrC);
		Memory::Memzero(StrD);

		always_check(TCString<T>::Copy(StrD, 4, StrA, IGNORE_SIZE) == nullptr);
		
		always_check(TCString<T>::Compare(StrC, BUFFER_SIZE, StrD, BUFFER_SIZE) == 0);
		always_check(TCString<T>::Compare(StrC, IGNORE_SIZE, StrD, IGNORE_SIZE) == 0);

		always_check(TCString<T>::Copy(StrD, IGNORE_SIZE, StrA, 4) != nullptr);

		always_check(TCString<T>::Length(StrD, IGNORE_SIZE) == 4);
		
		always_check(TCString<T>::Compare(StrA,           4, StrD,           4) == 0);
		always_check(TCString<T>::Compare(StrA, IGNORE_SIZE, StrD, IGNORE_SIZE) > 0);

		always_check(TCString<T>::Copy(   StrB, IGNORE_SIZE, LITERAL(T, "World!"), 5) != nullptr);
		always_check(TCString<T>::Compare(StrB, IGNORE_SIZE, LITERAL(T, "World" ), IGNORE_SIZE) == 0);

		Memory::Memzero(StrD);

		always_check(TCString<T>::Cat(StrD, 8, StrA,            IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Cat(StrD, 8, LITERAL(T, " "), IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Cat(StrD, 8, StrB,            IGNORE_SIZE) == nullptr);

		always_check(TCString<T>::Compare(StrD, IGNORE_SIZE, LITERAL(T, "Hello "), IGNORE_SIZE) == 0);

		Memory::Memzero(StrD);
		
		always_check(TCString<T>::Cat(StrD, IGNORE_SIZE, StrA,            IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Cat(StrD, IGNORE_SIZE, LITERAL(T, " "), IGNORE_SIZE) != nullptr);
		always_check(TCString<T>::Cat(StrD, IGNORE_SIZE, StrB,            IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::Compare(StrD, IGNORE_SIZE, LITERAL(T, "Hello World"), IGNORE_SIZE) == 0);

		always_check(TCString<T>::Copy(StrA, IGNORE_SIZE, LITERAL(T, "Hello"), IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T, '\0'); }) == StrA + 5);
		always_check(TCString<T>::Find(StrA, BUFFER_SIZE, [](T A) { return A == LITERAL(T, '\0'); }) == StrA + 5);
		always_check(TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T,  'o'); }) == StrA + 4);
		always_check(TCString<T>::Find(StrA,           4, [](T A) { return A == LITERAL(T,  'o'); }) == nullptr);

		always_check(TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T, 'o'); })
			      == TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T, 'o'); }, ESearchDirection::FromEnd));

		always_check(TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T, 'l'); })
			      != TCString<T>::Find(StrA, IGNORE_SIZE, [](T A) { return A == LITERAL(T, 'l'); }, ESearchDirection::FromEnd));
				  
		always_check(TCString<T>::Find(StrA, BUFFER_SIZE, [](T A) { return A == LITERAL(T, 'o'); })
			      == TCString<T>::Find(StrA, BUFFER_SIZE, [](T A) { return A == LITERAL(T, 'o'); }, ESearchDirection::FromEnd));

		always_check(TCString<T>::Find(StrA, BUFFER_SIZE, [](T A) { return A == LITERAL(T, 'l'); })
			      != TCString<T>::Find(StrA, BUFFER_SIZE, [](T A) { return A == LITERAL(T, 'l'); }, ESearchDirection::FromEnd));
				  
		always_check(TCString<T>::Find(StrA,           4, [](T A) { return A == LITERAL(T, 'o'); })
			      == TCString<T>::Find(StrA,           4, [](T A) { return A == LITERAL(T, 'o'); }, ESearchDirection::FromEnd));

		always_check(TCString<T>::Find(StrA,           3, [](T A) { return A == LITERAL(T, 'l'); })
			      == TCString<T>::Find(StrA,           3, [](T A) { return A == LITERAL(T, 'l'); }, ESearchDirection::FromEnd));

		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, '\0')) == StrA + 5);
		always_check(TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T, '\0')) == StrA + 5);
		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T,  'o')) == StrA + 4);
		always_check(TCString<T>::FindChar(StrA,           4, LITERAL(T,  'o')) == nullptr);

		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, 'o'))
			      == TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, 'o'), ESearchDirection::FromEnd));

		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, 'l'))
			      != TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, 'l'), ESearchDirection::FromEnd));
				  
		always_check(TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T, 'o'))
			      == TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T, 'o'), ESearchDirection::FromEnd));

		always_check(TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T, 'l'))
			      != TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T, 'l'), ESearchDirection::FromEnd));
				  
		always_check(TCString<T>::FindChar(StrA,           4, LITERAL(T, 'o'))
			      == TCString<T>::FindChar(StrA,           4, LITERAL(T, 'o'), ESearchDirection::FromEnd));

		always_check(TCString<T>::FindChar(StrA,           3, LITERAL(T, 'l'))
			      == TCString<T>::FindChar(StrA,           3, LITERAL(T, 'l'), ESearchDirection::FromEnd));

		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T,  ""), IGNORE_SIZE) == nullptr);
		always_check(TCString<T>::FindChar(StrA, BUFFER_SIZE, LITERAL(T,  ""), IGNORE_SIZE) == nullptr);
		always_check(TCString<T>::FindChar(StrA, IGNORE_SIZE, LITERAL(T, "o"), IGNORE_SIZE) == StrA + 4);
		always_check(TCString<T>::FindChar(StrA,           4, LITERAL(T, "o"), IGNORE_SIZE) == nullptr);

		always_check(TCString<T>::Copy(StrA, IGNORE_SIZE, LITERAL(T, "HIH"), IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T, '\0')) == StrA);
		always_check(TCString<T>::FindNotChar(StrA, BUFFER_SIZE, LITERAL(T, '\0')) == StrA);
		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T,  'I')) == StrA);
		always_check(TCString<T>::FindNotChar(StrA,           2, LITERAL(T,  'I')) == StrA);
		
		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T, '\0'), ESearchDirection::FromEnd) == StrA + 2);
		always_check(TCString<T>::FindNotChar(StrA, BUFFER_SIZE, LITERAL(T, '\0'), ESearchDirection::FromEnd) == StrA + 2);
		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T,  'I'), ESearchDirection::FromEnd) == StrA + 3);
		always_check(TCString<T>::FindNotChar(StrA,           2, LITERAL(T,  'I'), ESearchDirection::FromEnd) == StrA + 0);

		always_check(TCString<T>::Copy(StrA, IGNORE_SIZE, LITERAL(T, "HIJIH"), IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T, "HIJ"), IGNORE_SIZE) == nullptr);
		always_check(TCString<T>::FindNotChar(StrA, BUFFER_SIZE, LITERAL(T, "HIJ"), IGNORE_SIZE) == nullptr);

		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T, "H J"), IGNORE_SIZE) == StrA + 1);
		always_check(TCString<T>::FindNotChar(StrA, BUFFER_SIZE, LITERAL(T, "H J"), IGNORE_SIZE) == StrA + 1);

		always_check(TCString<T>::FindNotChar(StrA, IGNORE_SIZE, LITERAL(T, "H J"), IGNORE_SIZE, ESearchDirection::FromEnd) == StrA + 3);
		always_check(TCString<T>::FindNotChar(StrA, BUFFER_SIZE, LITERAL(T, "H J"), IGNORE_SIZE, ESearchDirection::FromEnd) == StrA + 3);

		always_check(TCString<T>::Copy(StrA, IGNORE_SIZE, LITERAL(T, "01234567890123456789"), IGNORE_SIZE) != nullptr);

		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                               ""), IGNORE_SIZE)                            == StrA);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                               ""), IGNORE_SIZE, ESearchDirection::FromEnd) == StrA + 20);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                            "345"), IGNORE_SIZE)                            == StrA + 3);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                            "345"), IGNORE_SIZE, ESearchDirection::FromEnd) == StrA + 13);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T, "012345678901234567890123456789"), IGNORE_SIZE)                            == nullptr);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T, "012345678901234567890123456789"), IGNORE_SIZE, ESearchDirection::FromEnd) == nullptr);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                            "ABC"), IGNORE_SIZE)                            == nullptr);
		always_check(TCString<T>::FindString(StrA, IGNORE_SIZE, LITERAL(T,                            "ABC"), IGNORE_SIZE, ESearchDirection::FromEnd) == nullptr);
	};

	TestTCString(InPlaceType<char>);
	TestTCString(InPlaceType<wchar_t>);
	TestTCString(InPlaceType<char8_t>);
	TestTCString(InPlaceType<char16_t>);
	TestTCString(InPlaceType<char32_t>);
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
