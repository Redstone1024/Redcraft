#pragma once

#include "CoreTypes.h"
#include "Miscellaneous/Compare.h"

#include <typeinfo>

NAMESPACE_REDCRAFT_BEGIN
NAMESPACE_MODULE_BEGIN(Redcraft)
NAMESPACE_MODULE_BEGIN(Utility)

struct FTypeInfo
{
	constexpr FTypeInfo() : FTypeInfo(typeid(void)) { }

	constexpr FTypeInfo(FInvalid) : FTypeInfo() { }

	constexpr FTypeInfo(const std::type_info& InTypeInfo) : Ptr(&InTypeInfo) { }

	size_t GetTypeHash() const { return Ptr->hash_code(); }

	const char* GetName() const { return Ptr->name(); }

private:

	const std::type_info* Ptr;

	friend bool operator==(FTypeInfo LHS, FTypeInfo RHS) { return *LHS.Ptr == *RHS.Ptr; }

	friend bool operator<(FTypeInfo LHS, FTypeInfo RHS) { return LHS.Ptr->before(*RHS.Ptr); }

	friend bool operator<=(FTypeInfo LHS, FTypeInfo RHS) { return LHS == RHS || LHS.Ptr->before(*RHS.Ptr); }

	friend bool operator>=(FTypeInfo LHS, FTypeInfo RHS) { return LHS == RHS || !LHS.Ptr->before(*RHS.Ptr); }

	friend bool operator>(FTypeInfo LHS, FTypeInfo RHS) { return !LHS.Ptr->before(*RHS.Ptr); }

	friend strong_ordering operator<=>(FTypeInfo LHS, FTypeInfo RHS)
	{
		if (LHS == RHS) return strong_ordering::equal;
		return LHS < RHS ? strong_ordering::less : strong_ordering::greater;
	}

};

#define Typeid(...) (FTypeInfo(typeid(__VA_ARGS__)))

NAMESPACE_MODULE_END(Utility)
NAMESPACE_MODULE_END(Redcraft)
NAMESPACE_REDCRAFT_END
