#pragma once

#include <windows.h>
#include <list>

namespace JE
{
	class Entity;
	class Combat
	{
	public:
		Combat(); // 0x5A9610, factory545100.
		virtual ~Combat(); // 0x5A9A10: SaveSettings before Cleanup.
		virtual void Initialize(); // 0x5A9650, actual settings56AF30.
		virtual void SaveSettings(); // 0x5A98B0; mode only.
		virtual void OnActorsChanged(DWORD time); // Shared no-op5A9320.
		void SetMode(DWORD value); // 0x5A9330, live Entity creation list.
		void AddEntity(Entity* entity); // 0x5A9810, insert at front, ChangeState5.
		void RemoveEntity(Entity* entity); // 0x5A93C0, unlink before ChangeState4.
		void ClearEntities(); // 0x5A9440, unlink before every ChangeState4.
		void Cleanup(); // 0x5A95F0; cursor is intentionally not reset.
		void Update(DWORD time); // 0x5A94A0.
		DWORD mode; // Original+4, initial0.
		BYTE active; // Original+8, initial0; gates World maps/entities/time.
		std::list<Entity*>::iterator cursor; // Original+0C, initially singular.
		DWORD timeOffset; // Original+10, initial0.
		BYTE stopRequested, showOutlines; // Original+14/+15, initial0/1.
		std::list<Entity*> actors; // Original+18, borrowed. Size+20 selects round clock.
	private:
		Combat(const Combat&);
		Combat& operator=(const Combat&);
	};
}
