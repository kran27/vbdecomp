/* Game rules recovered from F3.exe 53AD70. The scanner skeleton is Flex 2.5.4a;
 * it is a reused dependency, not reconstructed proprietary code. */
%option noyywrap nodefault
%x comment quoted
%{
/* TextParser.cpp owns these real game semantic values and input lifetime. */
%}
%%
[a-zA-Z_][a-zA-Z0-9_]* { g_Identifier = InternIdentifier(yytext); return 256; }
[+-]?([0-9]+\.?([Ee][+-]?[0-9]+)?|[0-9]*\.[0-9]+([Ee][+-]?[0-9]+)?) { g_Number = atof(yytext); return 257; }
\"                    { BEGIN(quoted); }
[(){}]                { return *yytext; }
[ \t\r,]+             ;
#[^\n]*               ;
\n                    { ++g_Line; }
.                     { JE::FatalError("Lex error on line %d", g_Line); }
"/*"                  { BEGIN(comment); }
<comment>[^*\n]*      ;
<comment>"*"+[^*/\n]* ;
<comment>\n           { ++g_Line; }
<comment>"*"+"/"      { BEGIN(INITIAL); }
<quoted>\n            { JE::FatalError("linefeed in quoted text"); }
<quoted>[^"\n]*       { g_Identifier = InternIdentifier(yytext); return 256; }
<quoted>\"            { BEGIN(INITIAL); }
<comment><<EOF>>       { JE::FatalError("Unterminated comment"); }
<INITIAL,quoted><<EOF>> { return 0; }
%%
