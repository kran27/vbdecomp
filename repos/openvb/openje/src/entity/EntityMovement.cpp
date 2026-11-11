#include "Entity.hpp"

namespace JE
{
	void Entity::OnMoveFailed(DWORD) //54BEB0.
	{
		moving = 0;
		ClearMapAssociations();
	}
	void Entity::OnMoveArrived(DWORD) //Same original54BEB0.
	{
		moving = 0;
		ClearMapAssociations();
	}
}
