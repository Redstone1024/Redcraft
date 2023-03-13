#pragma once

#include "CoreTypes.h"
#include "Memory/Memory.h"
#include "Memory/Allocator.h"
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

template <CUnsignedIntegral InBlockType> requires (!CSameAs<InBlockType, bool>)
using TDefaultBitsetAllocator = TInlineAllocator<(40 - 3 * sizeof(size_t)) / sizeof(InBlockType)>;

NAMESPACE_PRIVATE_END

template <CUnsignedIntegral InBlockType, CInstantiableAllocator Allocator = NAMESPACE_PRIVATE::TDefaultBitsetAllocator<InBlockType>> requires (!CSameAs<InBlockType, bool>)
class TBitset final
{
private:

	template <bool bConst>
	class IteratorImpl;

public:

	using BlockType     = InBlockType;
	using ElementType   = bool;
	using AllocatorType = Allocator;

	class      Reference;
	using ConstReference = bool;

	using      Iterator = IteratorImpl<false>;
	using ConstIterator = IteratorImpl<true >;

	using      ReverseIterator = TReverseIterator<     Iterator>;
	using ConstReverseIterator = TReverseIterator<ConstIterator>;

	static_assert(CRandomAccessIterator<     Iterator>);
	static_assert(CRandomAccessIterator<ConstIterator>);

	static constexpr size_t BlockWidth = sizeof(BlockType) * 8;

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
		static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");

		if constexpr (sizeof(BlockType) == sizeof(uint8))
		{
			Impl.Pointer[0] = static_cast<BlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<BlockType>(InValue >>  8);
			Impl.Pointer[2] = static_cast<BlockType>(InValue >> 16);
			Impl.Pointer[3] = static_cast<BlockType>(InValue >> 24);
			Impl.Pointer[4] = static_cast<BlockType>(InValue >> 32);
			Impl.Pointer[5] = static_cast<BlockType>(InValue >> 40);
			Impl.Pointer[6] = static_cast<BlockType>(InValue >> 48);
			Impl.Pointer[7] = static_cast<BlockType>(InValue >> 56);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint16))
		{
			Impl.Pointer[0] = static_cast<BlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<BlockType>(InValue >> 16);
			Impl.Pointer[2] = static_cast<BlockType>(InValue >> 32);
			Impl.Pointer[3] = static_cast<BlockType>(InValue >> 48);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint32))
		{
			Impl.Pointer[0] = static_cast<BlockType>(InValue >>  0);
			Impl.Pointer[1] = static_cast<BlockType>(InValue >> 32);
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint64))
		{
			Impl.Pointer[0] = static_cast<BlockType>(InValue >>  0);
		}
		else check_no_entry();

		size_t BlockInteger = sizeof(uint64) / sizeof(BlockType);

		Memory::Memset(Impl.Pointer + BlockInteger, 0, (NumBlocks() - BlockInteger) * sizeof(BlockType));

		Impl.BitsetNum = InCount;
	}

	/** Constructs the bitset with the bits of the range ['First', 'Last'). */
	template <CInputIterator I, CSentinelFor<I> S> requires (CConstructibleFrom<bool, TIteratorReferenceType<I>>)
	TBitset(I First, S Last)
	{
		if constexpr (CForwardIterator<I>)
		{
			if (CSizedSentinelFor<S, I>) { checkf(First <= Last, TEXT("Illegal range iterator. Please check First <= Last.")); }

			const size_t InCount = Iteration::Distance(First, Last);

			new (this) TBitset(InCount);

			BlockType* CurrentBlock = Impl.Pointer - 1;

			for (Reference Ref: *this) Ref = *First++;
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

	/** Copy constructor. Constructs the bitset with the copy of the bits of 'InValue'. */
	FORCEINLINE TBitset(const TBitset& InValue)
	{
		Impl.BitsetNum = InValue.Num();
		Impl.BlocksMax = Impl->CalculateSlackReserve(NumBlocks());
		Impl.Pointer   = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(BlockType));
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

			Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(BlockType));
		}
	}

	/** Constructs the bitset with the bits of the initializer list. */
	FORCEINLINE TBitset(initializer_list<bool> IL) : TBitset(Iteration::Begin(IL), Iteration::End(IL)) { }

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

			Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(BlockType));

			return *this;
		}

		check(InValue.Num() <= Max());

		Impl.BitsetNum = InValue.Num();

		Memory::Memcpy(Impl.Pointer, InValue.Impl.Pointer, NumBlocks() * sizeof(BlockType));

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
		auto First = Iteration::Begin(IL);

		const size_t BlocksCount = (GetNum(IL) + BlockWidth - 1) / BlockWidth;

		size_t NumToAllocate = BlocksCount;

		NumToAllocate = NumToAllocate > MaxBlocks() ? Impl->CalculateSlackGrow(BlocksCount, MaxBlocks())   : NumToAllocate;
		NumToAllocate = NumToAllocate < MaxBlocks() ? Impl->CalculateSlackShrink(BlocksCount, MaxBlocks()) : NumToAllocate;

		if (NumToAllocate != MaxBlocks())
		{
			Impl->Deallocate(Impl.Pointer);

			Impl.BitsetNum = GetNum(IL);
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			for (Reference Ref : *this) Ref = *First++;

			return *this;
		}

		Impl.BitsetNum = GetNum(IL);

		for (Reference Ref : *this) Ref = *First++;

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

		const BlockType LastBlockBitmask = LHS.Num() % BlockWidth != 0 ? (1ull << LHS.Num() % BlockWidth) - 1 : -1;

		return (LHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask) == (RHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask);
	}

	/** Compares the bits of two bitsets. */
	NODISCARD friend strong_ordering operator<=>(const TBitset& LHS, const TBitset& RHS)
	{
		if (LHS.Num() < RHS.Num()) return strong_ordering::less;
		if (LHS.Num() > RHS.Num()) return strong_ordering::greater;

		if (LHS.NumBlocks() == 0) return strong_ordering::equivalent;

		for (size_t Index = 0; Index != LHS.NumBlocks() - 1; ++Index)
		{
			strong_ordering Ordering = LHS.Impl.Pointer[Index] <=> RHS.Impl.Pointer[Index];

			if (Ordering != strong_ordering::equivalent) return Ordering;
		}

		const BlockType LastBlockBitmask = LHS.Num() % BlockWidth != 0 ? (1ull << LHS.Num() % BlockWidth) - 1 : -1;

		return (LHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask) <=> (RHS.Impl.Pointer[LHS.NumBlocks() - 1] & LastBlockBitmask);
	}

	/** Sets the bits to the result of binary AND on corresponding pairs of bits of *this and other. */
	TBitset& operator&=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0 || InValue.Num() == 0) return *this;

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

			const BlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;
			
			Impl.Pointer[LastBlock] &= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;

			for (size_t Index = LastBlock + 1; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] &= 0;
			}
		}

		return *this;
	}

	/** Sets the bits to the result of binary OR on corresponding pairs of bits of *this and other. */
	TBitset& operator|=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0 || InValue.Num() == 0) return *this;

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

			const BlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;

			Impl.Pointer[LastBlock] |= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;

			for (size_t Index = LastBlock + 1; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] |= 0;
			}
		}

		return *this;
	}

	/** Sets the bits to the result of binary XOR on corresponding pairs of bits of *this and other. */
	TBitset& operator^=(const TBitset& InValue)
	{
		if (&InValue == this) UNLIKELY return *this;

		if (Num() == 0 || InValue.Num() == 0) return *this;

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

			const BlockType LastBlockBitmask = InValue.Num() % BlockWidth != 0 ? (1ull << InValue.Num() % BlockWidth) - 1 : -1;

			Impl.Pointer[LastBlock] ^= InValue.Impl.Pointer[LastBlock] & LastBlockBitmask;

			for (size_t Index = LastBlock + 1; Index != NumBlocks(); ++Index)
			{
				Impl.Pointer[Index] ^= 0;
			}
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

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

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

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		return (Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask) != 0;
	}

	/** @return true if none of the bits are set to true, otherwise false. */
	NODISCARD FORCEINLINE bool None() const { return !Any(); }

	/** @return The number of bits that are set to true. */
	NODISCARD size_t Count() const
	{
		if (Num() == 0) return 0;

		size_t Result = 0;

		static constexpr auto BlockCount = [](BlockType Block)
		{
			static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");

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
			Result += BlockCount(Impl.Pointer[Index]);
		}

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;

		Result += BlockCount(Impl.Pointer[NumBlocks() - 1] & LastBlockBitmask);

		return Result;
	}

	/** Sets all bits to true. */
	TBitset& Set(bool InValue = true)
	{
		Memory::Memset(Impl.Pointer, static_cast<uint8>(InValue ? -1 : 0), NumBlocks() * sizeof(BlockType));

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
		checkf(Num() <= 64, TEXT("The bitset can not be represented in uint64. Please check Num()."));

		uint64 Result = 0;

		static_assert(sizeof(BlockType) <= sizeof(uint64), "The block width of TBitset is unexpected");
		
		if constexpr (sizeof(BlockType) == sizeof(uint8))
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
		else if constexpr (sizeof(BlockType) == sizeof(uint16))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
			Result |= static_cast<uint64>(Impl.Pointer[1]) << 16;
			Result |= static_cast<uint64>(Impl.Pointer[2]) << 32;
			Result |= static_cast<uint64>(Impl.Pointer[3]) << 48;
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint32))
		{
			Result |= static_cast<uint64>(Impl.Pointer[0]) <<  0;
			Result |= static_cast<uint64>(Impl.Pointer[1]) << 32;
		}
		else if constexpr (sizeof(BlockType) == sizeof(uint64))
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
			BlockType* OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = NumBlocks();

			Impl.BitsetNum = InCount;
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			if (NumToDestruct <= Num())
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, NumToDestruct * sizeof(BlockType));
			}
			else
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, BlocksCount * sizeof(BlockType));
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

		const BlockType LastBlockBitmask = Num() % BlockWidth != 0 ? (1ull << Num() % BlockWidth) - 1 : -1;
		const BlockType BlocksValueToSet = static_cast<BlockType>(InValue ? -1 : 0);

		if (NumToAllocate != MaxBlocks())
		{
			BlockType*   OldAllocation = Impl.Pointer;
			const size_t NumToDestruct = NumBlocks();

			Impl.BitsetNum = InCount;
			Impl.BlocksMax = NumToAllocate;
			Impl.Pointer   = Impl->Allocate(MaxBlocks());

			if (NumToDestruct <= NumBlocks())
			{
				if (NumToDestruct != 0)
				{
					Memory::Memcpy(Impl.Pointer, OldAllocation, (NumToDestruct - 1) * sizeof(BlockType));

					Impl.Pointer[NumToDestruct - 1] = OldAllocation[NumToDestruct - 1] & LastBlockBitmask | BlocksValueToSet & ~LastBlockBitmask;
				}

				Memory::Memset(Impl.Pointer + NumToDestruct, static_cast<uint8>(BlocksValueToSet), (BlocksCount - NumToDestruct) * sizeof(BlockType));
			}
			else
			{
				Memory::Memcpy(Impl.Pointer, OldAllocation, BlocksCount * sizeof(BlockType));
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

			Memory::Memset(Impl.Pointer + NumBlocks(), static_cast<uint8>(BlocksValueToSet), (BlocksCount - NumBlocks()) * sizeof(BlockType));
		}

		Impl.BitsetNum = InCount;
	}

	/** Increase the max capacity of the bitset to a value that's greater or equal to 'InCount'. */
	void Reserve(size_t InCount)
	{
		if (InCount <= Max()) return;

		const size_t BlocksCount = (InCount + BlockWidth - 1) / BlockWidth;

		const size_t NumToAllocate = Impl->CalculateSlackReserve(BlocksCount);
		BlockType*   OldAllocation = Impl.Pointer;

		check(NumToAllocate > MaxBlocks());

		Impl.BlocksMax = NumToAllocate;
		Impl.Pointer  = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, OldAllocation, NumBlocks() * sizeof(BlockType));

		Impl->Deallocate(OldAllocation);
	}

	/** Requests the removal of unused capacity. */
	void Shrink()
	{
		size_t NumToAllocate = Impl->CalculateSlackReserve(NumBlocks());

		check(NumToAllocate <= MaxBlocks());

		if (NumToAllocate == MaxBlocks()) return;

		BlockType* OldAllocation = Impl.Pointer;

		Impl.BitsetNum = NumToAllocate * BlockWidth;
		Impl.Pointer   = Impl->Allocate(MaxBlocks());

		Memory::Memcpy(Impl.Pointer, OldAllocation, NumBlocks() * sizeof(BlockType));

		Impl->Deallocate(OldAllocation);
	}

	/** @return The pointer to the underlying element storage. */
	NODISCARD FORCEINLINE TObserverPtr<      BlockType[]> GetData()       { return TObserverPtr<      BlockType[]>(Impl.Pointer); }
	NODISCARD FORCEINLINE TObserverPtr<const BlockType[]> GetData() const { return TObserverPtr<const BlockType[]>(Impl.Pointer); }

	/** @return The iterator to the first or end bit. */
	NODISCARD FORCEINLINE      Iterator Begin()       { return      Iterator(this, Impl.Pointer, 0);     }
	NODISCARD FORCEINLINE ConstIterator Begin() const { return ConstIterator(this, Impl.Pointer, 0);     }
	NODISCARD FORCEINLINE      Iterator End()         { return      Iterator(this, Impl.Pointer, Num()); }
	NODISCARD FORCEINLINE ConstIterator End()   const { return ConstIterator(this, Impl.Pointer, Num()); }

	/** @return The reverse iterator to the first or end bit. */
	NODISCARD FORCEINLINE      ReverseIterator RBegin()       { return      ReverseIterator(End());   }
	NODISCARD FORCEINLINE ConstReverseIterator RBegin() const { return ConstReverseIterator(End());   }
	NODISCARD FORCEINLINE      ReverseIterator REnd()         { return      ReverseIterator(Begin()); }
	NODISCARD FORCEINLINE ConstReverseIterator REnd()   const { return ConstReverseIterator(Begin()); }

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
	NODISCARD FORCEINLINE bool IsValidIterator(ConstIterator Iter) const { return Begin() <= Iter && Iter <= End(); }

	/** @return The reference to the requested bit. */
	NODISCARD FORCEINLINE      Reference operator[](size_t Index)       { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }
	NODISCARD FORCEINLINE ConstReference operator[](size_t Index) const { checkf(Index < Num(), TEXT("Read access violation. Please check IsValidIterator().")); return *(Begin() + Index); }

	/** @return The reference to the first or last bit. */
	NODISCARD FORCEINLINE      Reference Front()       { return *Begin();     }
	NODISCARD FORCEINLINE ConstReference Front() const { return *Begin();     }
	NODISCARD FORCEINLINE      Reference Back() { return *(End() - 1); }
	NODISCARD FORCEINLINE ConstReference Back()  const { return *(End() - 1); }

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

		const BlockType LastBlockBitmask = A.Num() % BlockWidth != 0 ? (1ull << A.Num() % BlockWidth) - 1 : -1;

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

	ALLOCATOR_WRAPPER_BEGIN(AllocatorType, BlockType, Impl)
	{
		size_t BitsetNum;
		size_t BlocksMax;
		BlockType* Pointer;
	}
	ALLOCATOR_WRAPPER_END(AllocatorType, BlockType, Impl)

public:

	class Reference final : private FSingleton
	{
	public:

		FORCEINLINE Reference& operator=(bool InValue) { Data = (Data & ~Mask) | (InValue ? Mask : 0); return *this; }

		FORCEINLINE Reference& operator=(const Reference& InValue) { return *this = static_cast<bool>(InValue); }

		FORCEINLINE Reference& operator&=(bool InValue) { Data &= InValue ? -1 : ~Mask; return *this; }
		FORCEINLINE Reference& operator|=(bool InValue) { Data |= InValue ? Mask :   0; return *this; }
		FORCEINLINE Reference& operator^=(bool InValue) { *this = InValue ^ *this;      return *this; }

		FORCEINLINE bool operator~() const { return !*this; }

		FORCEINLINE operator bool() const { return (Data & Mask) != 0; }

	private:

		FORCEINLINE Reference(BlockType& InData, BlockType InMask)
			: Data(InData), Mask(InMask)
		{ }

		BlockType& Data;
		BlockType  Mask;

		friend Iterator;

	};

private:

	template <bool bConst>
	class IteratorImpl
	{
	private:

	public:

		using ElementType = TConditional<bConst, const bool, bool>;

		FORCEINLINE IteratorImpl() = default;

#		if DO_CHECK
		FORCEINLINE IteratorImpl(const IteratorImpl<false> &InValue) requires (bConst)
			: Owner(InValue.Owner), Pointer(InValue.Pointer), Offset(InValue.Offset)
		{ }
#		else
		FORCEINLINE IteratorImpl(const IteratorImpl<false> &InValue) requires (bConst)
			: Pointer(InValue.Pointer), Offset(InValue.Offset)
		{ }
#		endif

		FORCEINLINE IteratorImpl(const IteratorImpl&)            = default;
		FORCEINLINE IteratorImpl(IteratorImpl&&)                 = default;
		FORCEINLINE IteratorImpl& operator=(const IteratorImpl&) = default;
		FORCEINLINE IteratorImpl& operator=(IteratorImpl&&)      = default;

		NODISCARD friend FORCEINLINE bool operator==(const IteratorImpl& LHS, const IteratorImpl& RHS) { return LHS.Pointer == RHS.Pointer && LHS.Offset == RHS.Offset; }

		NODISCARD friend FORCEINLINE strong_ordering operator<=>(const IteratorImpl& LHS, const IteratorImpl& RHS) { return 0 <=> LHS.Offset - RHS.Offset; }

		NODISCARD FORCEINLINE      Reference operator*() const requires (!bConst) { CheckThis(true); return Reference(*(Pointer + Offset / BlockWidth), 1ull << Offset % BlockWidth); }
		NODISCARD FORCEINLINE ConstReference operator*() const requires ( bConst) { CheckThis(true); return       (*(Pointer + Offset / BlockWidth) & (1ull << Offset % BlockWidth)); }

		NODISCARD FORCEINLINE auto operator[](ptrdiff Index) const { IteratorImpl Temp = *this + Index; return *Temp; }

		FORCEINLINE IteratorImpl& operator++() { ++Offset; CheckThis(); return *this; }
		FORCEINLINE IteratorImpl& operator--() { --Offset; CheckThis(); return *this; }

		FORCEINLINE IteratorImpl operator++(int) { IteratorImpl Temp = *this; ++*this; return Temp; }
		FORCEINLINE IteratorImpl operator--(int) { IteratorImpl Temp = *this; --*this; return Temp; }

		FORCEINLINE IteratorImpl& operator+=(ptrdiff Offset) { this->Offset += Offset; CheckThis(); return *this; }
		FORCEINLINE IteratorImpl& operator-=(ptrdiff Offset) { this->Offset -= Offset; CheckThis(); return *this; }

		NODISCARD friend FORCEINLINE IteratorImpl operator+(IteratorImpl Iter, ptrdiff Offset) { IteratorImpl Temp = Iter; Temp += Offset; return Temp; }
		NODISCARD friend FORCEINLINE IteratorImpl operator+(ptrdiff Offset, IteratorImpl Iter) { IteratorImpl Temp = Iter; Temp += Offset; return Temp; }

		NODISCARD FORCEINLINE IteratorImpl operator-(ptrdiff Offset) const { IteratorImpl Temp = *this; Temp -= Offset; return Temp; }

		NODISCARD friend FORCEINLINE ptrdiff operator-(const IteratorImpl& LHS, const IteratorImpl& RHS) { return (reinterpret_cast<uintptr>(LHS.Pointer) * BlockWidth + LHS.Offset) - (reinterpret_cast<uintptr>(RHS.Pointer) * BlockWidth + RHS.Offset); }

	private:

#		if DO_CHECK
		const TBitset* Owner = nullptr;
#		endif

		BlockType* Pointer = nullptr;
		size_t     Offset = 0;

#		if DO_CHECK
		FORCEINLINE IteratorImpl(const TBitset* InContainer, BlockType* InPointer, size_t InOffset)
			: Owner(InContainer), Pointer(InPointer), Offset(InOffset)
		{ }
#		else
		FORCEINLINE IteratorImpl(const TBitset* InContainer, BlockType* InPointer, size_t InOffset)
			: Pointer(InPointer), Offset(InOffset)
		{ }
#		endif

		FORCEINLINE void CheckThis(bool bExceptEnd = false) const
		{
			checkf(Owner && Owner->IsValidIterator(*this), TEXT("Read access violation. Please check IsValidIterator()."));
			checkf(!(bExceptEnd && Owner->End() == *this), TEXT("Read access violation. Please check IsValidIterator()."));
		}

		template <bool> friend class IteratorImpl;

		friend TBitset;

	};

};

using FBitset = TBitset<uint64>;

static_assert(sizeof(FBitset) == 40, "The byte size of FBitset is unexpected");

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
