#include "Testing/StringTesting.h"

#include "String/Char.h"
#include "Memory/Memory.h"
#include "String/String.h"
#include "Numeric/Numeric.h"
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
	TestStringConversion();
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
	auto Test = []<typename T>(TInPlaceType<T>)
	{
		always_check(TChar<T>::IsASCII(LITERAL(T, '0')));
		always_check(TChar<T>::IsASCII(LITERAL(T, 'A')));
		always_check(TChar<T>::IsASCII(LITERAL(T, 'a')));
		always_check(TChar<T>::IsASCII(LITERAL(T, 'A')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '0')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '\n')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '!')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '\t')));
		always_check(TChar<T>::IsASCII(LITERAL(T, ' ')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '#')));
		always_check(TChar<T>::IsASCII(LITERAL(T, '[')));

		always_check(TChar<T>::IsAlnum(LITERAL(T, '0')));
		always_check(TChar<T>::IsAlpha(LITERAL(T, 'A')));
		always_check(TChar<T>::IsLower(LITERAL(T, 'a')));
		always_check(TChar<T>::IsUpper(LITERAL(T, 'A')));
		always_check(TChar<T>::IsDigit(LITERAL(T, '0')));
		always_check(TChar<T>::IsCntrl(LITERAL(T, '\n')));
		always_check(TChar<T>::IsGraph(LITERAL(T, '!')));
		always_check(TChar<T>::IsSpace(LITERAL(T, '\t')));
		always_check(TChar<T>::IsBlank(LITERAL(T, ' ')));
		always_check(TChar<T>::IsPrint(LITERAL(T, '#')));
		always_check(TChar<T>::IsPunct(LITERAL(T, '[')));

		always_check(!TChar<T>::IsAlnum(LITERAL(T, '$')));
		always_check(!TChar<T>::IsAlpha(LITERAL(T, '0')));
		always_check(!TChar<T>::IsLower(LITERAL(T, 'A')));
		always_check(!TChar<T>::IsUpper(LITERAL(T, 'a')));
		always_check(!TChar<T>::IsDigit(LITERAL(T, 'I')));
		always_check(!TChar<T>::IsCntrl(LITERAL(T, '_')));
		always_check(!TChar<T>::IsGraph(LITERAL(T, ' ')));
		always_check(!TChar<T>::IsSpace(LITERAL(T, '=')));
		always_check(!TChar<T>::IsBlank(LITERAL(T, '+')));
		always_check(!TChar<T>::IsPrint(LITERAL(T, '\n')));
		always_check(!TChar<T>::IsPunct(LITERAL(T, 'H')));

		always_check( TChar<T>::IsDigit(LITERAL(T, 'F'), 16));
		always_check(!TChar<T>::IsDigit(LITERAL(T, 'G'), 16));

		always_check(TChar<T>::ToLower(LITERAL(T, 'i')) == LITERAL(T, 'i'));
		always_check(TChar<T>::ToUpper(LITERAL(T, 'l')) == LITERAL(T, 'L'));

		always_check(0x0 == TChar<T>::ToDigit(LITERAL(T, '0')));
		always_check(0xF == TChar<T>::ToDigit(LITERAL(T, 'f')));
		always_check(0xF == TChar<T>::ToDigit(LITERAL(T, 'F')));

		always_check(0x0 == TChar<T>::ToDigit(LITERAL(T, '0'), false));
		always_check(0xF != TChar<T>::ToDigit(LITERAL(T, 'f'), false));
		always_check(0xF == TChar<T>::ToDigit(LITERAL(T, 'F'), false));

		always_check(0x0 == TChar<T>::ToDigit(LITERAL(T, '0'), true));
		always_check(0xF == TChar<T>::ToDigit(LITERAL(T, 'f'), true));
		always_check(0xF != TChar<T>::ToDigit(LITERAL(T, 'F'), true));

		always_check(LITERAL(T, '0') == TChar<T>::FromDigit(0x0));
		always_check(LITERAL(T, 'f') != TChar<T>::FromDigit(0xF));
		always_check(LITERAL(T, 'F') == TChar<T>::FromDigit(0xF));

		always_check(LITERAL(T, '0') == TChar<T>::FromDigit(0x0, false));
		always_check(LITERAL(T, 'f') != TChar<T>::FromDigit(0xF, false));
		always_check(LITERAL(T, 'F') == TChar<T>::FromDigit(0xF, false));

		always_check(LITERAL(T, '0') == TChar<T>::FromDigit(0x0, true));
		always_check(LITERAL(T, 'f') == TChar<T>::FromDigit(0xF, true));
		always_check(LITERAL(T, 'F') != TChar<T>::FromDigit(0xF, true));
	};

	Test(InPlaceType<char>);
	Test(InPlaceType<wchar>);
	Test(InPlaceType<u8char>);
	Test(InPlaceType<u16char>);
	Test(InPlaceType<u32char>);
	Test(InPlaceType<unicodechar>);
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

			TStringView ViewA(ViewI.Begin(), 13);
			TStringView ViewB(ViewI.Begin(), ViewI.End());
			TStringView ViewC(&Buffer[0], 13);
			TStringView ViewD(&Buffer[0]);

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

		{
			always_check(LITERAL_VIEW(T, "   ABC   ").TrimStart()       == LITERAL(T,    "ABC   "));
			always_check(LITERAL_VIEW(T, "   ABC   ").TrimEnd()         == LITERAL(T, "   ABC"   ));
			always_check(LITERAL_VIEW(T, "   ABC   ").TrimStartAndEnd() == LITERAL(T,    "ABC"   ));

			always_check(LITERAL_VIEW(T, "   A\0C   ").TrimToNullTerminator() == LITERAL(T, "   A"));
		}

		{
			always_check( LITERAL_VIEW(T, "012345678900").IsASCII());
			always_check(!LITERAL_VIEW(T, "\u4E38\u8FA3").IsASCII());
			always_check( LITERAL_VIEW(T, "012345678900").IsInteger());
			always_check(!LITERAL_VIEW(T, "\u4E38\u8FA3").IsInteger());
			always_check(!LITERAL_VIEW(T, "0123456789AB").IsInteger());
			always_check( LITERAL_VIEW(T, "0123456789AB").IsInteger(16));
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
			always_check(TStringView<T>(*Empty) == LITERAL(T, ""));

			TString<T> StrA(32, LITERAL(T, 'A'));

			TString StrB(LITERAL(T, "ABCDEFG"), 3);
			TString StrC(LITERAL(T, "ABCDEFG"));
			TString StrD(TStringView(LITERAL(T, "ABCDEFG")));
			TString StrE({ LITERAL(T, 'A'), LITERAL(T, 'B'), LITERAL(T, 'C') });

			always_check(TStringView<T>(*StrA) == LITERAL(T, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
			always_check(TStringView<T>(*StrB) == LITERAL(T, "ABC"));
			always_check(TStringView<T>(*StrC) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView<T>(*StrD) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView<T>(*StrE) == LITERAL(T, "ABC"));

			TString StrI(StrC);
			TString StrII(MoveTemp(StrC));

			TString StrIII = Empty;
			TString StrIV  = Empty;

			StrIII = StrD;
			StrIV  = MoveTemp(StrD);

			always_check(TStringView(StrI  ) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrII ) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrIII) == LITERAL(T, "ABCDEFG"));
			always_check(TStringView(StrIV ) == LITERAL(T, "ABCDEFG"));

			always_check(StrC == LITERAL(T, ""));
			always_check(StrD == LITERAL(T, ""));

			StrA.Reset();

			always_check(StrA.IsEmpty());
			always_check(StrA == LITERAL(T, ""));
		}

		{
			TString Str = LITERAL(T, "A");

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
			TString Str = LITERAL(T, "##");

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
			TString Str = LITERAL(T, "A");

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

		{
			always_check(TString(LITERAL(T, "   ABC   ")).TrimStart()       == LITERAL(T,    "ABC   "));
			always_check(TString(LITERAL(T, "   ABC   ")).TrimEnd()         == LITERAL(T, "   ABC"   ));
			always_check(TString(LITERAL(T, "   ABC   ")).TrimStartAndEnd() == LITERAL(T,    "ABC"   ));

			always_check(TString(LITERAL(T, "   A\0C   ")).TrimToNullTerminator() == LITERAL(T, "   A"));
		}

		{
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToString()        ==        TEXT("\u4E38\u8FA3"));
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToWString()       ==       WTEXT("\u4E38\u8FA3"));
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToU8String()      ==      U8TEXT("\u4E38\u8FA3"));
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToU16String()     ==     U16TEXT("\u4E38\u8FA3"));
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToU32String()     ==     U32TEXT("\u4E38\u8FA3"));
			always_check(TString(LITERAL(T, "\u4E38\u8FA3")).ToUnicodeString() == UNICODETEXT("\u4E38\u8FA3"));
		}
	};

	Test(InPlaceType<char>);
	Test(InPlaceType<wchar>);
	Test(InPlaceType<u8char>);
	Test(InPlaceType<u16char>);
	Test(InPlaceType<u32char>);
	Test(InPlaceType<unicodechar>);
}

void TestStringConversion()
{
	auto Test = []<typename T>(TInPlaceType<T>)
	{

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), true ) == LITERAL(T, "#True#" ));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), false) == LITERAL(T, "#False#"));

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), +0) == LITERAL(T, "#0#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"),  0) == LITERAL(T, "#0#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), -0) == LITERAL(T, "#0#"));

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), 42) == LITERAL(T, "#42#"));

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), +0.0) == LITERAL(T,  "#0.000000#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"),  0.0) == LITERAL(T,  "#0.000000#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), -0.0) == LITERAL(T, "#-0.000000#"));

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), 3.14) == LITERAL(T, "#3.140000#"));

		always_check(TString<T>::Format(LITERAL(T, "#{}#"), +TNumericLimits<float>::Infinity()) == LITERAL(T,  "#Infinity#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), -TNumericLimits<float>::Infinity()) == LITERAL(T, "#-Infinity#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), +TNumericLimits<float>::QuietNaN()) == LITERAL(T,  "#NaN#"));
		always_check(TString<T>::Format(LITERAL(T, "#{}#"), -TNumericLimits<float>::QuietNaN()) == LITERAL(T, "#-NaN#"));

		auto CheckParseArithmetic = []<typename U>(TStringView<T> View, U Result)
		{
			U Object;

			if      constexpr (CSameAs<U, bool>)  always_check(View.Parse(LITERAL(T, "{0:}"),    Object) == 1);
			else if constexpr (CIntegral<U>)      always_check(View.Parse(LITERAL(T, "{0:+#I}"), Object) == 1);
			else if constexpr (CFloatingPoint<U>) always_check(View.Parse(LITERAL(T, "{0:+#G}"), Object) == 1);

			if constexpr (CFloatingPoint<U>)
			{
				always_check(Math::IsInfinity(Result) == Math::IsInfinity(Object));
				always_check(Math::IsNaN(Result)      == Math::IsNaN(Object));

				always_check(Math::IsNegative(Result) == Math::IsNegative(Object));

				if (Math::IsInfinity(Result) || Math::IsNaN(Result)) return;

				always_check(Math::IsNearlyEqual(Object, Result, 1e-4));
			}
			else always_check(Object == Result);
		};

		CheckParseArithmetic(LITERAL(T, "true" ), true );
		CheckParseArithmetic(LITERAL(T, "false"), false);

		auto CheckParseInt = [&]<typename U>(TInPlaceType<U>)
		{
			CheckParseArithmetic(LITERAL(T, "+0"), static_cast<U>(+0.0));
			CheckParseArithmetic(LITERAL(T, " 0"), static_cast<U>( 0.0));
			CheckParseArithmetic(LITERAL(T, "-0"), static_cast<U>(-0.0));

			CheckParseArithmetic(LITERAL(T,       "+42"), static_cast<U>(      +42));
			CheckParseArithmetic(LITERAL(T,      "+052"), static_cast<U>(     +052));
			CheckParseArithmetic(LITERAL(T,     "+0x2A"), static_cast<U>(    +0x2A));
			CheckParseArithmetic(LITERAL(T, "+0b101010"), static_cast<U>(+0b101010));

			CheckParseArithmetic(LITERAL(T,       "42"), static_cast<U>(      42));
			CheckParseArithmetic(LITERAL(T,      "052"), static_cast<U>(     052));
			CheckParseArithmetic(LITERAL(T,     "0x2A"), static_cast<U>(    0x2A));
			CheckParseArithmetic(LITERAL(T, "0b101010"), static_cast<U>(0b101010));

			CheckParseArithmetic(LITERAL(T,       "-42"), static_cast<U>(      -42));
			CheckParseArithmetic(LITERAL(T,      "-052"), static_cast<U>(     -052));
			CheckParseArithmetic(LITERAL(T,     "-0x2A"), static_cast<U>(    -0x2A));
			CheckParseArithmetic(LITERAL(T, "-0b101010"), static_cast<U>(-0b101010));
		};

		CheckParseInt(InPlaceType<int8>);
		CheckParseInt(InPlaceType<int16>);
		CheckParseInt(InPlaceType<int32>);
		CheckParseInt(InPlaceType<int64>);

		auto CheckParseFloat = [&]<typename U>(TInPlaceType<U>)
		{
			CheckParseArithmetic(LITERAL(T,         "+3.14"), static_cast<U>(        +3.14));
			CheckParseArithmetic(LITERAL(T,       "+3.14e2"), static_cast<U>(      +3.14e2));
			CheckParseArithmetic(LITERAL(T,      "+3.14e-2"), static_cast<U>(     +3.14e-2));
			CheckParseArithmetic(LITERAL(T, "+0x1.91eb86p1"), static_cast<U>(+0x1.91eb86p1));

			CheckParseArithmetic(LITERAL(T,         "3.14"), static_cast<U>(        3.14));
			CheckParseArithmetic(LITERAL(T,       "3.14e2"), static_cast<U>(      3.14e2));
			CheckParseArithmetic(LITERAL(T,      "3.14e-2"), static_cast<U>(     3.14e-2));
			CheckParseArithmetic(LITERAL(T, "0x1.91eb86p1"), static_cast<U>(0x1.91eb86p1));

			CheckParseArithmetic(LITERAL(T,         "-3.14"), static_cast<U>(        -3.14));
			CheckParseArithmetic(LITERAL(T,       "-3.14e2"), static_cast<U>(      -3.14e2));
			CheckParseArithmetic(LITERAL(T,      "-3.14e-2"), static_cast<U>(     -3.14e-2));
			CheckParseArithmetic(LITERAL(T, "-0x1.91eb86p1"), static_cast<U>(-0x1.91eb86p1));

			CheckParseArithmetic(LITERAL(T, "+Infinity"), +TNumericLimits<U>::Infinity());
			CheckParseArithmetic(LITERAL(T, " Infinity"), +TNumericLimits<U>::Infinity());
			CheckParseArithmetic(LITERAL(T, "-Infinity"), -TNumericLimits<U>::Infinity());

			CheckParseArithmetic(LITERAL(T, "+NaN"), +TNumericLimits<U>::QuietNaN());
			CheckParseArithmetic(LITERAL(T, " NaN"), +TNumericLimits<U>::QuietNaN());
			CheckParseArithmetic(LITERAL(T, "-NaN"), -TNumericLimits<U>::QuietNaN());
		};

		CheckParseFloat(InPlaceType<float>);
		CheckParseFloat(InPlaceType<double>);

		{
			always_check(TString<T>::FromBool(true ) == LITERAL(T, "True" ));
			always_check(TString<T>::FromBool(false) == LITERAL(T, "False"));
		}

		{
			always_check(TString<T>::FromInt(42)         == LITERAL(T, "42"        ));
			always_check(TString<T>::FromInt(255, 16)    == LITERAL(T, "FF"        ));
			always_check(TString<T>::FromInt(-42)        == LITERAL(T, "-42"       ));
			always_check(TString<T>::FromInt(0)          == LITERAL(T, "0"         ));
			always_check(TString<T>::FromInt(1234567890) == LITERAL(T, "1234567890"));
			always_check(TString<T>::FromInt(255, 2)     == LITERAL(T, "11111111"  ));
			always_check(TString<T>::FromInt(255, 8)     == LITERAL(T, "377"       ));
			always_check(TString<T>::FromInt(255, 36)    == LITERAL(T, "73"        ));
		}

		{
			always_check(TString<T>::FromFloat(3.14f)                       == LITERAL(T, "3.14"    ));
			always_check(TString<T>::FromFloat(0.0f)                        == LITERAL(T, "0"       ));
			always_check(TString<T>::FromFloat(-3.14f)                      == LITERAL(T, "-3.14"   ));
			always_check(TString<T>::FromFloat(3.14f, true, false)          == LITERAL(T, "3.14"    ));
			always_check(TString<T>::FromFloat(3.14f, false, true)          == LITERAL(T, "3.14e+00"));
			always_check(TString<T>::FromFloat(3.14f, false, false, 2)      == LITERAL(T, "1.92p+1" ));
			always_check(TString<T>::FromFloat(1.0f / 3.0f, true, false, 5) == LITERAL(T, "0.33333" ));
		}

		{
			always_check( LITERAL_VIEW(T, "True"  ).ToBool());
			always_check(!LITERAL_VIEW(T, "False" ).ToBool());
			always_check( LITERAL_VIEW(T, "1"     ).ToBool());
			always_check(!LITERAL_VIEW(T, "0"     ).ToBool());
			always_check(!LITERAL_VIEW(T, "random").ToBool());
		}

		{
			always_check(LITERAL_VIEW(T, "42"     ).ToInt()   == 42 );
			always_check(LITERAL_VIEW(T, "FF"     ).ToInt(16) == 255);
			always_check(LITERAL_VIEW(T, "-42"    ).ToInt()   == -42);
			always_check(LITERAL_VIEW(T, "0"      ).ToInt()   == 0  );
			always_check(LITERAL_VIEW(T, "Invalid").ToInt()   == 0  );

			always_check(LITERAL_VIEW(T,  "999999999999999999999999999999").ToInt() == 0);
			always_check(LITERAL_VIEW(T, "-999999999999999999999999999999").ToInt() == 0);
		}

		{
			always_check(LITERAL_VIEW(T, "3.14"    ).ToFloat() ==  3.14f);
			always_check(LITERAL_VIEW(T, "3.14e+00").ToFloat() ==  3.14f);
			always_check(LITERAL_VIEW(T, "-3.14"   ).ToFloat() == -3.14f);
			always_check(LITERAL_VIEW(T, "0.0"     ).ToFloat() ==   0.0f);

			always_check(Math::IsNaN(LITERAL_VIEW(T,  "1e+308").ToFloat()));
			always_check(Math::IsNaN(LITERAL_VIEW(T, "-1e+308").ToFloat()));
			always_check(Math::IsNaN(LITERAL_VIEW(T,  "1e-308").ToFloat()));
			always_check(Math::IsNaN(LITERAL_VIEW(T, "-1e-308").ToFloat()));
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
