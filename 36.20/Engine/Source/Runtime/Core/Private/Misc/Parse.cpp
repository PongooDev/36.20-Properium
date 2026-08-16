#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Parse.h"

#include <cwctype>

static TCHAR ParseToUpper(TCHAR Char)
{
	return static_cast<uint16>(Char - TEXT('a')) <= 25u ? TCHAR(Char - 32) : Char;
}

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
	if (!Stream || !*Stream || !Param || !*Param)
		return false;

	const TCHAR* const Start = Stream;
	const int32 ParamLen = static_cast<int32>(wcslen(Param));
	const TCHAR ParamFirst = ParseToUpper(*Param);

	while (Stream)
	{
		bool bPrevAlnum = false;
		bool bInQuote = false;

		const TCHAR* Cursor = Stream;
		TCHAR Char = *Cursor++;
		bool bMatched = false;

		while (Char)
		{
			const TCHAR Upper = ParseToUpper(Char);

			if (!bInQuote && !bPrevAlnum && Upper == ParamFirst
				&& _wcsnicmp(Cursor, Param + 1, static_cast<size_t>(ParamLen) - 1) == 0)
			{
				bMatched = true;
				break;
			}

			bPrevAlnum = static_cast<uint16>(Upper - TEXT('A')) <= 25u
				|| static_cast<uint16>(Upper - TEXT('0')) <= 9u;

			if (Upper == TEXT('"'))
				bInQuote = !bInQuote;

			Char = *Cursor++;
		}

		if (!bMatched)
			return false;

		const TCHAR* const Match = Cursor - 1;

		if (Match > Start && (*(Cursor - 2) == TEXT('-') || *(Cursor - 2) == TEXT('/'))
			&& (Start > Cursor - 3 || iswspace(*(Cursor - 3))))
		{
			const TCHAR* const End = Match + ParamLen;

			if (!*End || iswspace(*End))
				return true;
		}

		Stream = Cursor;
	}

	return false;
}
