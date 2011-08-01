#include "../../RunTimeCompiler/ObjectInterfacePerModule.h"

#include "../SimpleTest/IObject.h"
#include "../../Systems/IUpdateable.h"

#include <iostream>


// Note: Currently need to put the interface for IID_* before the template
class RuntimeObject01 : public IAUUpdateable, public IObject
{
public:
	RuntimeObject01()
	{

	}

	virtual void Serialize(ISimpleSerializer *pSerializer)
	{
	}

	virtual void Update( float deltaTime )
	{
		std::cout << "Runtime Object 01 udate called!\n";
	}

	virtual void GetInterface( InterfaceID _iid, void** pReturn )
	{
		switch(_iid)
		{
		case IID_IUPDATEABLE:
			*pReturn= static_cast<IAUUpdateable*>( this );
			break;
		default:
			IObject::GetInterface(_iid, pReturn);
		}
	}

};


REGISTERCLASS(RuntimeObject01);