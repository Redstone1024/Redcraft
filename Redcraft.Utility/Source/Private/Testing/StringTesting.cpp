#include "Testing/StringTesting.h"

#include "String/Char.h"
#include "Memory/Memory.h"
#include "String/String.h"
#include "String/StringView.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_BEGIN(Testing)

void TestString()
{
	TestChar();
	TestStringView();
	TestTemplateString();
}

void TestChar()
{
	{
		always_check(!CCharType<int>);
		always_check(CCharType<char>);
		always_check(CCharType<wchar>);
		always_check(CCharType<u8char>);
		always_check(CCharType<u16char>);
		always_check(CCharType<u32char>);
		always_check(CCharType<unicodechar>);
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
		always_check(FU16Char::IsAlnum(U16TEXT('0')));
		always_check(FU16Char::IsAlpha(U16TEXT('A')));
		always_check(FU16Char::IsLower(U16TEXT('a')));
		always_check(FU16Char::IsUpper(U16TEXT('A')));
		always_check(FU16Char::IsDigit(U16TEXT('0')));
		always_check(FU16Char::IsCntrl(U16TEXT('\n')));
		always_check(FU16Char::IsGraph(U16TEXT('!')));
		always_check(FU16Char::IsSpace(U16TEXT('\t')));
		always_check(FU16Char::IsBlank(U16TEXT(' ')));
		always_check(FU16Char::IsPrint(U16TEXT('#')));
		always_check(FU16Char::IsPunct(U16TEXT('[')));
	}

	{
		always_check(FU32Char::IsAlnum(U32TEXT('0')));
		always_check(FU32Char::IsAlpha(U32TEXT('A')));
		always_check(FU32Char::IsLower(U32TEXT('a')));
		always_check(FU32Char::IsUpper(U32TEXT('A')));
		always_check(FU32Char::IsDigit(U32TEXT('0')));
		always_check(FU32Char::IsCntrl(U32TEXT('\n')));
		always_check(FU32Char::IsGraph(U32TEXT('!')));
		always_check(FU32Char::IsSpace(U32TEXT('\t')));
		always_check(FU32Char::IsBlank(U32TEXT(' ')));
		always_check(FU32Char::IsPrint(U32TEXT('#')));
		always_check(FU32Char::IsPunct(U32TEXT('[')));
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
		always_check(!FChar::IsBlank(TEXT('+')));
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
		always_check(!FWChar::IsBlank(WTEXT('+')));
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
		always_check(!FU8Char::IsBlank(U8TEXT('+')));
		always_check(!FU8Char::IsPrint(U8TEXT('\n')));
		always_check(!FU8Char::IsPunct(U8TEXT('H')));
	}

	{
		always_check(!FU16Char::IsAlnum(U16TEXT('$')));
		always_check(!FU16Char::IsAlpha(U16TEXT('0')));
		always_check(!FU16Char::IsLower(U16TEXT('A')));
		always_check(!FU16Char::IsUpper(U16TEXT('a')));
		always_check(!FU16Char::IsDigit(U16TEXT('I')));
		always_check(!FU16Char::IsCntrl(U16TEXT('_')));
		always_check(!FU16Char::IsGraph(U16TEXT(' ')));
		always_check(!FU16Char::IsSpace(U16TEXT('=')));
		always_check(!FU16Char::IsBlank(U16TEXT('+')));
		always_check(!FU16Char::IsPrint(U16TEXT('\n')));
		always_check(!FU16Char::IsPunct(U16TEXT('H')));
	}

	{
		always_check(!FU32Char::IsAlnum(U32TEXT('$')));
		always_check(!FU32Char::IsAlpha(U32TEXT('0')));
		always_check(!FU32Char::IsLower(U32TEXT('A')));
		always_check(!FU32Char::IsUpper(U32TEXT('a')));
		always_check(!FU32Char::IsDigit(U32TEXT('I')));
		always_check(!FU32Char::IsCntrl(U32TEXT('_')));
		always_check(!FU32Char::IsGraph(U32TEXT(' ')));
		always_check(!FU32Char::IsSpace(U32TEXT('=')));
		always_check(!FU32Char::IsBlank(U32TEXT('+')));
		always_check(!FU32Char::IsPrint(U32TEXT('\n')));
		always_check(!FU32Char::IsPunct(U32TEXT('H')));
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
		always_check(FU16Char::ToLower(U16TEXT('i')) == U16TEXT('i'));
		always_check(FU16Char::ToUpper(U16TEXT('l')) == U16TEXT('L'));
		always_check(FU32Char::ToLower(U32TEXT('i')) == U32TEXT('i'));
		always_check(FU32Char::ToUpper(U32TEXT('l')) == U32TEXT('L'));
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

void TestStringView()
{
	auto Test = []<typename T>(TInPlaceType<T>)
	{
		{
			TStringView<T> Empty;

			always_check(Empty == LITERAL(T, ""));

			TStringView ViewI = LITERAL(T, "#Hello, World! Goodbye, World!#");

			ViewI.RemovePrefix(1);
			ViewI.RemoveSuffix(1);

			T Buffer[64];

			Memory::Memzero(Buffer);

			ViewI.Copy(Buffer);

			TStringView ViewII = Buffer;

			always_check(ViewI  == LITERAL(T, "Hello, World! Goodbye, World!"));
			always_check(ViewII == LITERAL(T, "Hello, World! Goodbye, World!"));

			TStringView<T> ViewA(ViewI.Begin(), 13);
			TStringView<T> ViewB(ViewI.Begin(), ViewI.End());
			TStringView<T> ViewC(&Buffer[0], 13);
			TStringView<T> ViewD(&Buffer[0]);

			always_check(ViewA == LITERAL(T, "Hello, World!"));
			always_check(ViewB == LITERAL(T, "Hello, World! Goodbye, World!"));
			always_check(ViewC == LITERAL(T, "Hello, World!"));
			always_check(ViewD == LITERAL(T, "Hello, World! Goodbye, World!"));
		}

		{
			TStringView View = LITERAL(T, "Hello, World! Goodbye, World!");

			always_check( View.StartsWith(LITERAL(T, "Hello, World!")));
			always_check(!View.StartsWith(LITERAL(T, "Goodbye, World!")));
			always_check( View.StartsWith(LITERAL(T, 'H')));
			always_check(!View.StartsWith(LITERAL(T, 'G')));
			always_check(!View.EndsWith(LITERAL(T, "Hello, World!")));
			always_check( View.EndsWith(LITERAL(T, "Goodbye, World!")));
			always_check( View.EndsWith(LITERAL(T, '!')));
			always_check(!View.EndsWith(LITERAL(T, '?')));
			always_check( View.Contains(LITERAL(T, "Hello, World!")));
			always_check( View.Contains(LITERAL(T, "Goodbye, World!")));
			always_check( View.Contains(LITERAL(T, '!')));
			always_check(!View.Contains(LITERAL(T, '?')));
		}

		{
			TStringView View = LITERAL(T, "Hello, World! Goodbye, World!");

			always_check(View.Find(LITERAL(T, ""))       ==  0);
			always_check(View.Find(LITERAL(T, "World"))  ==  7);
			always_check(View.Find(LITERAL(T, 'l'))      ==  2);
			always_check(View.RFind(LITERAL(T, ""))      == 29);
			always_check(View.RFind(LITERAL(T, "World")) == 23);
			always_check(View.RFind(LITERAL(T, 'l'))     == 26);

			always_check(View.Find(LITERAL(T, ""), 13)       == 13);
			always_check(View.Find(LITERAL(T, "World"), 13)  == 23);
			always_check(View.Find(LITERAL(T, 'l'), 13)      == 26);
			always_check(View.RFind(LITERAL(T, ""), 13)      == 13);
			always_check(View.RFind(LITERAL(T, "World"), 13) ==  7);
			always_check(View.RFind(LITERAL(T, 'l'), 13)     == 10);

			always_check(View.FindFirstOf(LITERAL(T, "eor")) ==  1);
			always_check(View.FindFirstOf(LITERAL(T, 'l'))   ==  2);
			always_check(View.FindLastOf(LITERAL(T, "eor"))  == 25);
			always_check(View.FindLastOf(LITERAL(T, 'l'))    == 26);

			always_check(View.FindFirstNotOf(LITERAL(T, "Hello! Goodbye!")) ==  5);
			always_check(View.FindFirstNotOf(LITERAL(T, '!'))               ==  0);
			always_check(View.FindLastNotOf(LITERAL(T, "Hello! Goodbye!"))  == 25);
			always_check(View.FindLastNotOf(LITERAL(T, '!'))                == 27);
		}
	};

	Test(InPlaceType<char>);
	Test(InPlaceType<wchar>);
	Test(InPlaceType<u8char>);
	Test(InPlaceType<u16char>);
	Test(InPlaceType<u32char>);
	Test(InPlaceType<unicodechar>);
}

void TestTemplateString()
{
	auto Test = []<typename T>(TInPlaceType<T>)
	{
		{
			TString<T> Empty;

			always_check(Empty.IsEmpty());
			always_check(TStringView(Empty.ToCString()) == LITERAL(T, ""));

			TString<T> StrA(32, LITERAL(T, 'A'));
			TString<T> StrB(LITERAL(T, "ABCDEFG"), 3);
			TString<T> StrC(LITERAL(T, "ABCDEFG"));
			TString<T> StrD(TStringView(LITERAL(T, "ABCDEFG")));
			TString<T> StrE({ LITERAL(T, 'A'), LITERAL(T, 'B'), LITERAL(T, 'C') });

			always_check(TStringView(StrA.ToCString()) == LITERAL(T, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
			always_check(TStringView(StrB.ToCString()) == LITERAL(T, "ABC"));
			always_check(TStringView(StrC.ToCString()) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrD.ToCString()) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrE.ToCString()) == LITERAL(T, "ABC"));

			TString<T> StrI(StrC);
			TString<T> StrII(MoveTemp(StrC));

			TString<T> StrIII = Empty;
			TString<T> StrIV  = Empty;

			StrIII = StrD;
			StrIV  = MoveTemp(StrD);

			always_check(TStringView(StrI  .ToCString()) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrII .ToCString()) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrIII.ToCString()) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrIV .ToCString()) == LITERAL(T, "ABCDEFG"));

			always_check(TStringView(StrC.ToCString()) == LITERAL(T, ""));
			always_check(TStringView(StrD.ToCString()) == LITERAL(T, ""));

			StrA.Reset();

			always_check(StrA.IsEmpty());
			always_check(TStringView(StrA.ToCString()) == LITERAL(T, ""));
		}

		{
			TString<T> Str = LITERAL(T, "A");

			always_check(!Str.IsEmpty());
			always_check(Str.Num() == 1);

			always_check(Str == TString<T>(LITERAL(T, "A")));
			always_check(Str ==            LITERAL(T, 'A') );
			always_check(Str ==            LITERAL(T, 'A') );
			always_check(TString<T>(LITERAL(T, "A")) == Str);
			always_check(           LITERAL(T, 'A')  == Str);
			always_check(           LITERAL(T, "A")  == Str);

			always_check(Str != TString<T>(LITERAL(T, "B")));
			always_check(Str !=            LITERAL(T, 'B') );
			always_check(Str !=            LITERAL(T, "B") );
			always_check(TString<T>(LITERAL(T, "B")) != Str);
			always_check(           LITERAL(T, 'B')  != Str);
			always_check(           LITERAL(T, "B")  != Str);

			always_check(Str < TString<T>(LITERAL(T, "B")));
			always_check(Str <            LITERAL(T, 'B') );
			always_check(Str <            LITERAL(T, "B") );
			always_check(TString<T>(LITERAL(T, "B")) > Str);
			always_check(           LITERAL(T, 'B')  > Str);
			always_check(           LITERAL(T, "B")  > Str);
		}

		{
			TString<T> Str = LITERAL(T, "##");

			Str.Insert(1, LITERAL(T, 'A'));

			always_check(Str == LITERAL(T, "#A#"));

			Str.Insert(2, LITERAL(T, "BCD"));

			always_check(Str == LITERAL(T, "#ABCD#"));

			Str.Insert(3, 3, LITERAL(T, '*'));

			always_check(Str == LITERAL(T, "#AB***CD#"));

			Str.Erase(4);

			always_check(Str == LITERAL(T, "#AB**CD#"));
		}

		{
			TString<T> Str = LITERAL(T, "A");

			Str.PushBack(LITERAL(T, 'B'));

			always_check(Str == LITERAL(T, "AB"));

			Str.PopBack();

			always_check(Str == LITERAL(T, "A"));

			Str.Append(2, LITERAL(T, 'B'));

			always_check(Str == LITERAL(T, "ABB"));

			Str.Append(LITERAL(T, "CD"));

			always_check(Str == LITERAL(T, "ABBCD"));

			Str.Append({ LITERAL(T, 'E'), LITERAL(T, 'F') });

			always_check(Str == LITERAL(T, "ABBCDEF"));

			Str = LITERAL(T, "A");

			Str += LITERAL(T, 'B');

			always_check(Str == LITERAL(T, "AB"));

			Str += LITERAL(T, "CD");

			always_check(Str == LITERAL(T, "ABCD"));

			Str += { LITERAL(T, 'E'), LITERAL(T, 'F') };

			always_check(Str == LITERAL(T, "ABCDEF"));
		}

		{
			TString StrA = LITERAL(T, "A");
			TString StrB = LITERAL(T, "B");

			always_check(StrA + StrB              == LITERAL(T, "AB"));
			always_check(StrA + LITERAL(T, 'B')   == LITERAL(T, "AB"));
			always_check(StrA + LITERAL(T, "BCD") == LITERAL(T, "ABCD"));
			always_check(LITERAL(T, 'B')   + StrB == LITERAL(T, "BB"));
			always_check(LITERAL(T, "BCD") + StrB == LITERAL(T, "BCDB"));

			StrA = LITERAL(T, "A"); StrB = LITERAL(T, "B");
			always_check(MoveTemp(StrA) + MoveTemp(StrB)    == LITERAL(T, "AB"));
			StrA = LITERAL(T, "A"); StrB = LITERAL(T, "B");
			always_check(MoveTemp(StrA) + LITERAL(T, 'B')   == LITERAL(T, "AB"));
			StrA = LITERAL(T, "A"); StrB = LITERAL(T, "B");
			always_check(MoveTemp(StrA) + LITERAL(T, "BCD") == LITERAL(T, "ABCD"));
			StrA = LITERAL(T, "A"); StrB = LITERAL(T, "B");
			always_check(LITERAL(T, 'B')   + MoveTemp(StrB) == LITERAL(T, "BB"));
			StrA = LITERAL(T, "A"); StrB = LITERAL(T, "B");
			always_check(LITERAL(T, "BCD") + MoveTemp(StrB) == LITERAL(T, "BCDB"));
		}

		{
			TString Str = LITERAL(T, "Hello, World! Goodbye, World!");

			always_check( Str.StartsWith(LITERAL(T, "Hello, World!")));
			always_check(!Str.StartsWith(LITERAL(T, "Goodbye, World!")));
			always_check( Str.StartsWith(LITERAL(T, 'H')));
			always_check(!Str.StartsWith(LITERAL(T, 'G')));
			always_check(!Str.EndsWith(LITERAL(T, "Hello, World!")));
			always_check( Str.EndsWith(LITERAL(T, "Goodbye, World!")));
			always_check( Str.EndsWith(LITERAL(T, '!')));
			always_check(!Str.EndsWith(LITERAL(T, '?')));
			always_check( Str.Contains(LITERAL(T, "Hello, World!")));
			always_check( Str.Contains(LITERAL(T, "Goodbye, World!")));
			always_check( Str.Contains(LITERAL(T, '!')));
			always_check(!Str.Contains(LITERAL(T, '?')));
		}

		{
			TString Str = LITERAL(T, "#AB**CD#");

			always_check(Str.Replace(3, 2, 3, LITERAL(T, '^')) == LITERAL(T, "#AB^^^CD#"));

			always_check(Str.Replace(3, 3, LITERAL(T, "123")) == LITERAL(T, "#AB123CD#"));

			always_check(Str.Substr(3, 3) == LITERAL(T, "123"));

			always_check(Str.Substr(3) == LITERAL(T, "123CD#"));
		}

		{
			TString Str = LITERAL(T, "Hello, World! Goodbye, World!");

			always_check(Str.Find(LITERAL(T, ""))       ==  0);
			always_check(Str.Find(LITERAL(T, "World"))  ==  7);
			always_check(Str.Find(LITERAL(T, 'l'))      ==  2);
			always_check(Str.RFind(LITERAL(T, ""))      == 29);
			always_check(Str.RFind(LITERAL(T, "World")) == 23);
			always_check(Str.RFind(LITERAL(T, 'l'))     == 26);

			always_check(Str.Find(LITERAL(T, ""), 13)       == 13);
			always_check(Str.Find(LITERAL(T, "World"), 13)  == 23);
			always_check(Str.Find(LITERAL(T, 'l'), 13)      == 26);
			always_check(Str.RFind(LITERAL(T, ""), 13)      == 13);
			always_check(Str.RFind(LITERAL(T, "World"), 13) ==  7);
			always_check(Str.RFind(LITERAL(T, 'l'), 13)     == 10);

			always_check(Str.FindFirstOf(LITERAL(T, "eor")) ==  1);
			always_check(Str.FindFirstOf(LITERAL(T, 'l'))   ==  2);
			always_check(Str.FindLastOf(LITERAL(T, "eor"))  == 25);
			always_check(Str.FindLastOf(LITERAL(T, 'l'))    == 26);

			always_check(Str.FindFirstNotOf(LITERAL(T, "Hello! Goodbye!")) ==  5);
			always_check(Str.FindFirstNotOf(LITERAL(T, '!'))               ==  0);
			always_check(Str.FindLastNotOf(LITERAL(T, "Hello! Goodbye!"))  == 25);
			always_check(Str.FindLastNotOf(LITERAL(T, '!'))                == 27);
		}
	};

	Test(InPlaceType<char>);
	Test(InPlaceType<wchar>);
	Test(InPlaceType<u8char>);
	Test(InPlaceType<u16char>);
	Test(InPlaceType<u32char>);
	Test(InPlaceType<unicodechar>);
}

NAMESPACE_END(Testing)

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
