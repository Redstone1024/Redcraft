#pragma once

// Define the normal namespace
#define NAMESPACE_BEGIN(Name)			namespace Name {
#define NAMESPACE_END(Name)				}
#define NAMESPACE_USING(Name)			using namespace Name;

// Define the redcraft master namespace
#define NAMESPACE_REDCRAFT				RFur
#define NAMESPACE_REDCRAFT_BEGIN		NAMESPACE_BEGIN(NAMESPACE_REDCRAFT)
#define NAMESPACE_REDCRAFT_END			NAMESPACE_END(NAMESPACE_REDCRAFT)
#define NAMESPACE_REDCRAFT_USING		NAMESPACE_USING(NAMESPACE_REDCRAFT)

// Define the private namespace - Used to hide the realization of something
#define NAMESPACE_PRIVATE				Private
#define NAMESPACE_PRIVATE_BEGIN			NAMESPACE_BEGIN(NAMESPACE_PRIVATE)
#define NAMESPACE_PRIVATE_END			NAMESPACE_END(NAMESPACE_PRIVATE)

// Define the STD namespace
#define NAMESPACE_STD					std
#define NAMESPACE_STD_BEGIN				NAMESPACE_BEGIN(NAMESPACE_STD)
#define NAMESPACE_STD_END				NAMESPACE_END(NAMESPACE_STD)
#define NAMESPACE_STD_USING				NAMESPACE_USING(NAMESPACE_STD)

// Define the unnamed namespace
#define NAMESPACE_UNNAMED_BEGIN			namespace {
#define NAMESPACE_UNNAMED_END			}

// Create an alias for the namespace - like typedef
#define NAMESPACE_DEFINE(Source, Target)	NAMESPACE_BEGIN(Target) NAMESPACE_USING(Source) NAMESPACE_END(Target)

NAMESPACE_REDCRAFT_BEGIN

enum { INDEX_NONE = -1 };
enum { UNICODE_BOM = 0xfeff };

enum EForceInit { ForceInit };

NAMESPACE_REDCRAFT_END
