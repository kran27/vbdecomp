#pragma once

// Original8-byte proprietary exception. Its one-slot RTTI/ThrowInfo graph
// has no base class; the borrowed message is never freed. Actual CRT C++
// exception transport is reused, without introducing std::exception ancestry.
namespace Common
{
	struct Exception
	{
		explicit Exception(const char* text) throw() : message(text) {}
		virtual ~Exception() {} //4011A0, original vfptr store followed by RET.
		const char* what() const throw() { return message; }
		const char* message;
	};
}
