#pragma once

#include "Action.hpp"

namespace JE
{
	class ActionEquip : public Action
	{
	public:
		ActionEquip(); //55FB40.
		virtual BYTE IsComplete() const;
		virtual void Cancel();
		virtual int Start(DWORD time);
		virtual int Update(DWORD time, DWORD budget);
		virtual int Finish(DWORD time);
		BYTE completed;
	};
	class ActionUnequip : public Action
	{
	public:
		ActionUnequip(); //5638B0.
		virtual BYTE IsComplete() const;
		virtual void Cancel();
		virtual int Start(DWORD time);
		virtual int Update(DWORD time, DWORD budget);
		virtual int Finish(DWORD time);
		BYTE completed;
	};
	class ActionSetActiveWeapon : public Action
	{
	public:
		ActionSetActiveWeapon(); //563890.
		virtual BYTE IsComplete() const;
		virtual void Cancel();
		virtual int Start(DWORD time);
		virtual int Update(DWORD time, DWORD budget);
		virtual int Finish(DWORD time);
		BYTE completed;
	};
}
