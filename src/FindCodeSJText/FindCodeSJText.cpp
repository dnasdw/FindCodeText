#include <sdw.h>

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

bool isValidSJIS(const u8* a_pCode, u32 a_uCodeSize, u32* a_pSize, bool a_bEmptyIsValid)
{
	u32 uSize = 0;
	for (u32 i = 0; i < a_uCodeSize; i++)
	{
		const u8* pSJIS = a_pCode + i;
		if (pSJIS[0] == 0)
		{
			uSize = i;
			if (a_pSize != nullptr)
			{
				*a_pSize = uSize;
			}
			return a_bEmptyIsValid || uSize != 0;
		}
		else if (pSJIS[0] > 0xFC)
		{
			return false;
		}
		else if (pSJIS[0] == 0xFC)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] > 0x4B)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0xFA && pSJIS[0] <= 0xFB)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0xF0 && pSJIS[0] <= 0xF9)
		{
			return false;
		}
		else if (pSJIS[0] == 0xEF)
		{
			return false;
		}
		else if (pSJIS[0] == 0xEE)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] == 0xED || pSJIS[1] == 0xEE || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0xED)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0xEB || pSJIS[0] == 0xEC)
		{
			return false;
		}
		else if (pSJIS[0] == 0xEA)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xA4)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0xE0 && pSJIS[0] <= 0xE9)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0xA1 && pSJIS[0] <= 0xDF)
		{
			if (i + 1 >= a_uCodeSize)
			{
				return false;
			}
		}
		else if (pSJIS[0] == 0xA0)
		{
			return false;
		}
		else if (pSJIS[0] >= 0x99 && pSJIS[0] <= 0x9F)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x98)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || (pSJIS[1] >= 0x73 && pSJIS[1] <= 0x9E) || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0x89 && pSJIS[0] <= 0x97)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x88)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x9F || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x87)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x5E || (pSJIS[1] >= 0x76 && pSJIS[1] <= 0x7D) || pSJIS[1] == 0x7F || pSJIS[1] > 0x9C)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x85 || pSJIS[0] == 0x86)
		{
			return false;
		}
		else if (pSJIS[0] == 0x84)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || (pSJIS[1] >= 0x61 && pSJIS[1] <= 0x6F) || pSJIS[1] == 0x7F || (pSJIS[1] >= 0x92 && pSJIS[1] <= 0x9E) || pSJIS[1] > 0xBE)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x83)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || (pSJIS[1] >= 0x97 && pSJIS[1] <= 0x9E) || (pSJIS[1] >= 0xB7 && pSJIS[1] <= 0xBE) || pSJIS[1] > 0xD6)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x82)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x4F || (pSJIS[1] >= 0x59 && pSJIS[1] <= 0x5F) || (pSJIS[1] >= 0x7A && pSJIS[1] <= 0x80) || (pSJIS[1] >= 0x9B && pSJIS[1] <= 0x9E) || pSJIS[1] > 0xF1)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] == 0x81)
		{
			if (i + 2 >= a_uCodeSize || pSJIS[1] < 0x40 || pSJIS[1] == 0x7F || (pSJIS[1] >= 0xAD && pSJIS[1] <= 0xB7) || (pSJIS[1] >= 0xC0 && pSJIS[1] <= 0xC7) || (pSJIS[1] >= 0xCF && pSJIS[1] <= 0xD9) || (pSJIS[1] >= 0xE9 && pSJIS[1] <= 0xEF) || (pSJIS[1] >= 0xF8 && pSJIS[1] <= 0xFB) || pSJIS[1] > 0xFC)
			{
				return false;
			}
			i++;
		}
		else if (pSJIS[0] >= 0x80)
		{
			return false;
		}
		else if (pSJIS[0] < 0x20 && pSJIS[0] != 0x0D && pSJIS[0] != 0x0A)
		{
			return false;
		}
	}
	return false;
}

int UMain(int argc, UChar* argv[])
{
	if (argc != 7)
	{
		return 1;
	}
	n32 nFindMethod = SToN32(argv[3]);
	if (nFindMethod != 0 && nFindMethod != 1 && nFindMethod != 2)
	{
		return 1;
	}
	n32 nOutputMethod = SToN32(argv[4]);
	if (nOutputMethod != 0 && nOutputMethod != 1)
	{
		return 1;
	}
	bool bIncludeEmpty = UCscmp(argv[5], USTR("0")) != 0;
	bool bIncludeASCIIOnly = UCscmp(argv[6], USTR("0")) != 0;
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
	if (nFindMethod == 0)
	{
		for (u32 i = 0; i < uCodeSize; i++)
		{
			u32 uSize = 0;
			if (isValidSJIS(pCode + i, uCodeSize - i, &uSize, bIncludeEmpty))
			{
				wstring sTxt = XToW(reinterpret_cast<char*>(pCode + i), 932, "CP932");
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
	else if (nFindMethod == 1)
	{
		for (u32 i = 0; i < uCodeSize / 4 * 4; i += 4)
		{
			u32 uRamOffset = *reinterpret_cast<u32*>(pCode + i);
			if (uRamOffset >= 0x100000 && uRamOffset < 0x100000 + uCodeSize && isValidSJIS(pCode + uRamOffset - 0x100000, uCodeSize - (uRamOffset - 0x100000), nullptr, bIncludeEmpty))
			{
				string sTxtA = reinterpret_cast<char*>(pCode + uRamOffset - 0x100000);
				wstring sTxt;
				try
				{
					sTxt = XToW(sTxtA, 932, "CP932");
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
						mOffsetAddress.insert(make_pair(i, uRamOffset - 0x100000));
						mOffsetSize.insert(make_pair(i, sTxtA.size()));
						mOffsetText.insert(make_pair(i, sTxt));
					}
				}
			}
		}
	}
	else if (nFindMethod == 2)
	{
		for (u32 i = 0; i < uCodeSize / 4 * 4; i += 4)
		{
			u32 uIns = *reinterpret_cast<u32*>(pCode + i);
			// ADR
			if ((uIns & 0xFFF0000) != 0x28F0000 && (uIns & 0xFFF0000) != 0x24F0000)
			{
				continue;
			}
			u32 uImm12 = uIns & 0xFFF;
			u32 uUnrotatedValue = uImm12 & 0xFF;
			u32 uRoRBits = (uImm12 >> 8 & 0xF) * 2;
			// PC + 8
			u32 uRamOffset = i + 0x100000 + 8;
			if ((uIns & 0xFFF0000) == 0x28F0000)
			{
				uRamOffset += (uUnrotatedValue >> uRoRBits) | (uUnrotatedValue << (32 - uRoRBits));
			}
			else if ((uIns & 0xFFF0000) == 0x24F0000)
			{
				uRamOffset -= (uUnrotatedValue >> uRoRBits) | (uUnrotatedValue << (32 - uRoRBits));
			}
			if (uRamOffset >= 0x100000 && uRamOffset < 0x100000 + uCodeSize && isValidSJIS(pCode + uRamOffset - 0x100000, uCodeSize - (uRamOffset - 0x100000), nullptr, bIncludeEmpty))
			{
				string sTxtA = reinterpret_cast<char*>(pCode + uRamOffset - 0x100000);
				wstring sTxt;
				try
				{
					sTxt = XToW(sTxtA, 932, "CP932");
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
						mOffsetAddress.insert(make_pair(i, uRamOffset - 0x100000));
						mOffsetSize.insert(make_pair(i, sTxtA.size()));
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
	if (nOutputMethod == 0)
	{
		for (map<u32, u32>::iterator it = mOffsetSize.begin(); it != mOffsetSize.end(); ++it)
		{
			wstring sTxt = Replace(mOffsetText[it->first], L'\r', L"");
			sTxt = Replace(sTxt, L'\n', L"");
			fu16printf(fp, L"%X,%X,%u,%ls\r\n", it->first, mOffsetAddress[it->first], it->second, sTxt.c_str());
		}
	}
	else if (nOutputMethod == 1)
	{
		map<u32, u32> mAddressOffset;
		for (map<u32, u32>::iterator it = mOffsetAddress.begin(); it != mOffsetAddress.end(); ++it)
		{
			mAddressOffset.insert(make_pair(it->second, it->first));
		}
		for (map<u32, u32>::iterator it = mAddressOffset.begin(); it != mAddressOffset.end(); ++it)
		{
			u32 uOffset = it->second;
			wstring sTxt = Replace(mOffsetText[uOffset], L'\r', L"");
			sTxt = Replace(sTxt, L'\n', L"");
			fu16printf(fp, L"%X,%X,%u,%ls\r\n", uOffset, it->first, mOffsetSize[uOffset], sTxt.c_str());
		}
	}
	fclose(fp);
	return 0;
}
