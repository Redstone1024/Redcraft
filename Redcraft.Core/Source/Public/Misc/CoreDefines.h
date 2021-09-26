#pragma once

#define NS_REDCRAFT			RFur
#define NS_REDCRAFT_BEGIN	namespace NS_REDCRAFT {
#define NS_REDCRAFT_END		}
#define NS_REDCRAFT_USING	using namespace NS_REDCRAFT;

#define NS_STD_BEGIN	namespace std {
#define NS_STD_END		}
#define NS_STD_USING	using namespace std;

NS_REDCRAFT_BEGIN

enum { INDEX_NONE  =     -1 };
enum { UNICODE_BOM = 0xfeff };

enum EForceInit { ForceInit };

NS_REDCRAFT_END
