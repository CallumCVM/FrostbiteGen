

#ifndef CLASSINFO_H
#define CLASSINFO_H

#include "required.h"
#include "structs.h"



class ClassInfoManager
{
public:
	ClassInfoManager(ClassInfo* info);

	void BuildClassList();
	void DumpClasses();

private:
	std::vector<ClassInfo*> GetParents(ClassInfo* c);
	void	DumpClass(ClassInfo* c);
	int		DumpClassMembers(std::ofstream& file, std::vector<FieldInfo*>& members, int parentSize);
	void	ParseClassMembers(TypeInfo* ti, std::vector<FieldInfo*>& members);
	char*	GetFixedClassName(const char* orig);
	void	ResolveHeaders(std::vector<FieldInfo*> members, std::ofstream& file);
	
	void	DumpEnum(ClassInfo* c);
	void	DumpEnumMembers(std::ofstream& file, TypeInfo* ti);

	void	DumpStruct(ClassInfo* c);
	void	ParseStructMembers(TypeInfo* ti, std::vector<FieldInfo*>& members);

	void	DumpTypeInfo(ClassInfo* c,std::ofstream& file);
	void	DumpHeader(std::ofstream& file, const char* fileName);

private:
	ClassInfo* m_listHead;
	std::map<std::string, ClassInfo*, std::greater<std::string>> m_classMap;
};

#endif