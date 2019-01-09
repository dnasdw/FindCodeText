#include <sdw.h>

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

bool isValidUTF8(const u8* a_pCode, u32 a_uCodeSize, u32* a_pSize = nullptr)
{
	u32 uSize = 0;
	for (u32 i = 0; i < a_uCodeSize; i++)
	{
		const u8* pUTF8 = a_pCode + i;
		if (pUTF8[0] == 0)
		{
			uSize = i;
			if (a_pSize != nullptr)
			{
				*a_pSize = uSize;
			}
			return uSize != 0;
		}
		else if (pUTF8[0] >= 0xE0 && pUTF8[0] <= 0xEF)
		{
			if (i + 3 >= a_uCodeSize || pUTF8[1] < 0x80 || pUTF8[1] > 0xBF || pUTF8[2] < 0x80 || pUTF8[2] > 0xBF)
			{
				return false;
			}
			i += 2;
		}
		else if (pUTF8[0] >= 0xC0 && pUTF8[0] <= 0xDF)
		{
			if (i + 2 >= a_uCodeSize || pUTF8[1] < 0x80 || pUTF8[1] > 0xBF)
			{
				return false;
			}
			i++;
		}
		else if (pUTF8[0] >= 0x80)
		{
			return false;
		}
		else if (pUTF8[0] < 0x20 && pUTF8[0] != 0x0D && pUTF8[0] != 0x0A)
		{
			return false;
		}
	}
	return false;
}

int UMain(int argc, UChar* argv[])
{
	if (argc != 6)
	{
		return 1;
	}
	n32 nMethod = SToN32(argv[3]);
	if (nMethod != 0 && nMethod != 1)
	{
		return 1;
	}
	bool bIncludeEmpty = UCscmp(argv[4], USTR("0")) != 0;
	bool bIncludeASCIIOnly = UCscmp(argv[5], USTR("0")) != 0;
	FILE* fp = UFopen(argv[1], USTR("rb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	u32 uCodeSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	u8* pCode = new u8[uCodeSize];
	fread(pCode, 1, uCodeSize, fp);
	fclose(fp);
	map<u32, u32> mOffsetAddress;
	map<u32, u32> mOffsetSize;
	map<u32, wstring> mOffsetText;
	if (nMethod == 0)
	{
		for (u32 i = 0; i < uCodeSize; i++)
		{
			u32 uSize = 0;
			if (isValidUTF8(pCode + i, uCodeSize - i, &uSize))
			{
				wstring sTxt = U8ToW(reinterpret_cast<char*>(pCode + i));
				bool bEmpty = sTxt.empty();
				if (bIncludeEmpty || !bEmpty)
				{
					bool bASCII = count_if(sTxt.begin(), sTxt.end(), isASCII) == sTxt.size();
					if (bIncludeASCIIOnly || !bASCII)
					{
						mOffsetAddress.insert(make_pair(i, i));
						mOffsetSize.insert(make_pair(i, uSize));
						mOffsetText.insert(make_pair(i, sTxt));
						i += uSize;
					}
				}
			}
		}
	}
	else if (nMethod == 1)
	{
		for (u32 i = 0; i < uCodeSize / 4 * 4; i += 4)
		{
			u32 uRamOffset = *reinterpret_cast<u32*>(pCode + i);
			if (uRamOffset % 2 == 0 && uRamOffset >= 0x100000 && uRamOffset < 0x100000 + uCodeSize)
			{
				string sTxtU8 = reinterpret_cast<char*>(pCode + uRamOffset - 0x100000);
				wstring sTxt;
				try
				{
					sTxt = U8ToW(sTxtU8);
				}
				catch (...)
				{
					continue;
				}
				bool bEmpty = sTxt.empty();
				if (bIncludeEmpty || !bEmpty)
				{
					bool bASCII = count_if(sTxt.begin(), sTxt.end(), isASCII) == sTxt.size();
					if (bIncludeASCIIOnly || !bASCII)
					{
						mOffsetAddress.insert(make_pair(i, uRamOffset));
						mOffsetSize.insert(make_pair(i, sTxtU8.size()));
						mOffsetText.insert(make_pair(i, sTxt));
					}
				}
			}
		}
	}
	delete[] pCode;
	fp = UFopen(argv[2], USTR("wb"), false);
	if (fp == nullptr)
	{
		return 1;
	}
	fwrite("\xFF\xFE", 2, 1, fp);
	for (map<u32, u32>::iterator it = mOffsetSize.begin(); it != mOffsetSize.end(); ++it)
	{
		wstring sTxt = Replace(mOffsetText[it->first], L'\r', L"");
		sTxt = Replace(sTxt, L'\n', L"");
		fu16printf(fp, L"%X,%X,%u,%ls\r\n", it->first, mOffsetAddress[it->first], it->second, sTxt.c_str());
	}
	fclose(fp);
	return 0;
}