#include "pch.h"

bool FURL::HasOption(const TCHAR* Test) const
{
	return GetOption(Test, nullptr) != nullptr;
}

const TCHAR* FURL::GetOption(const TCHAR* Match, const TCHAR* Default) const
{
	const int32 Len = static_cast<int32>(wcslen(Match));

	if (Len > 0)
	{
		for (int32 i = 0; i < Op.Num(); i++)
		{
			const TCHAR* s = Op[i].CStr();

			if (s && _wcsnicmp(s, Match, Len) == 0)
			{
				if (s[Len - 1] == '=' || s[Len] == '=' || s[Len] == '\0')
					return s + Len;
			}
		}
	}

	return Default;
}