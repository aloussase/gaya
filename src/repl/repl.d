import std.string;
import std.stdio : writeln, write;
import core.stdc.stdlib : exit, EXIT_SUCCESS;
import core.stdc.stdio : fgetc, stdin, EOF;
import core.runtime;
import std.algorithm;
import std.ascii : isControl;

extern (C++) void enableRawMode();
extern (C++) void disableRawMode();
extern (C++) void runLine(const char *) nothrow;
extern (C++) class lexer {
    static bool is_keyword(const char *) nothrow;
}

auto red(string s)   { return format("\x1b[31m%s\x1b[m", s); }
auto green(string s) { return format("\x1b[32m%s\x1b[m", s); }
auto yellow(string s) { return format("\x1b[33m%s\x1b[m", s); }
auto purple(string s) { return format("\x1b[35m%s\x1b[m", s); }

auto isOperator(string word)
{
    switch (word)
    {
        case "=", ">=", "<=", "==", "|>", "+", "-", "*", "/":
            return true;
        default:
            return false;
    }
}

auto highlightWord(string word)
{
    if (lexer.is_keyword(word.toStringz()))
    {
        return word.red();
    }
    
    if (word.isNumeric())
    {
        return word.yellow();
    }

    if (word.isOperator())
    {
        return word.green();
    }

    return word;
}

auto highlight(string line)
{
    return line.splitter(" ").map!(highlightWord).reduce!`a ~ ' ' ~ b`;
}

auto clearLine(string s)
{
    return format("\x1b[2K\x1b[9999D%s", s);
}

auto readline(string prompt)
{
    int c       = 0;
    string line = "";

    write(prompt);

    while ((c = fgetc(stdin)) != EOF && c != '\n')
    {
        if (!c.isControl())
        {
            line ~= c;
            (prompt ~ line.highlight()).clearLine().write();
        }
    }

    write('\n');

    return line;
}

extern (C++) auto repl() {
    Runtime.initialize();
    scope(exit) Runtime.terminate();

    enableRawMode();
    scope(exit) disableRawMode();

    auto prompt = "gaya> ".purple();
    auto multiLinePrompt = "gaya| ".purple();

    writeln("Gaya, version 0.0.1: https://github.com/aloussase/gaya");

    string line;
    while ((line = readline(prompt)) != ":quit")
    {
        if (line == ":{")
        {
            string program;
            while ((line = readline(multiLinePrompt)) != ":}")
            {
                program ~= line ~ '\n';
            }
            runLine(program.toStringz());
            continue;
        }

        if (line == "")
        {
            continue;
        }

        runLine(line.toStringz());
    }

    writeln("¡Hasta la vista!");
    exit(EXIT_SUCCESS);
}
