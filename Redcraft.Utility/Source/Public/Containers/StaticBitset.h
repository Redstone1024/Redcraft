#pragma once

#include "CoreTypes.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Container.h"
#include "Containers/Iterator.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

NAMESPACE_PRIVATE_BEGIN

template <size_t N>
using TDefaultBitsetBlockType =
	TConditional<(N <=  8), uint8,
	TConditional<(N <= 16), uint16,
	TConditional<(N <= 32), uint32, uint64>>>;

NAMESPACE_PRIVATE_END

#if 1

template <size_t N, CUnsignedIntegral InBlockType = NAMESPACE_PRIVATE::TDefaultBitsetBlockType<N>> requires (!CSameAs<InBlockType, bool>)
class TStaticBitset
{
private:

	template <bool bConst>
	class TIteratorImpl;

public:

	using BlockType     = InBlockType;
	using ElementType   = bool;

	class      Reference;
	using ConstReference = bool;

	using      Iterator = TIteratorImpl<false>;
	using ConstIterator = TIteratorImpl<true >;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CRandomAccessIterator<     Iterator>);
	static_assert(CRandomAccessIterator<ConstIterator>);

	static constexpr size_t BlockWidth = sizeof(BlockType) * 8;

	/** Default constructor. Constructs an empty bitset. */
	FORCEINLINE constexpr TStaticBitset() = default;

	/** Constructs a bitset from an integer. */
	constexpr TStaticBitset(uint64 InValue)
	{
		static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TStaticBitset is unexpected");

		if constexpr (sizeof(BlockType) == sizeof(uint8))
		{
			if constexpr (N >  0) Impl[0] = static_cast<BlockType>(InValue >>  0);
			if constexpr (N >  8) Impl[1] = static_cast<BlockType>(InValue >>  8);
			if constexpr (N > 16) Impl[2] = static_cast<BlockType>(InValue >> 16);
			if constexpr (N > 24) Impl[3] = static_cast<BlockType>(InValue >> 24);
			if constexpr (N > 32) Impl[4] = static_cast<BlockType>(InValue >> 32);
			if constexpr (N > 40) Impl[5] = static_cast<BlockType>(InValue >> 40);
			if constexpr (N > 48) Impl[6] = static_cast<BlockType>(InValue >> 48);
			if constexpr (N > 56) Impl[7] = static_cast<BlockType>(InValue >> 56);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint16))
		{
			if constexpr (N >  0) Impl[0] = static_cast<BlockType>(InValue >>  0);
			if constexpr (N > 16) Impl[1] = static_cast<BlockType>(InValue >> 16);
			if constexpr (N > 32) Impl[2] = static_cast<BlockType>(InValue >> 32);
			if constexpr (N > 48) Impl[3] = static_cast<BlockType>(InValue >> 48);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint32))
		{
			if constexpr (N >  0) Impl[0] = static_cast<BlockType>(InValue >>  0);
			if constexpr (N > 32) Impl[1] = static_cast<BlockType>(InValue >> 32);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint64))
		{
			if constexpr (N >  0) Impl[0] = static_cast<BlockType>(InValue >>  0);
		}
		else check_no_entry();

		constexpr size_t BlockInteger = sizeof(uint64) / sizeof(BlockType);

		if constexpr ((N + BlockWidth - 1) / BlockWidth <= BlockInteger) return;

		for (size_t Index = BlockInteger; Index != NumBlocks(); ++Index)
		{
			Impl[Index] = 0;
		}
	}

	/** Copy constructor. Constructs the bitset with the copy of the bits of 'InValue'. */
	FORCEINLINE constexpr TStaticBitset(const TStaticBitset&) = default;

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE constexpr TStaticBitset(TStaticBitset&&) = default;

	/** Destructs the bitset. The storage is deallocated. */
	FORCEINLINE constexpr ~TStaticBitset() = default;

	/** Copy assignment operator. Replaces the bits with a copy of the bits of 'InValue'. */
	FORCEINLINE constexpr TStaticBitset& operator=(const TStaticBitset&) = default;

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	FORCEINLINE constexpr TStaticBitset& operator=(TStaticBitset&&) = default;

	/** Compares the bits of two bitsets. */
	NODISCARD friend constexpr bool operator==(const TStaticBitset& LHS, const TStaticBitset& RHS)
	{
		if constexpr (N == 0) return true;

		for (size_t Index = 0; Index != LHS.NumBlocks() - 1; ++Index)
		{
			if (LHS.Impl[Index] != RHS.Impl[Index]) return false;
		}

		const BlockType LastBlockBitmask = LHS.Num() % BlockWidth != 0 ? (1ull << LHS.Num() % BlockWidth) - 1 : -1;

		return (LHS.Impl[LHS.NumBlocks() - 1] & LastBlockBitmask) == (RHS.Impl[LHS.NumBlocks() - 1] & LastBlockBitmask);
	}

	/** Sets the bits to the result of binary AND on corresponding pairs of bits of *this and other. */
	constexpr TStaticBitset& operator&=(const TStaticBitset& InValue)
	{
		if constexpr (N == 0) return *this;

		if (&InValue == this) UNLIKELY return *this;

		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl[Index] &= InValue.Impl[Index];
		}

		return *this;
	}

	/** Sets the bits to the result of binary OR on corresponding pairs of bits of *this and other. */
	constexpr TStaticBitset& operator|=(const TStaticBitset& InValue)
	{
		if constexpr (N == 0) return *this;

		if (&InValue == this) UNLIKELY return *this;

		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl[Index] |= InValue.Impl[Index];
		}

		return *this;
	}

	/** Sets the bits to the result of binary XOR on corresponding pairs of bits of *this and other. */
	constexpr TStaticBitset& operator^=(const TStaticBitset& InValue)
	{
		if constexpr (N == 0) return *this;

		if (&InValue == this) UNLIKELY return Set(false);

		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl[Index] ^= InValue.Impl[Index];
		}

		return *this;
	}

	NODISCARD friend FORCEINLINE constexpr TStaticBitset operator&(const TStaticBitset& LHS, const TStaticBitset& RHS) { return TStaticBitset(LHS) &= RHS; }
	NODISCARD friend FORCEINLINE constexpr TStaticBitset operator|(const TStaticBitset& LHS, const TStaticBitset& RHS) { return TStaticBitset(LHS) |= RHS; }
	NODISCARD friend FORCEINLINE constexpr TStaticBitset operator^(const TStaticBitset& LHS, const TStaticBitset& RHS) { return TStaticBitset(LHS) ^= RHS; }

	/** @return The temporary copy of *this with all bits flipped (binary NOT). */
	NODISCARD constexpr TStaticBitset operator~() const
	{
		TStaticBitset Result = *this;

		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Result.Impl[Index] = ~Result.Impl[Index];
		}

		return Result;
	}

	/** Performs binary shift left. */
	constexpr TStaticBitset& operator<<=(size_t Offset)
	{
		if constexpr (N == 0) return *this;

		const size_t Blockshift = Offset / BlockWidth;
		const size_t Bitshift   = Offset % BlockWidth;

		if (Blockshift != 0)
		{
			for (size_t Index = NumBlocks() - 1; Index != -1; --Index)
			{
				Impl[Index] = Index >= Blockshift ? Impl[Index - Blockshift] : 0;
			}
		}

		if (Bitshift != 0)
		{
			for (size_t Index = NumBlocks() - 1; Index != 0; --Index)
			{
				Impl[Index] = Impl[Index] << Bitshift | Impl[Index - 1] >> (BlockWidth - Bitshift);
			}

			Impl[0] <<= Bitshift;
		}

		return *this;
	}

	/** Performs binary shift right. */
	constexpr TStaticBitset& operator>>=(size_t Offset)
	{
		if constexpr (N == 0) return *this;

		const size_t Blockshift = Offset / BlockWidth;
		const size_t Bitshift   = Offset % BlockWidth;

		if constexpr (N % BlockWidth != 0)
		{
			Impl[NumBlocks() - 1] &= (1ull << Num() % BlockWidth) - 1;
		}

		if (Blockshift != 0)
		{
			for (size_t Index = 0; Index != NumBlocks(); ++Index)
			{
				Impl[Index] = Index < NumBlocks() - Blockshift ? Impl[Index + Blockshift] : 0;
			}
		}

		if (Bitshift != 0)
		{
			for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
			{
				Impl[Index] = Impl[Index] >> Bitshift | Impl[Index + 1] << (BlockWidth - Bitshift);
			}

			Impl[NumBlocks() - 1] >>= Bitshift;
		}

		return *this;
	}

	NODISCARD FORCEINLINE constexpr TStaticBitset operator<<(size_t Offset) const { return TStaticBitset(*this) <<= Offset; }
	NODISCARD FORCEINLINE constexpr TStaticBitset operator>>(size_t Offset) const { return TStaticBitset(*this) >>= Offset; }

	/** @return true if all bits are set to true, otherwise false. */
	NODISCARD constexpr bool All() const
	{
		if constexpr (N == 0) return true;

		for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
		{
			if (Impl[Index] != -1) return false;
		}

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		return (Impl[NumBlocks() - 1] | ~LastBlockBitmask) == -1;
	}

	/** @return true if any of the bits are set to true, otherwise false. */
	NODISCARD constexpr bool Any() const
	{
		if constexpr (N == 0) return false;

		for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
		{
			if (Impl[Index] != 0) return true;
		}

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		return (Impl[NumBlocks() - 1] & LastBlockBitmask) != 0;
	}

	/** @return true if none of the bits are set to true, otherwise false. */
	NODISCARD FORCEINLINE constexpr bool None() const { return !Any(); }

	/** @return The number of bits that are set to true. */
	NODISCARD constexpr size_t Count() const
	{
		if constexpr (N == 0) return 0;

		size_t Result = 0;

		constexpr auto BlockCount = [](BlockType Block) constexpr
		{
			static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TStaticBitset is unexpected");

			if constexpr (sizeof(BlockType) == sizeof(uint8))
			{
				Block = (Block & 0x55ull) + ((Block >> 1) & 0x55ull);
				Block = (Block & 0x33ull) + ((Block >> 2) & 0x33ull);
				Block = (Block & 0x0Full) + ((Block >> 4) & 0x0Full);
			}
			else if constexpr (sizeof(BlockType) == sizeof(uint16))
			{
				Block = (Block & 0x5555ull) + ((Block >>  1) & 0x5555ull);
				Block = (Block & 0x3333ull) + ((Block >>  2) & 0x3333ull);
				Block = (Block & 0x0F0Full) + ((Block >>  4) & 0x0F0Full);
				Block = (Block & 0x00FFull) + ((Block >>  8) & 0x00FFull);
			}
			else if constexpr (sizeof(BlockType) == sizeof(uint32))
			{
				Block = (Block & 0x55555555ull) + ((Block >>  1) & 0x55555555ull);
				Block = (Block & 0x33333333ull) + ((Block >>  2) & 0x33333333ull);
				Block = (Block & 0x0F0F0F0Full) + ((Block >>  4) & 0x0F0F0F0Full);
				Block = (Block & 0x00FF00FFull) + ((Block >>  8) & 0x00FF00FFull);
				Block = (Block & 0x0000FFFFull) + ((Block >> 16) & 0x0000FFFFull);
			}
			else if constexpr (sizeof(BlockType) == sizeof(uint64))
			{
				Block = (Block & 0x5555555555555555ull) + ((Block >>  1) & 0x5555555555555555ull);
				Block = (Block & 0x3333333333333333ull) + ((Block >>  2) & 0x3333333333333333ull);
				Block = (Block & 0x0F0F0F0F0F0F0F0Full) + ((Block >>  4) & 0x0F0F0F0F0F0F0F0Full);
				Block = (Block & 0x00FF00FF00FF00FFull) + ((Block >>  8) & 0x00FF00FF00FF00FFull);
				Block = (Block & 0x0000FFFF0000FFFFull) + ((Block >> 16) & 0x0000FFFF0000FFFFull);
				Block = (Block & 0x00000000FFFFFFFFull) + ((Block >> 32) & 0x00000000FFFFFFFFull);
			}
			else check_no_entry();

			return Block;
		};

		for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
		{
			Result += BlockCount(Impl[Index]);
		}

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		Result += BlockCount(Impl[NumBlocks() - 1] & LastBlockBitmask);

		return Result;
	}

	/** Sets all bits to true. */
	constexpr TStaticBitset& Set(bool InValue = true)
	{
		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl[Index] = static_cast<BlockType>(InValue ? -1 : 0);
		}

		return *this;
	}

	/** Flips all bits (like operator~, but in-place). */
	constexpr TStaticBitset& Flip()
	{
		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl[Index] = ~Impl[Index];
		}

		return *this;
	}

	/** Flips the bit at the position 'Index'. */
	constexpr TStaticBitset& Flip(size_t Index)
	{
		Impl[Index / BlockWidth] ^= 1ull << Index % BlockWidth;

		return *this;
	}

	/** Converts the contents of the bitset to an uint64 integer. */
	NODISCARD constexpr uint64 ToIntegral()
	{
		if constexpr (N > 64)
		{
			for (size_t Index = 64 / BlockWidth; Index < NumBlocks() - 1; ++Index)
			{
				checkf(Impl.Pointer[Index] != 0, TEXT("The bitset can not be represented in uint64. Please check Num()."));
			}

			const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;
			const BlockType LastBlock = Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask;

			checkf(LastBlock != 0, TEXT("The bitset can not be represented in uint64. Please check Num()."));
		}

		uint64 Result = 0;

		static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TStaticBitset is unexpected");

		if constexpr (sizeof(BlockType) == sizeof(uint8))
		{
			if constexpr (N >  0) Result |= static_cast<uint64>(Impl[0]) <<  0;
			if constexpr (N >  8) Result |= static_cast<uint64>(Impl[1]) <<  8;
			if constexpr (N > 16) Result |= static_cast<uint64>(Impl[2]) << 16;
			if constexpr (N > 24) Result |= static_cast<uint64>(Impl[3]) << 24;
			if constexpr (N > 32) Result |= static_cast<uint64>(Impl[4]) << 32;
			if constexpr (N > 40) Result |= static_cast<uint64>(Impl[5]) << 40;
			if constexpr (N > 48) Result |= static_cast<uint64>(Impl[6]) << 48;
			if constexpr (N > 56) Result |= static_cast<uint64>(Impl[7]) << 56;
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint16))
		{
			if constexpr (N >  0) Result |= static_cast<uint64>(Impl[0]) <<  0;
			if constexpr (N > 16) Result |= static_cast<uint64>(Impl[1]) << 16;
			if constexpr (N > 32) Result |= static_cast<uint64>(Impl[2]) << 32;
			if constexpr (N > 48) Result |= static_cast<uint64>(Impl[3]) << 48;
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint32))
		{
			if constexpr (N >  0) Result |= static_cast<uint64>(Impl[0]) <<  0;
			if constexpr (N > 32) Result |= static_cast<uint64>(Impl[1]) << 32;
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint64))
		{
			if constexpr (N >  0) Result |= static_cast<uint64>(Impl[0]) <<  0;
		}
		else check_no_entry();

		const uint64 Mask = Num() < 64 ? (1ull << Num()) - 1 : -1;

		return Result & Mask;
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE constexpr       BlockType* GetData()       { return Impl; }
	NODISCARD FORCEINLINE constexpr const BlockType* GetData() const { return Impl; }

	/** @return The iterator to the first or end bit. */
	NODISCARD FORCEINLINE constexpr      Iterator Begin()       { return      Iterator(this, Impl, 0);     }
	NODISCARD FORCEINLINE constexpr ConstIterator Begin() const { return ConstIterator(this, Impl, 0);     }
	NODISCARD FORCEINLINE constexpr      Iterator End()         { return      Iterator(this, Impl, Num()); }
	NODISCARD FORCEINLINE constexpr ConstIterator End()   const { return ConstIterator(this, Impl, Num()); }

	/** @return The reverse iterator to the first or end bit. */
	NODISCARD FORCEINLINE constexpr      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE constexpr      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE constexpr ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

	/** @return The number of bits in the bitset. */
	NODISCARD FORCEINLINE constexpr size_t Num() const { return N; }

	/** @return The number of blocks in the bitset. */
	NODISCARD FORCEINLINE constexpr size_t NumBlocks() const { return (Num() + BlockWidth - 1) / BlockWidth; }

	/** @return true if the bitset is empty, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE constexpr bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested bit. */
	NODISCARD FORCEINLINE constexpr      Reference operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }
	NODISCARD FORCEINLINE constexpr ConstReference operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }

	/** @return The reference to the first or last bit. */
	NODISCARD FORCEINLINE constexpr      Reference Front()       { return *Begin();     }
	NODISCARD FORCEINLINE constexpr ConstReference Front() const { return *Begin();     }
	NODISCARD FORCEINLINE constexpr      Reference Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE constexpr ConstReference Back()  const { return *(End() - 1); }

	/** Overloads the GetTypeHash algorithm for TStaticBitset. */
	NODISCARD friend FORCEINLINE constexpr size_t GetTypeHash(const TStaticBitset& A)
	{
		if constexpr (N == 0) return 1005426566;

		size_t Result = 0;

		for (size_t Index = 0; Index != A.NumBlocks() - 1; ++Index)
		{
			Result = HashCombine(Result, GetTypeHash(A.Impl[Index]));
		}

		const BlockType LastBlockBitmask = A.Num() % BlockWidth != 0 ? (1ull << A.Num() % BlockWidth) - 1 : -1;

		return HashCombine(Result, GetTypeHash(A.Impl[A.NumBlocks() - 1] & LastBlockBitmask));
	}

	/** Overloads the Swap algorithm for TStaticBitset. */
	friend constexpr void Swap(TStaticBitset& A, TStaticBitset& B) { Swap(A.Impl, B.Impl); }

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	BlockType Impl[N != 0 ? (N + BlockWidth - 1) / BlockWidth : 1];

public:

	class Reference final : private FSingleton
	{
	public:

		FORCEINLINE constexpr Reference& operator=(bool InValue) { Data = (Data & ~Mask) | (InValue ? Mask : 0); return *this; }

		FORCEINLINE constexpr Reference& operator=(const Reference& InValue) { *this = static_cast<bool>(InValue); return *this; }

		FORCEINLINE constexpr Reference& operator&=(bool InValue) { Data &= InValue ? -1 : ~Mask; return *this; }
		FORCEINLINE constexpr Reference& operator|=(bool InValue) { Data |= InValue ? Mask :   0; return *this; }
		FORCEINLINE constexpr Reference& operator^=(bool InValue) { *this = InValue ^ *this;      return *this; }

		FORCEINLINE constexpr bool operator~() const { return !*this; }

		FORCEINLINE constexpr operator bool() const { return (Data & Mask) != 0; }

	private:

		FORCEINLINE constexpr Reference(BlockType& InData, BlockType InMask)
			: Data(InData), Mask(InMask)
		{ }

		BlockType& Data;
		BlockType  Mask;

		friend Iterator;

	};

private:

	template <bool bConst>
	class TIteratorImpl final
	{
	private:

	public:

		using ElementType = bool;

		FORCEINLINE constexpr TIteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE constexpr TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer), BitOffset(InValue.BitOffset)
		{ }
#		else
		FORCEINLINE constexpr TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer), BitOffset(InValue.BitOffset)
		{ }
#		endif

		FORCEINLINE constexpr TIteratorImpl(const TIteratorImpl&)            = default;
		FORCEINLINE constexpr TIteratorImpl(TIteratorImpl&&)                 = default;
		FORCEINLINE constexpr TIteratorImpl& operator=(const TIteratorImpl&) = default;
		FORCEINLINE constexpr TIteratorImpl& operator=(TIteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE constexpr bool operator==(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset == RHS.BitOffset; }

		NODISCARD friend FORCEINLINE constexpr strong_ordering operator<=>(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset <=> RHS.BitOffset; }

		NODISCARD FORCEINLINE constexpr      Reference operator*() const requires (!bConst) { CheckThis(true); return Reference(*(Pointer + BitOffset / BlockWidth), 1ull << BitOffset % BlockWidth); }
		NODISCARD FORCEINLINE constexpr ConstReference operator*() const requires ( bConst) { CheckThis(true); return       (*(Pointer + BitOffset / BlockWidth) & (1ull << BitOffset % BlockWidth)); }

		NODISCARD FORCEINLINE constexpr auto operator[](ptrdiff Index) const { TIteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE constexpr TIteratorImpl& operator++() { ++BitOffset; CheckThis(); return *this; }
		FORCEINLINE constexpr TIteratorImpl& operator--() { --BitOffset; CheckThis(); return *this; }

		FORCEINLINE constexpr TIteratorImpl operator++(int) { TIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE constexpr TIteratorImpl operator--(int) { TIteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE constexpr TIteratorImpl& operator+=(ptrdiff Offset) { BitOffset += Offset; CheckThis(); return *this; }
		FORCEINLINE constexpr TIteratorImpl& operator-=(ptrdiff Offset) { BitOffset -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE constexpr TIteratorImpl operator+(TIteratorImpl Iter, ptrdiff Offset) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE constexpr TIteratorImpl operator+(ptrdiff Offset, TIteratorImpl Iter) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE constexpr TIteratorImpl operator-(ptrdiff Offset) const { TIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE constexpr ptrdiff operator-(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset - RHS.BitOffset; }

	private:

#		if DO_CHECK
		const TStaticBitset* Owner = nullptr;
#		endif

		using BlockPtr = TConditional<bConst, const BlockType*, BlockType*>;

		BlockPtr Pointer   = nullptr;
		size_t   BitOffset = 0;

#		if DO_CHECK
		FORCEINLINE constexpr TIteratorImpl(const TStaticBitset* InContainer, BlockPtr InPointer, size_t Offset)
			: Owner(InContainer), Pointer(InPointer), BitOffset(Offset)
		{ }
#		else
		FORCEINLINE constexpr TIteratorImpl(const TStaticBitset* InContainer, BlockPtr InPointer, size_t Offset)
			: Pointer(InPointer), BitOffset(Offset)
		{ }
#		endif

		FORCEINLINE constexpr void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool> friend class TIteratorImpl;

		friend TStaticBitset;

	};

};

#endif

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
