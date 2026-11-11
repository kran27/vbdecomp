#include "Entity.hpp"
#include "model/ModelInstance.hpp"

namespace JE
{
	BYTE DynamicObject::AddLight(Gfx_Light*) { return 0; } //591690.
	void DynamicObject::RemoveLight(Gfx_Light*) {} //5A9320.
	void DynamicObject::ClearLights() {} //404B70.
	void DynamicObject::FinishLights() {} //404B70.

	BYTE Entity::AddLight(Gfx_Light* light)
	{
		if (model) return model->AddLight(light);
		return 0;
	}
	void Entity::RemoveLight(Gfx_Light* light) { if (model) model->RemoveLight(light); }
	void Entity::ClearLights() { if (model) model->ClearLights(); }
	void Entity::FinishLights() { if (model) model->FinishLights(); }
}
