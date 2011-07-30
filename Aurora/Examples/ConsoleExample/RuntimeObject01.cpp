#include "../../RunTimeCompiler/ObjectInterfacePerModule.h"

#include "../SimpleTest/IObject.h"

class RuntimeObject01 : public IObject
{
public:
	RuntimeObject01()
	{
	}

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
	}

};


REGISTERCLASS(RuntimeObject01);