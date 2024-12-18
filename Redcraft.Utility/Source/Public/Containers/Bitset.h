#pragma once

#include "CoreTypes.h"
#include "Range/Range.h"
#include "Memory/Memory.h"
#include "Memory/Allocator.h"
#include "Iterator/Iterator.h"
#include "Templates/Utility.h"
#include "Templates/TypeHash.h"
#include "Templates/Noncopyable.h"
#include "TypeTraits/TypeTraits.h"
#include "Miscellaneous/Compare.h"
#include "Memory/MemoryOperator.h"
#include "Miscellaneous/AssertionMacros.h"

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

template <CUnsignedIntegral InBlockType> requires (!CSameAs<InBlockType, bool>)
using TDefaultBitsetAllocator = TInlineAllocator<(40 - 3 * sizeof(size_t)) / sizeof(InBlockType)>;

template <CUnsignedIntegral InBlockType, CAllocator<InBlockType> Allocator = TDefaultBitsetAllocator<InBlockType>> requires (CAllocatableObject<InBlockType> && !CSameAs<InBlockType, bool>)
class TBitset
{
private:

	template <bool bConst>
	class TIteratorImpl;

public:

	using FBlockType     = InBlockType;
	using FElementType   = bool;
	using FAllocatorType = Allocator;

	class      FReference;
	using FConstReference = bool;

	using      FIterator = TIteratorImpl<false>;
	using FConstIterator = TIteratorImpl<true >;

	using      FReverseIterator = TReverseIterator<     FIterator>;
	using FConstReverseIterator = TReverseIterator<FConstIterator>;

	static_assert(CRandomAccessIterator<     FIterator>);
	static_assert(CRandomAccessIterator<FConstIterator>);

	static constexpr size_t BlockWidth = sizeof(FBlockType) * 8;

	/** Default constructor. Constructs an empty bitset. */
	FORCEINLINE TBitset() : TBitset(0) { }

	/** Constructs the bitset with 'Count' uninitialized bits. */
	FORCEINLINE explicit TBitset(size_t InCount)
	{
		Impl.BitsetNum = InCount;
		Impl.BlocksMax = Impl->CalculateSlackReserve(NumBlocks());
		Impl.Pointer   = Impl->Allocate(MaxBlocks());
	}

	/** Constructs a bitset from an integer. */
	TBitset(size_t InCount, uint64 InValue) : TBitset(InCount > 64 ? InCount : 64)
	{
		static_assert(sizeof(FBlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");

		if constexpr (sizeof(FBlockType) == sizeof(uint8))
		{
			Impl.Pointer[0] = static_cast<FBlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<FBlockType>(InValue >>  8);
			Impl.Pointer[2] = static_cast<FBlockType>(InValue >> 16);
			Impl.Pointer[3] = static_cast<FBlockType>(InValue >> 24);
			Impl.Pointer[4] = static_cast<FBlockType>(InValue >> 32);
			Impl.Pointer[5] = static_cast<FBlockType>(InValue >> 40);
			Impl.Pointer[6] = static_cast<FBlockType>(InValue >> 48);
			Impl.Pointer[7] = static_cast<FBlockType>(InValue >> 56);
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint16))
		{
			Impl.Pointer[0] = static_cast<FBlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<FBlockType>(InValue >> 16);
			Impl.Pointer[2] = static_cast<FBlockType>(InValue >> 32);
			Impl.Pointer[3] = static_cast<FBlockType>(InValue >> 48);
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint32))
		{
			Impl.Pointer[0] = static_cast<FBlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<FBlockType>(InValue >> 32);
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint64))
		{
			Impl.Pointer[0] = static_cast<FBlockType>(InValue >>  0);
		}
		else check_no_entry();

		size_t BlockInteger = sizeof(uint64) / sizeof(FBlockType);

		Memory::Memset(Impl.Pointer + BlockInteger, 0, (NumBlocks() - BlockInteger) * sizeof(FBlockType));

		Impl.BitsetNum = InCount;
	}

	/** Constructs the bitset with the bits of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<bool, TIteratorReference<I>>)
	TBitset(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			size_t Count = 0;

			if constexpr (CSizedSentinelFor<S, I>)
			{
				checkf(First - Last <= 0, TEXT("Illegal range iterator. Please check First <= Last."));

				Count = Last - First;
			}
			else for (I Iter = First; Iter != Last; ++Iter) ++Count;

			new (this) TBitset(Count);

			for (FReference Ref: *this) Ref = *First++;
		}
		else
		{
			new (this) TBitset(0);

			while (First != Last)
			{
				PushBack(*First);
				++First;
			}
		}
	}

	/** Constructs the bitset with the bits of the range. */
	template <CInputRange R> requires (!CSameAs<TRemoveCVRef<R>, TBitset> && CConstructibleFrom<bool, TRangeReference<R>>)
	FORCEINLINE explicit TBitset(R&& Range) : TBitset(Range::Begin(Range), Range::End(Range)) { }

	/** Copy constructor. Constructs the bitset with the copy of the bits of 'InValue'. */
	FORCEINLINE TBitset(const TBitset& InValue)
	{
		Impl.BitsetNum = InValue.Num();
		Impl.BlocksMax = Impl->CalculateSlackReserve(NumBlocks());
		Impl.Pointer   = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(FBlockType));
	}

	/** Move constructor. After the move, 'InValue' is guaranteed to be empty. */
	TBitset(TBitset&& InValue)
	{
		Impl.BitsetNum = InValue.Num();

		if (InValue.Impl->IsTransferable(InValue.Impl.Pointer))
		{
			Impl.BlocksMax = InValue.MaxBlocks();
			Impl.Pointer   = InValue.Impl.Pointer;

			InValue.Impl.BitsetNum = 0;
			InValue.Impl.BlocksMax = InValue.Impl->CalculateSlackReserve(InValue.NumBlocks());
			InValue.Impl.Pointer   = InValue.Impl->Allocate(InValue.MaxBlocks());
		}
		else
		{
			Impl.BlocksMax = Impl->CalculateSlackReserve(NumBlocks());
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(FBlockType));
		}
	}

	/** Constructs the bitset with the bits of the initializer list. */
	FORCEINLINE TBitset(initializer_list<bool> IL) : TBitset(Range::Begin(IL), Range::End(IL)) { }

	/** Destructs the bitset. The storage is deallocated. */
	~TBitset()
	{
		Impl->Deallocate(Impl.Pointer);
	}

	/** Copy assignment operator. Replaces the bits with a copy of the bits of 'InValue'. */
	TBitset& operator=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		size_t NumToAllocate = InValue.NumBlocks();

		NumToAllocate = NumToAllocate > MaxBlocks() ? Impl->CalculateSlackGrow(InValue.NumBlocks(), MaxBlocks())   : NumToAllocate;
		NumToAllocate = NumToAllocate < MaxBlocks() ? Impl->CalculateSlackShrink(InValue.NumBlocks(), MaxBlocks()) : NumToAllocate;

		if (NumToAllocate != MaxBlocks())
		{
			Impl->Deallocate(Impl.Pointer);

			Impl.BitsetNum = InValue.Num();
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(FBlockType));

			return *this;
		}

		check(InValue.Num() <= Max());

		Impl.BitsetNum = InValue.Num();

		Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(FBlockType));

		return *this;
	}

	/** Move assignment operator. After the move, 'InValue' is guaranteed to be empty. */
	TBitset& operator=(TBitset&& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (InValue.Impl->IsTransferable(InValue.Impl.Pointer))
		{
			Impl->Deallocate(Impl.Pointer);

			Impl.Pointer = InValue.Impl.Pointer;

			InValue.Impl.BitsetNum = 0;
			InValue.Impl.BlocksMax = InValue.Impl->CalculateSlackReserve(InValue.NumBlocks());
			InValue.Impl.Pointer   = InValue.Impl->Allocate(InValue.MaxBlocks());

			return *this;
		}

		*this = InValue;

		InValue.Reset();

		return *this;
	}

	/** Replaces the bits with those identified by initializer list. */
	TBitset& operator=(initializer_list<bool> IL)
	{
		auto First = Range::Begin(IL);

		const size_t BlocksCount = (Range::Num(IL) + BlockWidth - 1) / BlockWidth;

		size_t NumToAllocate = BlocksCount;

		NumToAllocate = NumToAllocate > MaxBlocks() ? Impl->CalculateSlackGrow(BlocksCount, MaxBlocks())   : NumToAllocate;
		NumToAllocate = NumToAllocate < MaxBlocks() ? Impl->CalculateSlackShrink(BlocksCount, MaxBlocks()) : NumToAllocate;

		if (NumToAllocate != MaxBlocks())
		{
			Impl->Deallocate(Impl.Pointer);

			Impl.BitsetNum = Range::Num(IL);
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			for (FReference Ref : *this) Ref = *First++;

			return *this;
		}

		Impl.BitsetNum = Range::Num(IL);

		for (FReference Ref : *this) Ref = *First++;

		return *this;
	}

	/** Compares the bits of two bitsets. */
	NODISCARD friend bool operator==(const TBitset& LHS, const TBitset& RHS)
	{
		if (LHS.Num() != RHS.Num()) return false;

		if (LHS.NumBlocks() == 0) return true;

		for (size_t Index = 0; Index != LHS.NumBlocks() - 1; ++Index)
		{
			if (LHS.Impl.Pointer[Index] != RHS.Impl.Pointer[Index]) return false;
		}

		const FBlockType LastBlockBitmask = LHS.Num() % BlockWidth != 0 ? (1ull << LHS.Num() % BlockWidth) - 1 : -1;

		return (LHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask) == (RHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask);
	}

	/** Sets the bits to the result of binary AND on corresponding pairs of bits of *this and other. */
	TBitset& operator&=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0) return *this;

		if (InValue.Num() == 0) return Set(false);

		if (Num() <= InValue.Num())
		{
			for (size_t Index = 0; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] &= InValue.Impl.Pointer[Index];
			}
		}
		else
		{
			const size_t LastBlock = InValue.NumBlocks() - 1;

			for (size_t Index = 0; Index != LastBlock; ++Index)
			{
				Impl.Pointer[Index] &= InValue.Impl.Pointer[Index];
			}

			const FBlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;

			Impl.Pointer[LastBlock] &= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;

			for (size_t Index = LastBlock + 1; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] = 0;
			}
		}

		return *this;
	}

	/** Sets the bits to the result of binary OR on corresponding pairs of bits of *this and other. */
	TBitset& operator|=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0) return *this;

		if (InValue.Num() == 0) return *this;

		if (Num() <= InValue.Num())
		{
			for (size_t Index = 0; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] |= InValue.Impl.Pointer[Index];
			}
		}
		else
		{
			const size_t LastBlock = InValue.NumBlocks() - 1;

			for (size_t Index = 0; Index != LastBlock; ++Index)
			{
				Impl.Pointer[Index] |= InValue.Impl.Pointer[Index];
			}

			const FBlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;

			Impl.Pointer[LastBlock] |= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;
		}

		return *this;
	}

	/** Sets the bits to the result of binary XOR on corresponding pairs of bits of *this and other. */
	TBitset& operator^=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0) return *this;

		if (InValue.Num() == 0) return *this;

		if (Num() <= InValue.Num())
		{
			for (size_t Index = 0; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] ^= InValue.Impl.Pointer[Index];
			}
		}
		else
		{
			const size_t LastBlock = InValue.NumBlocks() - 1;

			for (size_t Index = 0; Index != LastBlock; ++Index)
			{
				Impl.Pointer[Index] ^= InValue.Impl.Pointer[Index];
			}

			const FBlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;

			Impl.Pointer[LastBlock] ^= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;
		}

		return *this;
	}

	NODISCARD friend FORCEINLINE TBitset operator&(const TBitset& LHS, const TBitset& RHS) { return LHS.Num() < RHS.Num() ? TBitset(RHS) &= LHS : TBitset(LHS) &= RHS; }
	NODISCARD friend FORCEINLINE TBitset operator|(const TBitset& LHS, const TBitset& RHS) { return LHS.Num() < RHS.Num() ? TBitset(RHS) |= LHS : TBitset(LHS) |= RHS; }
	NODISCARD friend FORCEINLINE TBitset operator^(const TBitset& LHS, const TBitset& RHS) { return LHS.Num() < RHS.Num() ? TBitset(RHS) ^= LHS : TBitset(LHS) ^= RHS; }

	/** @return The temporary copy of *this with all bits flipped (binary NOT). */
	NODISCARD TBitset operator~() const
	{
		TBitset Result = *this;

		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Result.Impl.Pointer[Index] = ~Result.Impl.Pointer[Index];
		}

		return Result;
	}

	/** Performs binary shift left. */
	TBitset& operator<<=(size_t Offset)
	{
		const size_t Blockshift = Offset / BlockWidth;
		const size_t Bitshift   = Offset % BlockWidth;

		if (Num() == 0) return *this;

		if (Blockshift != 0)
		{
			for (size_t Index = NumBlocks() - 1; Index != -1; --Index)
			{
				Impl.Pointer[Index] = Index >= Blockshift ? Impl.Pointer[Index - Blockshift] : 0;
			}
		}

		if (Bitshift != 0)
		{
			for (size_t Index = NumBlocks() - 1; Index != 0; --Index)
			{
				Impl.Pointer[Index] = Impl.Pointer[Index] << Bitshift | Impl.Pointer[Index - 1] >> (BlockWidth - Bitshift);
			}

			Impl.Pointer[0] <<= Bitshift;
		}

		return *this;
	}

	/** Performs binary shift right. */
	TBitset& operator>>=(size_t Offset)
	{
		const size_t Blockshift = Offset / BlockWidth;
		const size_t Bitshift   = Offset % BlockWidth;

		if (Num() == 0) return *this;

		if (Num() % BlockWidth != 0)
		{
			Impl.Pointer[NumBlocks() - 1] &= (1ull << Num() % BlockWidth) - 1;
		}

		if (Blockshift != 0)
		{
			for (size_t Index = 0; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] = Index < NumBlocks() - Blockshift ? Impl.Pointer[Index + Blockshift] : 0;
			}
		}

		if (Bitshift != 0)
		{
			for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
			{
				Impl.Pointer[Index] = Impl.Pointer[Index] >> Bitshift | Impl.Pointer[Index + 1] << (BlockWidth - Bitshift);
			}

			Impl.Pointer[NumBlocks() - 1] >>= Bitshift;
		}

		return *this;
	}

	NODISCARD FORCEINLINE TBitset operator<<(size_t Offset) const { return TBitset(*this) <<= Offset; }
	NODISCARD FORCEINLINE TBitset operator>>(size_t Offset) const { return TBitset(*this) >>= Offset; }

	/** @return true if all bits are set to true, otherwise false. */
	NODISCARD bool All() const
	{
		if (Num() == 0) return true;

		for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
		{
			if (Impl.Pointer[Index] != -1) return false;
		}

		const FBlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		return (Impl.Pointer[NumBlocks() - 1] | ~LastBlockBitmask) == -1;
	}

	/** @return true if any of the bits are set to true, otherwise false. */
	NODISCARD bool Any() const
	{
		if (Num() == 0) return false;

		for (size_t Index = 0; Index != NumBlocks() - 1; ++Index)
		{
			if (Impl.Pointer[Index] != 0) return true;
		}

		const FBlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		return (Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask) != 0;
	}

	/** @return true if none of the bits are set to true, otherwise false. */
	NODISCARD FORCEINLINE bool None() const { return !Any(); }

	/** @return The number of bits that are set to true. */
	NODISCARD size_t Count() const
	{
		if (Num() == 0) return 0;

		size_t Result = 0;

		constexpr auto BlockCount = [](FBlockType Block)
		{
			static_assert(sizeof(FBlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");

			if constexpr (sizeof(FBlockType) == sizeof(uint8))
			{
				Block = (Block & 0x55ull) + ((Block >> 1) & 0x55ull);
				Block = (Block & 0x33ull) + ((Block >> 2) & 0x33ull);
				Block = (Block & 0x0Full) + ((Block >> 4) & 0x0Full);
			}
			else if constexpr (sizeof(FBlockType) == sizeof(uint16))
			{
				Block = (Block & 0x5555ull) + ((Block >>  1) & 0x5555ull);
				Block = (Block & 0x3333ull) + ((Block >>  2) & 0x3333ull);
				Block = (Block & 0x0F0Full) + ((Block >>  4) & 0x0F0Full);
				Block = (Block & 0x00FFull) + ((Block >>  8) & 0x00FFull);
			}
			else if constexpr (sizeof(FBlockType) == sizeof(uint32))
			{
				Block = (Block & 0x55555555ull) + ((Block >>  1) & 0x55555555ull);
				Block = (Block & 0x33333333ull) + ((Block >>  2) & 0x33333333ull);
				Block = (Block & 0x0F0F0F0Full) + ((Block >>  4) & 0x0F0F0F0Full);
				Block = (Block & 0x00FF00FFull) + ((Block >>  8) & 0x00FF00FFull);
				Block = (Block & 0x0000FFFFull) + ((Block >> 16) & 0x0000FFFFull);
			}
			else if constexpr (sizeof(FBlockType) == sizeof(uint64))
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
			Result += BlockCount(Impl.Pointer[Index]);
		}

		const FBlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		Result += BlockCount(Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask);

		return Result;
	}

	/** Sets all bits to true. */
	TBitset& Set(bool InValue = true)
	{
		Memory::Memset(Impl.Pointer, static_cast<uint8>(InValue ? -1 : 0), NumBlocks() * sizeof(FBlockType));

		return *this;
	}

	/** Flips all bits (like operator~, but in-place). */
	TBitset& Flip()
	{
		for (size_t Index = 0; Index != NumBlocks(); ++Index)
		{
			Impl.Pointer[Index] = ~Impl.Pointer[Index];
		}

		return *this;
	}

	/** Flips the bit at the position 'Index'. */
	TBitset& Flip(size_t Index)
	{
		Impl.Pointer[Index / BlockWidth] ^= 1ull << Index % BlockWidth;

		return *this;
	}

	/** Converts the contents of the bitset to an uint64 integer. */
	NODISCARD uint64 ToIntegral()
	{
		if (Num() > 64)
		{
			for (size_t Index = 64 / BlockWidth; Index < NumBlocks() - 1; ++Index)
			{
				checkf(Impl.Pointer[Index] != 0, TEXT("The bitset can not be represented in uint64. Please check Num()."));
			}

			const FBlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;
			const FBlockType LastBlock = Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask;

			checkf(LastBlock != 0, TEXT("The bitset can not be represented in uint64. Please check Num()."));
		}

		uint64 Result = 0;

		static_assert(sizeof(FBlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");

		if constexpr (sizeof(FBlockType) == sizeof(uint8))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
			Result |= static_cast<uint64>(Impl.Pointer[1]) <<  8;
			Result |= static_cast<uint64>(Impl.Pointer[2]) << 16;
			Result |= static_cast<uint64>(Impl.Pointer[3]) << 24;
			Result |= static_cast<uint64>(Impl.Pointer[4]) << 32;
			Result |= static_cast<uint64>(Impl.Pointer[5]) << 40;
			Result |= static_cast<uint64>(Impl.Pointer[6]) << 48;
			Result |= static_cast<uint64>(Impl.Pointer[7]) << 56;
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint16))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
			Result |= static_cast<uint64>(Impl.Pointer[1]) << 16;
			Result |= static_cast<uint64>(Impl.Pointer[2]) << 32;
			Result |= static_cast<uint64>(Impl.Pointer[3]) << 48;
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint32))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
			Result |= static_cast<uint64>(Impl.Pointer[1]) << 32;
		}
		else if constexpr (sizeof(FBlockType) == sizeof(uint64))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
		}
		else check_no_entry();

		const uint64 Mask = Num() < 64 ? (1ull << Num()) - 1 : -1;

		return Result & Mask;
	}

	/** Appends the given bit value to the end of the bitset. */
	FORCEINLINE void PushBack(bool InValue)
	{
		SetNum(Num() + 1);
		Back() = InValue;
	}

	/** Removes the last bit of the bitset. The bitset cannot be empty. */
	FORCEINLINE void PopBack(bool bAllowShrinking = true)
	{
		checkf(Num() != 0, TEXT("The bitset is empty. Please check Num()."));
		SetNum(Num() - 1, bAllowShrinking);
	}

	/** Resizes the bitset to contain 'InCount' bits. Additional uninitialized bits are appended. */
	void SetNum(size_t InCount, bool bAllowShrinking = true)
	{
		const size_t BlocksCount = (InCount + BlockWidth - 1) / BlockWidth;

		size_t NumToAllocate = BlocksCount;

		NumToAllocate = NumToAllocate > MaxBlocks()                    ? Impl->CalculateSlackGrow(BlocksCount, MaxBlocks())                  : NumToAllocate;
		NumToAllocate = NumToAllocate < MaxBlocks() ? (bAllowShrinking ? Impl->CalculateSlackShrink(BlocksCount, MaxBlocks()) : MaxBlocks()) : NumToAllocate;

		if (NumToAllocate != MaxBlocks())
		{
			FBlockType*  OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = NumBlocks();

			Impl.BitsetNum = InCount;
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			if (NumToDestruct <= Num())
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, NumToDestruct * sizeof(FBlockType));
			}
			else
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, BlocksCount * sizeof(FBlockType));
			}

			Impl->Deallocate(OldAllocation);

			return;
		}

		check(InCount <= Max());

		Impl.BitsetNum = InCount;
	}

	/** Resizes the bitset to contain 'InCount' elements. Additional copies of 'InValue' are appended. */
	void SetNum(size_t InCount, bool InValue, bool bAllowShrinking /*= true*/)
	{
		const size_t BlocksCount = (InCount + BlockWidth - 1) / BlockWidth;

		size_t NumToAllocate = BlocksCount;

		NumToAllocate = NumToAllocate > MaxBlocks()                    ? Impl->CalculateSlackGrow(BlocksCount, MaxBlocks())                  : NumToAllocate;
		NumToAllocate = NumToAllocate < MaxBlocks() ? (bAllowShrinking ? Impl->CalculateSlackShrink(BlocksCount, MaxBlocks()) : MaxBlocks()) : NumToAllocate;

		const FBlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;
		const FBlockType BlocksValueToSet = static_cast<FBlockType>(InValue ? -1 : 0);

		if (NumToAllocate != MaxBlocks())
		{
			FBlockType*  OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = NumBlocks();

			Impl.BitsetNum = InCount;
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			if (NumToDestruct <= NumBlocks())
			{
				if (NumToDestruct != 0)
				{
					Memory::Memcpy(Impl.Pointer, OldAllocation, (NumToDestruct - 1) * sizeof(FBlockType));

					Impl.Pointer[NumToDestruct - 1] = OldAllocation[NumToDestruct - 1] & LastBlockBitmask | BlocksValueToSet & ~LastBlockBitmask;
				}

				Memory::Memset(Impl.Pointer + NumToDestruct, static_cast<uint8>(BlocksValueToSet), (BlocksCount - NumToDestruct) * sizeof(FBlockType));
			}
			else
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, BlocksCount * sizeof(FBlockType));
			}

			Impl->Deallocate(OldAllocation);

			return;
		}

		check(InCount <= Max());

		if (InCount > Num())
		{
			if (NumBlocks() != 0)
			{
				Impl.Pointer[NumBlocks() - 1] = Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask | BlocksValueToSet & ~LastBlockBitmask;
			}

			Memory::Memset(Impl.Pointer + NumBlocks(), static_cast<uint8>(BlocksValueToSet), (BlocksCount - NumBlocks()) * sizeof(FBlockType));
		}

		Impl.BitsetNum = InCount;
	}

	/** Increase the max capacity of the bitset to a value that's greater or equal to 'InCount'. */
	void Reserve(size_t InCount)
	{
		if (InCount <= Max()) return;

		const size_t BlocksCount = (InCount + BlockWidth - 1) / BlockWidth;

		const size_t NumToAllocate = Impl->CalculateSlackReserve(BlocksCount);
		FBlockType*  OldAllocation = Impl.Pointer;

		check(NumToAllocate > MaxBlocks());

		Impl.BlocksMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, OldAllocation, NumBlocks() * sizeof(FBlockType));

		Impl->Deallocate(OldAllocation);
	}

	/** Requests the removal of unused capacity. */
	void Shrink()
	{
		size_t NumToAllocate = Impl->CalculateSlackReserve(NumBlocks());

		check(NumToAllocate <= MaxBlocks());

		if (NumToAllocate == MaxBlocks()) return;

		FBlockType* OldAllocation = Impl.Pointer;

		Impl.BitsetNum = NumToAllocate * BlockWidth;
		Impl.Pointer   = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, OldAllocation, NumBlocks() * sizeof(FBlockType));

		Impl->Deallocate(OldAllocation);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE       FBlockType* GetData()       { return Impl.Pointer; }
	NODISCARD FORCEINLINE const FBlockType* GetData() const { return Impl.Pointer; }

	/** @return The iterator to the first or end bit. */
	NODISCARD FORCEINLINE      FIterator Begin()       { return      FIterator(this, Impl.Pointer, 0);     }
	NODISCARD FORCEINLINE FConstIterator Begin() const { return FConstIterator(this, Impl.Pointer, 0);     }
	NODISCARD FORCEINLINE      FIterator End()         { return      FIterator(this, Impl.Pointer, Num()); }
	NODISCARD FORCEINLINE FConstIterator End()   const { return FConstIterator(this, Impl.Pointer, Num()); }

	/** @return The reverse iterator to the first or end bit. */
	NODISCARD FORCEINLINE      FReverseIterator RBegin()       { return      FReverseIterator(End());   }
	NODISCARD FORCEINLINE FConstReverseIterator RBegin() const { return FConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      FReverseIterator REnd()         { return      FReverseIterator(Begin()); }
	NODISCARD FORCEINLINE FConstReverseIterator REnd()   const { return FConstReverseIterator(Begin()); }

	/** @return The number of bits in the bitset. */
	NODISCARD FORCEINLINE size_t Num() const { return Impl.BitsetNum; }

	/** @return The number of bits that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t Max() const { return MaxBlocks() * BlockWidth; }

	/** @return The number of blocks in the bitset. */
	NODISCARD FORCEINLINE size_t NumBlocks() const { return (Num() + BlockWidth - 1) / BlockWidth; }

	/** @return The number of blocks that can be held in currently allocated storage. */
	NODISCARD FORCEINLINE size_t MaxBlocks() const { return Impl.BlocksMax; }

	/** @return true if the bitset is empty, false otherwise. */
	NODISCARD FORCEINLINE bool IsEmpty() const { return Num() == 0; }

	/** @return true if the iterator is valid, false otherwise. */
	NODISCARD FORCEINLINE bool IsValidIterator(FConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested bit. */
	NODISCARD FORCEINLINE      FReference operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }
	NODISCARD FORCEINLINE FConstReference operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }

	/** @return The reference to the first or last bit. */
	NODISCARD FORCEINLINE      FReference Front()       { return *Begin();     }
	NODISCARD FORCEINLINE FConstReference Front() const { return *Begin();     }
	NODISCARD FORCEINLINE      FReference Back()        { return *(End() - 1); }
	NODISCARD FORCEINLINE FConstReference Back()  const { return *(End() - 1); }

	/** Erases all bits from the bitset. After this call, Num() returns zero. */
	void Reset(bool bAllowShrinking = true)
	{
		const size_t NumToAllocate = Impl->CalculateSlackReserve(0);

		if (bAllowShrinking && NumToAllocate != MaxBlocks())
		{
			Impl->Deallocate(Impl.Pointer);

			Impl.BitsetNum = 0;
			Impl.BlocksMax = Impl->CalculateSlackReserve(NumBlocks());
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			return;
		}

		Impl.BitsetNum = 0;
	}

	/** Overloads the GetTypeHash algorithm for TBitset. */
	NODISCARD friend FORCEINLINE size_t GetTypeHash(const TBitset& A)
	{
		if (A.NumBlocks() == 0) return 855406835;

		size_t Result = 0;

		for (size_t Index = 0; Index != A.NumBlocks() - 1; ++Index)
		{
			Result = HashCombine(Result, GetTypeHash(A.Impl.Pointer[Index]));
		}

		const FBlockType LastBlockBitmask = A.Num() % BlockWidth != 0 ? (1ull << A.Num() % BlockWidth) - 1 : -1;

		return HashCombine(Result, GetTypeHash(A.Impl.Pointer[A.NumBlocks() - 1] & LastBlockBitmask));
	}

	/** Overloads the Swap algorithm for TBitset. */
	friend void Swap(TBitset& A, TBitset& B)
	{
		const bool bIsTransferable =
			A.Impl->IsTransferable(A.Impl.Pointer) &&
			B.Impl->IsTransferable(B.Impl.Pointer);

		if (bIsTransferable)
		{
			Swap(A.Impl.BitsetNum, B.Impl.BitsetNum);
			Swap(A.Impl.BlocksMax, B.Impl.BlocksMax);
			Swap(A.Impl.Pointer,   B.Impl.Pointer);

			return;
		}

		TBitset Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
	}

	ENABLE_RANGE_BASED_FOR_LOOP_SUPPORT

private:

	ALLOCATOR_WRAPPER_BEGIN(FAllocatorType, FBlockType, Impl)
	{
		size_t BitsetNum;
		size_t BlocksMax;
		FBlockType* Pointer;
	}
	ALLOCATOR_WRAPPER_END(FAllocatorType, FBlockType, Impl)

public:

	class FReference final : private FSingleton
	{
	public:

		FORCEINLINE FReference& operator=(bool InValue) { Data = (Data & ~Mask) | (InValue ? Mask : 0); return *this; }

		FORCEINLINE FReference& operator=(const FReference& InValue) { *this = static_cast<bool>(InValue); return *this; }

		FORCEINLINE FReference& operator&=(bool InValue) { Data &= InValue ? -1 : ~Mask; return *this; }
		FORCEINLINE FReference& operator|=(bool InValue) { Data |= InValue ? Mask :   0; return *this; }
		FORCEINLINE FReference& operator^=(bool InValue) { *this = InValue ^ *this;      return *this; }

		FORCEINLINE bool operator~() const { return !*this; }

		FORCEINLINE operator bool() const { return (Data & Mask) != 0; }

	private:

		FORCEINLINE FReference(FBlockType& InData, FBlockType InMask)
			: Data(InData), Mask(InMask)
		{ }

		FBlockType& Data;
		FBlockType  Mask;

		friend FIterator;

	};

private:

	template <bool bConst>
	class TIteratorImpl final
	{
	public:

		using FElementType = bool;

		FORCEINLINE TIteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer), BitOffset(InValue.BitOffset)
		{ }
#		else
		FORCEINLINE TIteratorImpl(const TIteratorImpl<false>& InValue) requires (bConst)
			: Pointer(InValue.Pointer), BitOffset(InValue.BitOffset)
		{ }
#		endif

		FORCEINLINE TIteratorImpl(const TIteratorImpl&)            = default;
		FORCEINLINE TIteratorImpl(TIteratorImpl&&)                 = default;
		FORCEINLINE TIteratorImpl& operator=(const TIteratorImpl&) = default;
		FORCEINLINE TIteratorImpl& operator=(TIteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE bool operator==(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset == RHS.BitOffset; }

		NODISCARD friend FORCEINLINE strong_ordering operator<=>(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset <=> RHS.BitOffset; }

		NODISCARD FORCEINLINE      FReference operator*() const requires (!bConst) { CheckThis(true); return FReference(*(Pointer + BitOffset / BlockWidth), 1ull << BitOffset % BlockWidth);  }
		NODISCARD FORCEINLINE FConstReference operator*() const requires ( bConst) { CheckThis(true); return         (*(Pointer + BitOffset / BlockWidth) & (1ull << BitOffset % BlockWidth)); }

		NODISCARD FORCEINLINE auto operator[](ptrdiff Index) const { TIteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE TIteratorImpl& operator++() { ++BitOffset; CheckThis(); return *this; }
		FORCEINLINE TIteratorImpl& operator--() { --BitOffset; CheckThis(); return *this; }

		FORCEINLINE TIteratorImpl operator++(int) { TIteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE TIteratorImpl operator--(int) { TIteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE TIteratorImpl& operator+=(ptrdiff Offset) { BitOffset += Offset; CheckThis(); return *this; }
		FORCEINLINE TIteratorImpl& operator-=(ptrdiff Offset) { BitOffset -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE TIteratorImpl operator+(TIteratorImpl Iter, ptrdiff Offset) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE TIteratorImpl operator+(ptrdiff Offset, TIteratorImpl Iter) { TIteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE TIteratorImpl operator-(ptrdiff Offset) const { TIteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE ptrdiff operator-(const TIteratorImpl& LHS, const TIteratorImpl& RHS) { check(LHS.Pointer == RHS.Pointer); return LHS.BitOffset - RHS.BitOffset; }

	private:

#		if DO_CHECK
		const TBitset* Owner = nullptr;
#		endif

		using FBlockPtr = TConditional<bConst, const FBlockType*, FBlockType*>;

		FBlockPtr Pointer   = nullptr;
		size_t    BitOffset = 0;

#		if DO_CHECK
		FORCEINLINE TIteratorImpl(const TBitset* InContainer, FBlockPtr InPointer, size_t Offset)
			: Owner(InContainer), Pointer(InPointer), BitOffset(Offset)
		{ }
#		else
		FORCEINLINE TIteratorImpl(const TBitset* InContainer, FBlockPtr InPointer, size_t Offset)
			: Pointer(InPointer), BitOffset(Offset)
		{ }
#		endif

		FORCEINLINE void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool> friend class TIteratorImpl;

		friend TBitset;

	};

};

using FBitset = TBitset<uint64>;

static_assert(sizeof(FBitset) == 40, "The byte size of FBitset is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
