#include "TextParser.hpp"
#include "../model/CommonException.hpp"
#include <list>
#include <map>
#include <strstream>
#include <stdlib.h>
#include <io.h>

// The original callers retain their continuation after this real game API.
// Do not import the later host declaration's noreturn optimization contract.
namespace JE { void __cdecl FatalError(const char* format, ...); }

namespace
{
    //70B268/70B274: two actual VC7 maps, not a custom tree implementation.
    struct IdentifierDictionary
    {
        void Clear();
        void Add(const std::string& name, const LONG& identifier);
        std::map<std::string, LONG> byName;
        std::map<LONG, std::string> byIdentifier;
    };

    LONG g_Line; //70B254: scalar current input line, not a pointer.
    LONG g_NextIdentifier; //70B258: BSS0; Reset deliberately does not clear it.
    std::list<G3DText::Token> g_Tokens; //70B25C
    IdentifierDictionary g_Dictionary; //70B268
    LONG g_Identifier; //70BB84: last identifier semantic value.
    double g_Number; //70BB88: last number semantic value.

    void IdentifierDictionary::Clear() //52B700
    {
        byName.clear();
        byIdentifier.clear();
    }

    void IdentifierDictionary::Add(const std::string& name, const LONG& identifier) //52BEC0
    {
        if (byName.find(name) != byName.end())
            JE::FatalError("Duplicate symbol found.\n");
        if (byIdentifier.find(identifier) != byIdentifier.end())
            JE::FatalError("Duplicate value found.\n");
        byName[name] = identifier;
        byIdentifier[identifier] = name;
    }

    LONG InternIdentifier(const char* text) //52BF40
    {
        if (g_Dictionary.byName.find(std::string(text)) == g_Dictionary.byName.end())
        {
            g_Dictionary.Add(std::string(text), g_NextIdentifier);
            ++g_NextIdentifier;
        }
        return g_Dictionary.byName.find(std::string(text))->second;
    }

    LONG CopyTokenText(const std::string& text, char* output, DWORD capacity) //4CA870
    {
        DWORD length = text.size();
        if (length == 0 || length + 1 > capacity)
            return 1;
        for (DWORD index = 0; index < length + 1; ++index)
            output[index] = text.c_str()[index];
        return 0;
    }
}

// Real Flex2.5.4a output, generated from the recovered game rules. Keep in
// this TU: Reset accesses the actual scanner's private input/buffer state.
// No generator, reconstructed scanner skeleton or FILE facade runs in game.
#include "TextParserScanner.inc"

namespace G3DText
{
    Token ScanToken() //53B130; actual ordinary hidden16B return-object API.
    {
        Token token;
        token.kind = yylex();
        if (token.kind == 257)
            token.value.number = g_Number;
        else
            token.value.identifier = g_Identifier;
        // Original writes neither +4 nor the high payload DWORD for a
        // nonnumber token. The return copies those unwritten local bytes.
        return token;
    }

    Token NextToken() //52B940
    {
        if (g_Tokens.empty())
        {
            Token token = ScanToken();
            g_Tokens.push_back(token);
        }
        Token token = g_Tokens.back();
        g_Tokens.pop_back();
        return token;
    }

    void Expect(char kind) //52BA00
    {
        Token token = NextToken();
        if (token.kind != kind)
            JE::FatalError("parse error line %d: expected %c", g_Line, kind);
    }

    std::string TokenString(LONG identifier) //52AFE0
    {
        // The original dereferences find() without an end() fallback.
        return g_Dictionary.byIdentifier.find(identifier)->second;
    }

    std::string ReadString() //52BA50
    {
        Token token = NextToken();
        if (token.kind != 256)
            JE::FatalError("parse error line %d: expected identifier", g_Line);
        return g_Dictionary.byIdentifier.find(token.value.identifier)->second;
    }

    double ReadNumber() //52BAF0
    {
        Token token = NextToken();
        if (token.kind != 257)
            JE::FatalError("parse error, expected number");
        return token.value.number;
    }

    void Token::ParseError() const //4CC4D0
    {
        char buffer[1000];
        if (kind == 256)
        {
            std::string text = TokenString(value.identifier);
            CopyTokenText(text, buffer, sizeof(buffer));
        }
        else if (kind == 257)
            sprintf(buffer, "%g", value.number);
        else
        {
            buffer[0] = static_cast<char>(kind);
            buffer[1] = 0;
        }
        std::strstream message;
        message << "parse error line " << g_Line << ", token '" << buffer << "'" << std::endl;
        // str() freezes the actual CRT stream buffer; the game's Exception
        // borrows it. Preserve that lifetime/allocation policy and no ends().
        throw Common::Exception(message.str());
    }

    void Reset() //52BC00
    {
        g_Dictionary.Clear();
        g_Tokens.resize(0);
        g_Line = 0;
        yyin = NULL;
        if (yy_current_buffer)
        {
            yy_flush_buffer(yy_current_buffer);
            yy_delete_buffer(yy_current_buffer);
            yy_current_buffer = NULL;
        }
        yy_c_buf_p = NULL;
        yy_start = 0;
        yyin = NULL;
        yyout = NULL;
        yy_init = 1;
    }

    FILE* Initialize(FILE* input) //52BD60
    {
        Reset();
        g_Line = 1;
        yyin = input;
        return input;
    }
}
