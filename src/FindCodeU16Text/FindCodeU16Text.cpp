#include <sdw.h>

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

int UMain(int argc, UChar* argv[])
{
	if (argc != 7)
	{
		return 1;
	}
	n32 nFindMethod = SToN32(argv[3]);
	if (nFindMethod != 1 && nFindMethod != 2)
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
	if (nFindMethod == 1)
	{
		for (u32 i = 0; i < uCodeSize / 4 * 4; i += 4)
		{
			u32 uRamOffset = *reinterpret_cast<u32*>(pCode + i);
			if (uRamOffset % 2 == 0 && uRamOffset >= 0x100000 && uRamOffset < 0x100000 + uCodeSize)
			{
				U16String sTxt16 = reinterpret_cast<Char16_t*>(pCode + uRamOffset - 0x100000);
				wstring sTxt;
				try
				{
					sTxt = U16ToW(sTxt16);
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
						mOffsetSize.insert(make_pair(i, sTxt16.size() * 2));
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
			if (uRamOffset % 2 == 0 && uRamOffset >= 0x100000 && uRamOffset < 0x100000 + uCodeSize)
			{
				U16String sTxt16 = reinterpret_cast<Char16_t*>(pCode + uRamOffset - 0x100000);
				wstring sTxt;
				try
				{
					sTxt = U16ToW(sTxt16);
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
						mOffsetSize.insert(make_pair(i, sTxt16.size() * 2));
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
