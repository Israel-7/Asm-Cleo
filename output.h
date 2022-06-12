#pragma once

#ifndef OUTPUT_H
#define OUTPUT_H

#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <string>
#include <windows.h>

struct Files {
    const char* input;
    const char* output;
};

enum class Mode {
    None,
    CPP,
    ASM
};

struct Options {
    bool help;
    bool about;
    bool gta3sc;
    bool sb;
    bool cppCode;
    bool clip;

    Options() :
        help(false), about(false),
        gta3sc(false), sb(false),
        cppCode(false), clip(false)
    {}
};

enum ErrorCode {
    Success,
    ModeArg,
    ModeErr,
    InputArg,
    OptionErr,
    OutputErr
};

bool clip(const std::string&);
int output(Files&, Mode&, Options&, char*, char*, char*, char*);

#endif
