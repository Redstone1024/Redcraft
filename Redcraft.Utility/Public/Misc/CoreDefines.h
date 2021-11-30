#pragma once

// Define the normal namespace
#define NS_BEGIN(Name)				namespace Name {
#define NS_END(Name)				}
#define NS_USING(Name)				using namespace Name;

// Define the redcraft master namespace
#define NS_REDCRAFT					RFur
#define NS_REDCRAFT_BEGIN			NS_BEGIN(NS_REDCRAFT)
#define NS_REDCRAFT_END				NS_END(NS_REDCRAFT)
#define NS_REDCRAFT_USING			NS_USING(NS_REDCRAFT)

// Define the private namespace - Used to hide the realization of something
#define NS_PRIVATE					Private
#define NS_PRIVATE_BEGIN			NS_BEGIN(NS_PRIVATE)
#define NS_PRIVATE_END				NS_END(NS_PRIVATE)

// Define the STD namespace
#define NS_STD_BEGIN				NS_BEGIN(std)
#define NS_STD_END					NS_END(std)
#define NS_STD_USING				NS_USING(std)

// Define the unnamed namespace
#define NS_UNNAMED_BEGIN			namespace {
#define NS_UNNAMED_END				}

// Create an alias for the namespace - like typedef
#define NS_DEFINE(Source, Target)	NS_BEGIN(Target) NS_USING(Source) NS_END(Target)

NS_REDCRAFT_BEGIN

enum { INDEX_NONE = -1 };
enum { UNICODE_BOM = 0xfeff };

enum EForceInit { ForceInit };

NS_REDCRAFT_END
