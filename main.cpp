#include "output.h"

const char* helpMsg =
R"(Usage: helper [cpp|asm] input_file [options] output_file

Helper Options:
    -help                    Show this information.
    -about                   Show credits information.

Output Options:
    -gta3sc                  Output code with GTA3Script syntax.
    -sb                      Output code with Sanny Builder syntax.
    -cppcode                 Include C++ code (only for cpp mode).
    -clip                    Copy output to the clipboard.
)";

const char* creditsMsg =
R"(Helper to get assembly bytecode to use in cleo scripts.
Powered By: Israel.

2018.
)";

ErrorCode readArgs(char** argv, Files& files, Mode& mode, Options& options) {
    if (*(++argv))
    {
        if (!strcmp(*argv, "-help")) {
            options.help = true;
            return ErrorCode::Success;
        }
        else if (!strcmp(*argv, "-about")) {
            options.about = true;
            return ErrorCode::Success;
        }
        else if (!strcmp(*argv, "cpp")) {
            mode = Mode::CPP;
        }
        else if (!strcmp(*argv, "asm")) {
            mode = Mode::ASM;
        }
        else {
            return ErrorCode::ModeErr;
        }
    }
    else {
        return ErrorCode::ModeArg;
    }

    if (*(++argv)) {
        files.input = *argv;
    }
    else {
        return ErrorCode::InputArg;
    }

    ErrorCode result = ErrorCode::OptionErr;
    char temp[32];

    while (*argv) {
        if (!strcmp(*argv, "-gta3sc")) {
            options.gta3sc = true;
            result = ErrorCode::Success;
            strcpy(temp, *argv);
        }
        else if (!strcmp(*argv, "-sb")) {
            options.sb = true;
            result = ErrorCode::Success;
            strcpy(temp, *argv);
        }
        else if (!strcmp(*argv, "-cppcode")) {
            options.cppCode = true;
            result = ErrorCode::Success;
            strcpy(temp, *argv);
        }
        else if (!strcmp(*argv, "-clip")) {
            options.clip = true;
            result = ErrorCode::Success;
            strcpy(temp, *argv);
        }

        ++argv;
    }

    if (strcmp(*(--argv), temp)) {
        files.output = *argv;
    }
    else {
        result = ErrorCode::OutputErr;;
    }

    return result;
}

bool loadSettings(char* gppPath, char* nasmPath, char* gppArgs, char* nasmArgs)
{
    GetPrivateProfileStringA("MinGW", "G++ Path", nullptr, gppPath, 260, "./Config.ini");
    GetPrivateProfileStringA("MinGW", "Arguments", nullptr, gppArgs, 260, "./Config.ini");

    GetPrivateProfileStringA("Nasm", "Nasm Path", nullptr, nasmPath, 260, "./Config.ini");
    GetPrivateProfileStringA("Nasm", "Arguments", nullptr, nasmArgs, 260, "./Config.ini");

    if (GetLastError()) {
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    Files files;
    Mode mode = Mode::None;
    Options options;

    ErrorCode result = readArgs(argv, files, mode, options);

    if (result == ErrorCode::Success)
    {
        if (options.help) {
            printf(helpMsg);
            return EXIT_SUCCESS;
        } else if (options.about) {
            printf(creditsMsg);
            return EXIT_SUCCESS;
        } else if (std::fstream(files.input).fail()) {
            fprintf(stderr, "Unable to open input file.");
            return EXIT_FAILURE;
        }
    }
    else
    {
        switch (result)
        {
        case ErrorCode::ModeArg:
            fprintf(stderr, "Type -help to view help information.");
            break;
        case ErrorCode::ModeErr:
            fprintf(stderr, "Unrecognized mode, type -help to view available modes.");
            break;
        case ErrorCode::InputArg:
            fprintf(stderr, "No input file typed, type -help to view usage.");
            break;
        case ErrorCode::OptionErr:
            fprintf(stderr, "Unrecognized option, type -help to view available options.");
            break;
        case ErrorCode::OutputErr:
            fprintf(stderr, "No output file typed, type -help to view usage.");
        }

        return EXIT_FAILURE;
    }

    char gppPath[260];
    char nasmPath[260];

    char gppArgs[260];
    char nasmArgs[260];

    if (!loadSettings(gppPath, nasmPath, gppArgs, nasmArgs)) {
        fprintf(stderr, "Failed to load settings.");
        return EXIT_FAILURE;
    }

    return output(files, mode, options, gppPath, gppArgs, nasmPath, nasmArgs);
}
