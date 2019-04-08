#include <sdw.h>

#include SDW_MSC_PUSH_PACKED
struct NsoHeader
{
	u32 Signature;
	u32 Version;
	u32 Reserved1;
	u32 Flags;
	u32 TextFileOffset;
	u32 TextMemoryOffset;
	u32 TextSize;
	u32 ModuleNameOffset;
	u32 RoFileOffset;
	u32 RoMemoryOffset;
	u32 RoSize;
	u32 ModuleNameSize;
	u32 DataFileOffset;
	u32 DataMemoryOffset;
	u32 DataSize;
	u32 BssSize;
	u8 ModuleId[32];
	u32 TextFileSize;
	u32 RoFileSize;
	u32 DataFileSize;
	u8 Reserved2[4];
	u32 EmbededOffset;
	u32 EmbededSize;
	u8 Reserved3[40];
	u8 TextHash[32];
	u8 RoHash[32];
	u8 DataHash[32];
} SDW_GNUC_PACKED;
#include SDW_MSC_POP_PACKED

class CNso
{
public:
	enum NsoHeaderFlags
	{
		TextCompress = 1,
		RoCompress = 2,
		DataCompress = 4,
		TextHash = 8,
		RoHash = 16,
		DataHash = 32
	};
};

bool isASCII(wchar_t c)
{
	return c < 0x80;
}

bool isValidUTF8(const u8* a_pCode, u32 a_uCodeSize, u32* a_pSize, bool a_bEmptyIsValid)
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
			return a_bEmptyIsValid || uSize != 0;
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
	NsoHeader* pNsoHeader = reinterpret_cast<NsoHeader*>(pCode);
	if ((pNsoHeader->Flags & (CNso::TextCompress | CNso::RoCompress | CNso::DataCompress)) != 0)
	{
		delete[] pCode;
		return 1;
	}
	u32 uUncompressedSize = static_cast<u32>(Align(pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize, 0x1000));
	u8* pUncompressed = new u8[uUncompressedSize];
	memset(pUncompressed, 0, uUncompressedSize);
	memcpy(pUncompressed + pNsoHeader->TextMemoryOffset, pCode + pNsoHeader->TextFileOffset, pNsoHeader->TextSize);
	memcpy(pUncompressed + pNsoHeader->RoMemoryOffset, pCode + pNsoHeader->RoFileOffset, pNsoHeader->RoSize);
	memcpy(pUncompressed + pNsoHeader->DataMemoryOffset, pCode + pNsoHeader->DataFileOffset, pNsoHeader->DataSize);
	FILE* fpNso = fopen("temp.nso", "wb");
	if (fpNso != nullptr)
	{
		fwrite(pUncompressed, 1, uUncompressedSize, fpNso);
		fclose(fpNso);
	}
	map<u32, u32> mOffsetAddress;
	map<u32, u32> mOffsetSize;
	map<u32, wstring> mOffsetText;
	if (nFindMethod == 0)
	{
		for (u32 i = 0; i < pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize; i++)
		{
			u32 uSize = 0;
			if (isValidUTF8(pUncompressed + i, pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize - i, &uSize, bIncludeEmpty))
			{
				wstring sTxt = U8ToW(reinterpret_cast<char*>(pUncompressed + i));
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
		for (u32 i = 0; i < uUncompressedSize / 8 * 8; i += 8)
		{
			u64 uRamOffset64 = *reinterpret_cast<u64*>(pUncompressed + i);
			u32 uRamOffset = static_cast<u32>(uRamOffset64);
			if (uRamOffset == uRamOffset64 && uRamOffset >= pNsoHeader->TextMemoryOffset && uRamOffset < pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize && isValidUTF8(pUncompressed + uRamOffset, pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize - uRamOffset, nullptr, bIncludeEmpty))
			{
				string sTxtU8 = reinterpret_cast<char*>(pUncompressed + uRamOffset);
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
	else if (nFindMethod == 2)
	{
		set<u32> sRamOffset;
		for (u32 i = pNsoHeader->TextMemoryOffset; i < pNsoHeader->TextMemoryOffset + (pNsoHeader->TextMemoryOffset + pNsoHeader->TextSize - pNsoHeader->TextMemoryOffset) / 4 * 4; i += 4)
		{
			u32 uIns = *reinterpret_cast<u32*>(pUncompressed + i);
			// ADRP
			if ((uIns & 0x9F000000) != 0x90000000)
			{
				continue;
			}
			u32 uX = uIns & 0x1F;
			n32 nImm32 = static_cast<n32>((((uIns >> 5 & 0x7FFFF) << 2) | (uIns >> 29 & 0x3)) << 11) >> 11;
			n64 nImm64 = static_cast<n64>(nImm32) << 12;
			u32 uRamOffset = static_cast<u32>((i & 0xFFFFF000) + nImm64);
			bool bAdd = false;
			u32 uIns2End = i + 4 + 32 * 4;
			if (uIns2End > pNsoHeader->TextMemoryOffset + (pNsoHeader->TextMemoryOffset + pNsoHeader->TextSize - pNsoHeader->TextMemoryOffset) / 4 * 4)
			{
				uIns2End = pNsoHeader->TextMemoryOffset + (pNsoHeader->TextMemoryOffset + pNsoHeader->TextSize - pNsoHeader->TextMemoryOffset) / 4 * 4;
			}
			for (u32 j = i + 4; j < uIns2End; j += 4)
			{
				u32 uIns2 = *reinterpret_cast<u32*>(pUncompressed + j);
				// ADD
				if ((uIns2 & 0xFF8003FF) != (0x91000000 | (uX << 5) | uX))
				{
					continue;
				}
				bAdd = true;
				u32 uShift = uIns2 >> 22 & 0x3;
				u32 uImm12 = uIns2 >> 10 & 0xFFF;
				if (uShift == 0)
				{
					uRamOffset += uImm12;
				}
				else if (uShift == 1)
				{
					uRamOffset += uImm12 << 12;
				}
				if (uRamOffset >= pNsoHeader->TextMemoryOffset && uRamOffset < pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize && isValidUTF8(pUncompressed + uRamOffset, pNsoHeader->DataMemoryOffset + pNsoHeader->DataSize - uRamOffset, nullptr, bIncludeEmpty))
				{
					string sTxtU8 = reinterpret_cast<char*>(pUncompressed + uRamOffset);
					wstring sTxt;
					try
					{
						sTxt = U8ToW(sTxtU8);
					}
					catch (...)
					{
						break;
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
				break;
			}
			if (!bAdd)
			{
				continue;
			}
		}
	}
	delete[] pUncompressed;
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
