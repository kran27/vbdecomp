#pragma once

#include <windows.h>

namespace JE
{
	class Entity;
	class Action
	{
	public:
		Action() : starting(1) {}
		virtual ~Action() {} // Shared4010E0, no owned children.
		virtual BYTE IsComplete() const = 0;
		virtual void Cancel() = 0;
		virtual BYTE GetKind() const { return kind; } // 4010D0.
		virtual int Start(DWORD time) = 0;
		virtual int Update(DWORD time, DWORD budget) = 0;
		virtual int Finish(DWORD time) = 0;
		int Process(DWORD time, DWORD budget); // 55A1C0.
		BYTE kind, starting;
		Entity* owner; // Unwritten until Entity::QueueAction.
		DWORD minimumBudget; // Original+C is unwritten by ActionAnim's constructor.
	};

	class ActionIdle : public Action
	{
	public:
		ActionIdle(); // 560180.
		virtual BYTE IsComplete() const; // Shared404B60.
		virtual void Cancel(); // Shared404B70.
		virtual int Start(DWORD time); // Shared404BB0.
		virtual int Update(DWORD time, DWORD budget); // Shared5A9070.
		virtual int Finish(DWORD time); // Shared404BB0.
		BYTE completed;
	};

	class ActionAnim : public Action
	{
	public:
		ActionAnim(); // 5A9230.
		virtual BYTE IsComplete() const; // 5A9260.
		virtual void Cancel(); // 5A9270.
		virtual int Start(DWORD time); // 5A9280.
		virtual int Update(DWORD time, DWORD budget); // 5A92F0.
		virtual int Finish(DWORD time); // 5A9310.
		DWORD group, file;
		WORD loops;
		BYTE reverse, completed;
		DWORD deadline;
	};
}
