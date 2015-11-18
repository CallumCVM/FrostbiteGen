

#include "classinfo.h"

/// <summary>
/// Initializes a new instance of the <see cref="ClassInfoManager"/> class.
/// </summary>
/// <param name="info">The information.</param>
ClassInfoManager::ClassInfoManager(ClassInfo* info) :
	m_listHead(info)
{

}

/// <summary>
/// Builds the class list.
/// </summary>
void ClassInfoManager::BuildClassList()
{
	ClassInfo* c = m_listHead;
	while (c != NULL)
	{
		if (c->typeInfo && c->typeInfo->name)
		{
			TypeInfo* ti = c->typeInfo;

			m_classMap[c->typeInfo->name] = c;
		}

		c = c->next;
	}
}

/// <summary>
/// Dumps the classes.
/// </summary>
void ClassInfoManager::DumpClasses()
{
	for (auto it : m_classMap)
	{
		std::string name = it.first;
		ClassInfo* c = it.second;
		if (!c)
			continue;

		TypeInfo* ti = c->typeInfo;
		if (!ti)
			continue;

		if (!ti->name)
			continue;

#ifdef _DEBUG
		Log("Dumping Class 0x%016llX", c);
#endif

		if (ti->flags == 49289)
		{
			DumpEnum(c);
		}
		else if (ti->flags == 41 || ti->flags == 32809 || ti->flags == 53289)
		{
			DumpStruct(c);
		}
		else if (ti->flags == 13 || ti->flags == 49357 || ti->flags == 49453 || ti->flags == 49421 || ti->flags == 49389 || ti->flags == 16493 ||
			ti->flags == 49517 || ti->flags == 16765 || ti->flags == 49341 || ti->flags == 49437 || ti->flags == 49405 || ti->flags == 49373 || ti->flags == 49501 ||
			ti->flags == 49485 || ti->flags == 49469 || ti->flags == 16541 || ti->flags == 16509 || ti->flags == 49325)
		{
			// data types
		}
		else if (ti->flags == 69)
		{
			// templates classes?
			//DumpClass(c);
		}
		else if (ti->flags == 29)
		{
			// unknown
		}
		else
		{
			DumpClass(c);
		}		
	}
}

/// <summary>
/// Dumps the class.
/// </summary>
/// <param name="c">The c.</param>
void ClassInfoManager::DumpClass(ClassInfo* c)
{
	TypeInfo* ti = c->typeInfo;
	if (!ti)
		return;

	if (strlen(ti->name) == 0)
		return;

	std::vector<ClassInfo*> parents = GetParents(c);

	char headerFile[128], headerPath[MAX_PATH];
	sprintf(headerFile, "SDK\\%s.h", c->typeInfo->name);

	GetDirFile(headerFile, headerPath, sizeof(headerPath));

	std::ofstream file;
	file.open(headerPath, std::ios::out | std::ios::trunc);

	if (!file.is_open())
		return;	

#ifdef _DEBUG
	Log("Dumping class %s 0x%016llX", ti->name, c);
	Log("\tFlags: %d", ti->flags);
	Log("\tSize: %d", ti->totalSize);
	Log("\tAlignment: %d", ti->alignment);
	Log("\tField count: %d", ti->fieldCount);
	Log("\tUnknown1: 0x%016llX", ti->unknown1);
	Log("\tFields: 0x%016llX", ti->fields);
	Log("\tParent 0x%016llX", c->parent);
	Log("\tisDataContainer %d", c->isDataContainer);
#endif

	// print the header
	file << std::endl << "#ifndef FBGEN_" << c->typeInfo->name << "_H" << std::endl;
	file << "#define FBGEN_" << c->typeInfo->name << "_H" << std::endl << std::endl;

	std::vector<FieldInfo*> members;
	ParseClassMembers(ti, members);
	ResolveHeaders(members, file);

	if (parents.size() > 0)
	{
		file << "#include \"" << parents.at(0)->typeInfo->name << ".h\"" << std::endl << std::endl;

		file << "class " << ti->name << " :" << std::endl;
		DWORD inheritedOffset = 0;

		// can do multiple inheritence here instead, cleaner not to though
		//for (int i = parents.size() - 1; i >= 0; i--)
		{
			ClassInfo* p = parents.at(0);
			//if (i > 0)
			//	file << "\tpublic " << p->typeInfo->name << ", // Inherited class at 0x" << std::hex << inheritedOffset << std::endl;
			//else
			file << "\tpublic " << p->typeInfo->name << " // size = 0x" << std::hex << p->typeInfo->totalSize << std::endl;

			inheritedOffset += p->typeInfo->totalSize;
		}
	}
	else
	{
		file << "class " << ti->name << std::endl;
	}

	file << "{" << std::endl;
	file << "public:" << std::endl;

	DumpTypeInfo(c, file);

	int totalSizeOfClass = ti->totalSize;
	int totalSizeOfParents = 0;
	if (parents.size() > 0)
	{
		//for (auto p : parents)
		totalSizeOfParents += parents.at(0)->typeInfo->totalSize;
	}

	int memberSize = DumpClassMembers(file, members, totalSizeOfParents);
	if (memberSize + totalSizeOfParents < totalSizeOfClass)
		file << "\tunsigned char _0x" << std::hex << (memberSize + totalSizeOfParents) << "[0x" << std::hex << (totalSizeOfClass - (memberSize + totalSizeOfParents)) << "];" << std::endl;

	file << "}; // size = 0x" << std::hex << totalSizeOfClass << std::endl << std::endl;

	file << "#endif // FBGEN_" << c->typeInfo->name << "_H" << std::endl;

	file.close();
}

/// <summary>
/// Resolves the headers.
/// </summary>
/// <param name="members">The members.</param>
/// <param name="file">The file.</param>
void ClassInfoManager::ResolveHeaders(std::vector<FieldInfo*> members, std::ofstream& file)
{
	// remove duplicates
	for (unsigned int i = 0; i < members.size(); ++i)
	{
		FieldInfo* v1 = members.at(i);

		for (unsigned int j = 0; j < members.size(); ++j)
		{
			FieldInfo* v2 = members.at(j);

			if (v1 == v2)
				continue;

			if (!strcmp(v1->typeInfo->typeInfo->name, v2->typeInfo->typeInfo->name))
			{
				members.erase(members.begin()+i);
				i--;
				break;
			}
		}
	}
	for (auto m : members)
	{
		if (!strcmp(m->typeInfo->typeInfo->name, "Boolean"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Float32"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Float64"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Int8"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Int16"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Int32"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Int64"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Uint8"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Uint16"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Uint32"))
			continue;
		else if (!strcmp(m->typeInfo->typeInfo->name, "Uint64"))
			continue;

		file << "#include \"" << m->typeInfo->typeInfo->name << ".h\"" << std::endl;
	}
}

/// <summary>
/// Gets the name of the fixed class.
/// </summary>
/// <param name="orig">The original.</param>
/// <returns></returns>
char* ClassInfoManager::GetFixedClassName(const char* orig)
{
	if (!strcmp(orig, "Boolean"))
		return "bool";
	else if (!strcmp(orig, "Float32"))
		return "float";
	else if (!strcmp(orig, "Float64"))
		return "double";
	else if (!strcmp(orig, "Int8"))
		return "char";
	else if (!strcmp(orig, "Int16"))
		return "short";
	else if (!strcmp(orig, "Int32"))
		return "int";
	else if (!strcmp(orig, "Int64"))
		return "long";
	else if (!strcmp(orig, "Uint8"))
		return "unsigned char";
	else if (!strcmp(orig, "Uint16"))
		return "unsigned short";
	else if (!strcmp(orig, "Uint32"))
		return "unsigned int";
	else if (!strcmp(orig, "Uint64"))
		return "unsigned long";
	return (char*)orig;
}

/// <summary>
/// Parses the class members.
/// </summary>
/// <param name="ti">The ti.</param>
/// <param name="members">The members.</param>
void ClassInfoManager::ParseClassMembers(TypeInfo* ti, std::vector<FieldInfo*>& members)
{
	for (int i = 0; i < ti->fieldCount; ++i)
	{
		FieldInfo* fi = &ti->fields[i];

		if (!fi)
			continue;

		if (!fi->typeInfo || !fi->typeInfo->typeInfo)
			continue;

		members.push_back(fi);
	}

	auto memberSort = [](const FieldInfo* f1, const FieldInfo* f2) { return (f1->offset < f2->offset); };
	std::sort(members.begin(), members.end(), memberSort);
}

/// <summary>
/// Dumps the class members.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="members">The members.</param>
/// <param name="parentSize">Size of the parent.</param>
/// <returns></returns>
int ClassInfoManager::DumpClassMembers(std::ofstream& file, std::vector<FieldInfo*>& members, int parentSize)
{
	int totalSize = 0;

	bool first = true;	

	for (int i = 0; i < members.size(); ++i)
	{
		FieldInfo* fi = members.at(i);

		if (first)
		{
			//totalSize = fi->offset;

			if (fi->offset > parentSize)
			{
				file << "\tunsigned char _0x" << std::hex << parentSize << "[0x" << std::hex << (fi->offset - parentSize) << "];" << std::endl;
				totalSize += fi->offset - parentSize;
			}
			
			first = false;
		}
		
		MemberTypeInfo* fti = fi->typeInfo;

		if(fti->flags == 2527) // pointer
			file << "\t" << GetFixedClassName(fti->typeInfo->name) << "* m_" << fi->name << "; // 0x" << std::hex << fi->offset << std::endl;
		else
			file << "\t" << GetFixedClassName(fti->typeInfo->name) << " m_" << fi->name << "; // 0x" << std::hex << fi->offset << std::endl;

		totalSize += fti->typeInfo->totalSize;
	}

	return totalSize;
}

/// <summary>
/// Dumps the enum.
/// </summary>
/// <param name="c">The c.</param>
void ClassInfoManager::DumpEnum(ClassInfo* c)
{
	TypeInfo* ti = c->typeInfo;
	if (!ti)
		return;

	if (strlen(ti->name) == 0)
		return;

	char headerFile[128], headerPath[MAX_PATH];
	sprintf(headerFile, "SDK\\%s.h", c->typeInfo->name);

	GetDirFile(headerFile, headerPath, sizeof(headerPath));

	std::ofstream file;
	file.open(headerPath, std::ios::out | std::ios::trunc);

	if (!file.is_open())
		return;

#ifdef _DEBUG
	Log("Dumping enum %s 0x%016llX", ti->name, c);
	Log("\tFlags: %d", ti->flags);
	Log("\tSize: %d", ti->totalSize);
	Log("\tAlignment: %d", ti->alignment);
	Log("\tField count: %d", ti->fieldCount);
	Log("\tUnknown1: 0x%016llX", ti->unknown1);
	Log("\tFields: 0x%016llX", ti->fields);
	Log("\tParent 0x%016llX", c->parent);
	Log("\tisDataContainer %d", c->isDataContainer);
#endif

	// print the header
	file << std::endl << "#ifndef FBGEN_" << c->typeInfo->name << "_H" << std::endl;
	file << "#define FBGEN_" << c->typeInfo->name << "_H" << std::endl << std::endl;

	file << "enum " << ti->name << std::endl;

	file << "{" << std::endl;	

	DumpEnumMembers(file, ti);

	file << "};" << std::endl << std::endl;

	file << "#endif // FBGEN_" << c->typeInfo->name << "_H" << std::endl;

	file.close();
}

/// <summary>
/// Dumps the enum members.
/// </summary>
/// <param name="file">The file.</param>
/// <param name="ti">The ti.</param>
void ClassInfoManager::DumpEnumMembers(std::ofstream& file, TypeInfo* ti)
{
	for (int i = 0; i < ti->fieldCount; ++i)
	{
		FieldInfo* fi = &ti->enumFields[i];

		if (!fi)
			continue;

		FieldInfoEnum* fie = (FieldInfoEnum*)fi;

		file << "\t" << fie->name << " = " << std::hex << fie->value << "," << std::endl;
	}
}

/// <summary>
/// Gets the parents.
/// </summary>
/// <param name="c">The c.</param>
/// <returns></returns>
std::vector<ClassInfo*> ClassInfoManager::GetParents(ClassInfo* c)
{
	std::vector<ClassInfo*> parents;
	ClassInfo* p = c->parent;
	
	ClassInfo* lastP = c;
	while (p)
	{
		if (p == lastP)
			break;
		parents.push_back(p);
		lastP = p;
		p = p->parent;
	}

	return parents;
}

/// <summary>
/// Parses the structure members.
/// </summary>
/// <param name="ti">The ti.</param>
/// <param name="members">The members.</param>
void ClassInfoManager::ParseStructMembers(TypeInfo* ti, std::vector<FieldInfo*>& members)
{
	for (int i = 0; i < ti->fieldCount; ++i)
	{
		FieldInfo* fi = &ti->structFields[i];

		if (!fi)
			continue;

		if (!fi->typeInfo || !fi->typeInfo->typeInfo)
			continue;

		members.push_back(fi);
	}

	auto memberSort = [](const FieldInfo* f1, const FieldInfo* f2) { return (f1->offset < f2->offset); };
	std::sort(members.begin(), members.end(), memberSort);
}

/// <summary>
/// Dumps the structure.
/// </summary>
/// <param name="c">The c.</param>
void ClassInfoManager::DumpStruct(ClassInfo* c)
{
	TypeInfo* ti = c->typeInfo;
	if (!ti)
		return;

	if (strlen(ti->name) == 0)
		return;

	char headerFile[128], headerPath[MAX_PATH];
	sprintf(headerFile, "SDK\\%s.h", c->typeInfo->name);

	GetDirFile(headerFile, headerPath, sizeof(headerPath));

	std::ofstream file;
	file.open(headerPath, std::ios::out | std::ios::trunc);

	if (!file.is_open())
		return;

#ifdef _DEBUG
	Log("Dumping struct %s 0x%016llX", ti->name, c);
	Log("\tFlags: %d", ti->flags);
	Log("\tSize: %d", ti->totalSize);
	Log("\tAlignment: %d", ti->alignment);
	Log("\tField count: %d", ti->fieldCount);
	Log("\tUnknown1: 0x%016llX", ti->unknown1);
	Log("\tFields: 0x%016llX", ti->fields);
	Log("\tParent 0x%016llX", c->parent);
	Log("\tisDataContainer %d", c->isDataContainer);
#endif

	// print the header
	file << std::endl << "#ifndef FBGEN_" << c->typeInfo->name << "_H" << std::endl;
	file << "#define FBGEN_" << c->typeInfo->name << "_H" << std::endl << std::endl;

	std::vector<FieldInfo*> members;
	ParseStructMembers(ti, members);
	ResolveHeaders(members, file);

	file << "struct " << ti->name << std::endl;

	file << "{" << std::endl;

	DumpTypeInfo(c, file);

	int totalSizeOfClass = ti->totalSize;
	int totalSizeOfParents = 0;

	int memberSize = DumpClassMembers(file, members, totalSizeOfParents);
	if (memberSize + totalSizeOfParents < totalSizeOfClass)
		file << "\tunsigned char _0x" << std::hex << (memberSize + totalSizeOfParents) << "[0x" << std::hex << (totalSizeOfClass - (memberSize + totalSizeOfParents)) << "];" << std::endl;

	file << "}; // size = 0x" << std::hex << totalSizeOfClass << std::endl << std::endl;

	file << "#endif // FBGEN_" << c->typeInfo->name << "_H" << std::endl;

	file.close();
}

/// <summary>
/// Dumps the type information.
/// </summary>
/// <param name="c">The c.</param>
/// <param name="file">The file.</param>
void ClassInfoManager::DumpTypeInfo(ClassInfo* c, std::ofstream& file)
{
	file << "\tstatic void* GetTypeInfo()" << std::endl;
	file << "\t{" << std::endl;
	file << "\t\treturn (void*)0x" << std::hex << c << std::endl;
	file << "\t}" << std::endl;
}