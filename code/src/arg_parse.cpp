/**********************************************************************************
* .i. Peace Among Worlds .i.
*
* MIT License
*
* Copyright (c) 2017 Szilard Orban <devszilardo@gmail.com>
* All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
**********************************************************************************/

#include "arg_parse.h"

#define lerror(...)
#define linfo(...)

std::vector<std::pair<std::string, std::string>> pdrain::ArgParser::parseArguments(int argc, const char** argv)
{
    std::vector<std::pair<std::string, std::string>> values;
    for (int i = 0; i < argc; ++i)
    {
        std::string argument(argv[i]);
        const size_t delimiterPosition = argument.find_first_of("-");
        const size_t namePosition = argument.find_first_not_of("- \t");
        if (delimiterPosition > namePosition || delimiterPosition == std::string::npos)
        {
            linfo("Found free argument: ", argument);
            values.push_back(std::make_pair("free", argument));
            continue;
        }
        if (namePosition == std::string::npos || namePosition > argument.size() - 1)
        {
            lerror("Invalid argument, no value: ", argument);
            continue;
        }

        size_t valuePosition = argument.find_first_of('=');
        std::string argumentName;
        std::string argumentValue;
        if (valuePosition == std::string::npos)
        {
            argumentName = argument.substr(namePosition);
            argumentValue = "1";
        }
        else
        {
            ++valuePosition;
            if (namePosition > valuePosition)
            {
                lerror("Invalid argument, namePosition > valuePosition: ",
                       argument,
                       " namePos:",
                       std::to_string(namePosition),
                       " valuePos:",
                       std::to_string(valuePosition));
            }
            argumentName = argument.substr(namePosition, valuePosition - namePosition - 1);
            argumentValue = argument.substr(valuePosition);
        }
        values.push_back(std::make_pair(argumentName, argumentValue));
    }

    return values;
}
