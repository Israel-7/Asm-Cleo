#include "output.h"

bool clip(const std::string& text)
{
    if (auto ga = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1)) {
        OpenClipboard(HWND_DESKTOP);
        EmptyClipboard();

        memcpy(GlobalLock(ga), text.c_str(), text.length() + 1);
        GlobalUnlock(ga);
        SetClipboardData(CF_TEXT, ga);

        CloseClipboard();
        GlobalFree(ga);

        return true;
    }

    return false;
}

int output(Files& files, Mode& mode, Options& options, char* gppPath, char* gppArgs, char* nasmPath, char* nasmArgs)
{
    std::string source_;

    if (options.sb) {
        source_ = "hex\n/*\n";
    }
    else {
        source_ = "DUMP\n/*\n";
    }

    if (options.cppCode && mode == Mode::CPP)
    {
        std::fstream file(files.input, std::ios::in);

        while (!file.eof()) {
            static std::string line;
            getline(file, line);

            source_.append(line + "\n");
        }
        file.close();

        source_.append("*/\n\n");
    }
    else {
        source_ = source_.erase(source_.length() - 3);
    }

    char exec[260];

    if (mode == Mode::CPP) {
        sprintf(exec, "\"%s\" %s %s output.asm", gppPath, files.input, gppArgs);
        system(exec);
    }

    std::string asm_;
    const char* asmInOut = nullptr;

    if (mode == Mode::ASM) {
        asmInOut = files.input;
    }
    else if (mode == Mode::CPP) {
        asm_ = "BITS 32\n";
        asmInOut = "output.asm";
    }

    std::fstream asmFile(asmInOut, std::ios::in);

    while (!asmFile.eof())
    {
        static std::string line;
        getline(asmFile, line);

        if (line.find("\t.") != line.npos) {
            continue;
        }

        const char* keywords[]{
            " PTR ",
            " FLAT:",
            " OFFSET ",
            "/APP",
            "/NO_APP"
        };

        for (auto a : keywords)
        {
            if (line.find(a) != line.npos) {
                line.replace(line.find(a), strlen(a), " ");
            }
        }

        asm_.append(line + "\n");
    }

    asmFile.close();

    asmFile = std::fstream("output.asm", std::ios::out);
    asmFile << asm_;
    asmFile.close();

    sprintf(exec, "\"%s\" %s %s output.asm", nasmPath, nasmArgs, files.output);
    system(exec);

    std::string code_;

    std::fstream finalFile(files.output, std::ios::in);

    while (!finalFile.eof())
    {
        static std::string line;
        getline(finalFile, line);

        int lineNumber;
        char byteCounter[32];
        char byteCode[32]{ 0 };
        char firstInstruction[32];

        int results = sscanf(line.c_str(), "%i %s %s %s", &lineNumber, byteCounter, byteCode, firstInstruction);

        if (line.length() > 7 && results >= 4)
        {
            line.insert(line.find(firstInstruction), "// ");

            line.erase(0, 7);

            if (options.sb) {
                line.insert(8, "}");
                line.insert(0, "{");
            }
            else {
                line.insert(8, "*/");
                line.insert(0, "/*");
            }
        }

        if (results == 2 && line.find(":") != line.npos) {
            line.erase(0, 12);
            line.insert(line.find(byteCounter), "// ");
        }

        if (results == 1) {
            code_.append("\n");
            continue;
        }

        static bool inited = false;

        if (!inited)
        {
            if (results >= 4 && !strcmp(byteCounter, "00000000"))
                inited = true;
            else continue;
        }

        code_.append(line + "\n");
    }

    finalFile.close();

    code_.insert(0, source_);

    int pos;

    if (options.sb) {
        pos = code_.find_last_of("}");
    }
    else {
        pos = code_.find_last_of("*");
    }

    code_ = code_.erase(pos + (code_.substr(pos).find('\n')));

    if (options.sb) {
        code_.append("\nend");
    }
    else {
        code_.append("\nENDDUMP");
    }

    finalFile = std::fstream(files.output, std::ios::out);
    finalFile << code_;
    finalFile.close();

    if (options.clip)
    {
        if (clip(code_)) {
            printf("Content saved and copied to the clipboard.\n");
        }
        else {
            fprintf(stderr, "Failed to copy content.");
        }
    } else {
        printf("Content saved.\n");
    }

    return EXIT_SUCCESS;
}
