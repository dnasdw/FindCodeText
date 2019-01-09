#include <sdw.h>

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

int UMain(int argc, UChar* argv[])
{
	if (argc != 6)
	{
		return 1;
	}
	n32 nMethod = SToN32(argv[3]);
	if (nMethod != 1 && nMethod != 2)
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
	if (nMethod == 1)
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
	else if (nMethod == 2)
	{
		for (u32 i = 0; i < uCodeSize / 4 * 4; i += 4)
		{
			u32 uIns = *reinterpret_cast<u32*>(pCode + i);
			// ADR
			if ((uIns & 0xFFF0000) != 0x28F0000)
			{
				continue;
			}
			u32 uImm12 = uIns & 0xFFF;
			u32 uUnrotatedValue = uImm12 & 0xFF;
			u32 uRoRBits = (uImm12 >> 8 & 0xF) * 2;
			// PC + 8
			u32 uRamOffset = i + 0x100000 + 8;
			uRamOffset += (uUnrotatedValue >> uRoRBits) | (uUnrotatedValue << (32 - uRoRBits));
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
	for (map<u32, u32>::iterator it = mOffsetSize.begin(); it != mOffsetSize.end(); ++it)
	{
		wstring sTxt = Replace(mOffsetText[it->first], L'\r', L"");
		sTxt = Replace(sTxt, L'\n', L"");
		fu16printf(fp, L"%X,%X,%u,%ls\r\n", it->first, mOffsetAddress[it->first], it->second, sTxt.c_str());
	}
	fclose(fp);
	return 0;
}
