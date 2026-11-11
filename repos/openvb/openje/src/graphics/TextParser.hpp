#pragma once

#include <windows.h>
#include <cstddef>
#include <stdio.h>
#include <string>

// Shared original text-token APIs used by the G3D readers. Input/scanner
// ownership is recovered separately; this interface does not adapt FILE input.
namespace G3DText
{
	struct Token
	{
		void ParseError() const; // 0x4CC4D0, actual token receiver in ECX.
		LONG kind;
		LONG line; //+4 is unwritten by53B130; readers must not use it as current line.
		union Value
		{
			LONG identifier;
			double number;
		} value;
	};
	typedef char TokenSizeMustBe16[sizeof(Token) == 16 ? 1 : -1];
	typedef char TokenValueMustBeAt8[offsetof(Token, value) == 8 ? 1 : -1];

	Token NextToken(); //52B940; actual token queue pop_back.
	void Expect(char kind); //52BA00; current input line owns diagnostics.
	std::string TokenString(LONG identifier); //52AFE0; signed interner lookup, copied result.
	std::string ReadString(); //52BA50; requires identifier kind100h.
	double ReadNumber(); //52BAF0; requires number kind101h.
    FILE* Initialize(FILE* input); //52BD60; borrowed FILE, never closes it.
    void Reset(); //52BC00; actual scanner/dictionary/token lifetime cleanup.
}
