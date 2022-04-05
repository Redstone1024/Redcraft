#pragma once

#include "CoreTypes.h"
#include "Miscellaneous/Compare.h"

#include <typeinfo>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FTypeInfo
{
	FTypeInfo()                            = delete; 
	FTypeInfo(FTypeInfo&&)                 = delete;
	FTypeInfo(const FTypeInfo&)            = delete;
	FTypeInfo& operator=(FTypeInfo&&)      = delete;
	FTypeInfo& operator=(const FTypeInfo&) = delete;

	FORCEINLINE size_t  GetTypeHash() const { return reinterpret_cast<const std::type_info*>(this)->hash_code(); }
	FORCEINLINE const char* GetName() const { return reinterpret_cast<const std::type_info*>(this)->name();      }

private:

	friend FORCEINLINE bool operator==(const FTypeInfo& LHS, const FTypeInfo& RHS) { return &LHS != &RHS ? *reinterpret_cast<const std::type_info*>(&LHS) == *reinterpret_cast<const std::type_info*>(&RHS) : true; }
	friend FORCEINLINE bool operator< (const FTypeInfo& LHS, const FTypeInfo& RHS) { return reinterpret_cast<const std::type_info*>(&LHS)->before(*reinterpret_cast<const std::type_info*>(&RHS)); }
	friend FORCEINLINE bool operator<=(const FTypeInfo& LHS, const FTypeInfo& RHS) { return LHS == RHS || LHS < RHS; }
	friend FORCEINLINE bool operator>=(const FTypeInfo& LHS, const FTypeInfo& RHS) { return LHS == RHS || LHS > RHS; }
	friend FORCEINLINE bool operator> (const FTypeInfo& LHS, const FTypeInfo& RHS) { return !(LHS < RHS); }

	friend FORCEINLINE strong_ordering operator<=>(const FTypeInfo& LHS, const FTypeInfo& RHS)
	{
		if (LHS == RHS) return strong_ordering::equal;
		return LHS < RHS ? strong_ordering::less : strong_ordering::greater;
	}

};

#define Typeid(...) (*reinterpret_cast<const FTypeInfo*>(&typeid(__VA_ARGS__)))

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
