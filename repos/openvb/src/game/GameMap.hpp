#pragma once
#include "entity/Map.hpp"
namespace F3
{
	class GameMap : public JE::Map
	{
	public:
		GameMap(); // 0x46C8E0; actual base construction and derived table.
		virtual ~GameMap(); // 0x46C910.
		virtual JE::Entity* FindNearest(JE::Entity* from,const std::string& role); // 0x46C920.
		virtual void GetCameraBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const; // 0x46C980.
		virtual void AddEntity(JE::Entity* entity); // 0x46CBB0.
		virtual void RemoveEntity(JE::Entity* entity); // 0x46CAE0.
	};
}
