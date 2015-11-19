

#pragma once

class TypeInfo;
class FieldInfo;

DWORD_PTR FindPattern(DWORD_PTR dwAddress, DWORD_PTR dwLen, DWORD_PTR offset, bool deref, BYTE *bMask, char * szMask);

class ClassInfo
{
public:
	static ClassInfo* GetInstance()
	{
		static ClassInfo** instance = NULL;
		if (!instance)
		{
			DWORD_PTR dwMatch = FindPattern((DWORD_PTR)GetModuleHandle(NULL), -1, 0, false, (BYTE*)"\x48\x8B\x05\x00\x00\x00\x00\x48\x89\x41\x08\x48\x89\x0D\x00\x00\x00\x00\xC3", "xxx????xxxxxxx????x");
			if (!dwMatch)
				return NULL;

			DWORD_PTR dwOffset = *(DWORD*)(dwMatch + 3);

			BYTE* first = (BYTE*)&dwOffset;
			Log("First 0x%x", first[3]);
			if (first[3] == 0xFF)
			{
				Log("Reverse shit");
				dwOffset = dwOffset + 0xFFFFFFFF00000000;
			}

			DWORD_PTR dwOffset2 = (dwMatch + 7);

			instance = (ClassInfo**)(dwOffset + dwOffset2);
			Log("Instance found at 0x%016llX", instance );
		}
		return *instance;
	}

	TypeInfo* typeInfo; //0x0000 
	ClassInfo* next; //0x0008 
	unsigned short id; //0x0010 
	unsigned short isDataContainer; //0x0012 
	char pad_0x0014[0x4]; //0x0014
	ClassInfo* parent; //0x0018 
	char pad_0x0020[0x8]; //0x0020
	unsigned short id3; //0x0028 
	char pad_0x002C[0x94]; //0x002C

};//Size=0x00C0

class TypeInfo
{
public:
	char* name; //0x0000 
	unsigned short flags; //0x0008 
	unsigned short totalSize; //0x000A 
	char pad_0x000C[0x4];
	unsigned short flags2; //0x0010 
	char pad_0x0012[0x6]; //0x0012
	unsigned short alignment; //0x0018 
	unsigned short fieldCount; //0x001A 
	char pad_0x001C[0x4]; //0x001C
	FieldInfo* enumFields; //0x0020 
	FieldInfo* structFields; //0x0028 
	FieldInfo* fields; //0x0030 

};//Size=0x0038

class MemberInfoFlags
{
public:
	unsigned short flagBits;                     // this+0x0
	enum
	{
		kMemberTypeMask = 0x3,                    // constant 0x3
		kTypeCategoryShift = 0x2,                  // constant 0x2
		kTypeCategoryMask = 0x3,                     // constant 0x3
		kTypeCodeShift = 0x4,            // constant 0x4
		kTypeCodeMask = 0x1F,                     // constant 0x1F
		kMetadata = 0x800,                   // constant 0x800
		kHomogeneous = 0x1000,                     // constant 0x1000
		kAlwaysPersist = 0x2000,                     // constant 0x2000
		kExposed = 0x2000,                     // constant 0x2000
		kLayoutImmutable = 0x4000,                    // constant 0x4000
		kBlittable = 0xFFFF8000                     // constant 0xFFFF8000
	};
};

enum TypeFlags
{
	kTypeFlag_Enum = 0xC000
};

class MemberTypeInfo
{
public:
	TypeInfo* typeInfo;	
	unsigned short flags;
	char pad[0x8];
};

class FieldInfo
{
public:
	char* name;
	MemberInfoFlags flags;
	unsigned short offset;
	char pad[0x4];
	MemberTypeInfo* typeInfo;
};

class FieldInfoEnum
{
public:
	char* name;
	MemberInfoFlags flags;
	unsigned short offset;
	char pad[0x4];
	__int32 value;
	char pad2[0x4];
};