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

#include <iostream>
#include <string>
#include <chrono>
#include <time.h>

// TODO(Feature): Allow stat to run across multiple database files..

namespace pdrain
{
enum class Operation: char
{
    START,
    STOP,
    STAT,
    DUMP,
    UNKNOWN,
};

Operation convert(const std::string& op)
{
    if (op == "start")
    {
        return Operation::START;
    }
    else if (op == "stop")
    {
        return Operation::STOP;
    }
    else if (op == "stat")
    {
        return Operation::STAT;
    }
    else if (op == "dump")
    {
        return Operation::DUMP;
    }
    return Operation::UNKNOWN;
}

std::string trimWhiteSpace(const std::string& str)
{
    std::string out;
    for (int i = 0; i < str.size(); ++i)
    {
        if (str[i] == ' ')
        {
            continue;
        }
        else if (str[i] == '\t')
        {
            continue;
        }
        else
        {
            out = str.substr(i);
            break;
        }
    }

    size_t j = out.size();
    for (;j > 0; --j)
    {
        if (out[j - 1] == ' ')
        {
            continue;
        }
        else if (out[j - 1] == '\t')
        {
            continue;
        }
        else
        {
            out = out.substr(0, j);
            break;
        }
    }

    return out;
}

struct StartOperationData
{
    int64_t timestamp;
    std::string note;
};

struct StopOperationData
{
    std::string exitCode;
    int64_t timestamp;
};


struct BuildGraphData
{
    std::vector<int64_t> totalBuildTimes;
    std::vector<double> avgBuildTimes;
    std::vector<std::string> buildDates;
};

struct StatOperationData
{
    std::vector<std::pair<Operation, void*>> ops; // Note: Don't bother deleting the data, allocated once, OS will reclaim in the end

    size_t totalBuildCount;
    size_t successfulBuildCount;
    size_t totalBuildTime;
    double avgBuildTime;
    size_t lastBuildTime;
    BuildGraphData buildGraphData;
};

struct Context
{
    void* additionalOperationData; // Note: Don't bother deleting the data, allocated once, OS will reclaim in the end
    Operation operation;
    std::string outFilePath;
};


std::string computeDateStr(int64_t timestamp)
{
    struct tm tmbuff;
    gmtime_s(&tmbuff, &timestamp);
    const std::string dmy = std::to_string(tmbuff.tm_mday) + "." +
                            std::to_string(tmbuff.tm_mon + 1) + "." +
                            std::to_string(tmbuff.tm_year + 1900);
    return dmy;
}

bool init(const std::vector<std::pair<std::string, std::string>>& arguments, Context& ctx)
{
    const auto printHelp = []()
    {
        std::cout << "Description:" << std::endl;
        std::cout << "    Profit Drain is a simple tool that helps track the time spent waiting for builds to finish." << std::endl << std::endl;

        std::cout << "Usage: " << std::endl;
        std::cout << "    profitDrain -o=<timer database file> -x=<Operation To Execute>" << std::endl;
        std::cout << "       -x=<Operation to execute>" << std::endl;
        std::cout << "           \"start <note>\" - start timer" << std::endl;
        std::cout << "           \"stop <exit code>\" - stop timer" << std::endl;
        std::cout << "           stat - print build time statistics" << std::endl;
        std::cout << "           dump - dump raw data as text" << std::endl;
        std::cout << "       -o=<Timer database file name>" << std::endl;
        std::cout << "       -h Help" << std::endl << std::endl;

        std::cout << "Usage examples: " << std::endl;
        std::cout << "    profitDrain -o=t.db -x=stat" << std::endl;
        std::cout << "    profitDrain -o=t.db -x=dump" << std::endl;
        std::cout << "    profitDrain -o=t.db -x=start" << std::endl;
        std::cout << "    profitDrain -o=t.db -x=\"start First build after integrating library xyz.\"" << std::endl;
        std::cout << "    profitDrain -o=t.db -x=\"stop 0\"" << std::endl;
        std::cout << "    profitDrain -o=t.db -x=\"stop 32\"" << std::endl << std::endl;

        std::cout << "Motivation:" << std::endl;
        std::cout << "    Waiting for builds instead of actively working on solving problems is wasted time and can cause frustration,\n"
                     "loss of concentration, lower productivity, context switching, and many more issues. In case of a larger team, \n"
                     "the problems can become much worse.\n"
                     "    This tool should highlight the benefits of faster build times and short develop-build-test cycles, \n"
                     "and that this is something very much worth investing in. Fast development cycles are great for any project!" << std::endl;
        std::cout << "    Frustration and misery experienced with slow builds and joy felt working with well structured, fast builds has led\n"
                     "to the creation of this small tool. ";
        std::cout << "I hope someone else will find it useful." << std::endl << std::endl;

        std::cout << "Copyright:" << std::endl;
        std::cout << "    (c) 2017 Szilard Orban <devszilardo@gmail.com>. All Rights Reserved." << std::endl << std::endl;

        std::cout << "Support the work:" << std::endl;
        std::cout << "    If you feel that you are getting value from the software, please consider supporting my work by donating \n"
                     "and passing the word about the software to those that might find it useful." << std::endl;
        std::cout << "Your help is much appreciated:" << std::endl;
        std::cout << "    https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=P9FRPKSN7J6WC&source=url" << std::endl << std::endl;

        exit(0);
    };

    bool outputFileSet = false, operationSpecified = false;
    for(const auto& val: arguments)
    {
        if (val.first == "x")
        {
            const std::string command = trimWhiteSpace(val.second.substr(0, val.second.find_first_of(' ', 0)));
            ctx.operation = convert(command);
            if (ctx.operation == Operation::UNKNOWN)
            {
                std::cerr << "Invalid operation specified!" << std::endl;
                printHelp();
                return false;
            }
            else if (ctx.operation == Operation::START)
            {
                StartOperationData* startData = new StartOperationData();
                const std::string rawOption = trimWhiteSpace(val.second);
                const size_t firstSpacePos = rawOption.find_first_of(' ', 0);

                if (firstSpacePos != std::string::npos)
                {
                    const std::string note = rawOption.substr(firstSpacePos);
                    if (note.size() > 0)
                    {
                        startData->note = trimWhiteSpace(note);
                    }
                }
                ctx.additionalOperationData = startData;
                operationSpecified = true;
            }
            else if (ctx.operation == Operation::STOP)
            {
                StopOperationData* stopData = new StopOperationData();
                stopData->exitCode = trimWhiteSpace(val.second.substr(val.second.find_first_of(' ', 0)));
                ctx.additionalOperationData = stopData;
                operationSpecified = true;
            }
            else if (ctx.operation == Operation::STAT)
            {
                operationSpecified = true;
            }
            else if (ctx.operation == Operation::DUMP)
            {
                operationSpecified = true;
            }
        }
        else if (val.first == "o")
        {
            if (val.second.size() <= 0)
            {
                std::cerr << "Invalid output file specified: " << ctx.outFilePath << std::endl;
                printHelp();
                return false;
            }
            ctx.outFilePath = val.second;
            outputFileSet = true;
        }
        else if (val.first == "h")
        {
            printHelp();
        }
        else
        {
            std::cout << "Unknown parameter: " << val.first << " and value: " << val.second << std::endl << std::endl;
            printHelp();
        }
    }

    return outputFileSet && operationSpecified;
}


int writeData(const Context& context, StopOperationData* data)
{
    FILE* f = fopen(context.outFilePath.c_str(), "ab");
    if(!f)
    {
        std::cerr << "Failed to open output file: " << context.outFilePath << std::endl;
        return -2;
    }

    fwrite(&context.operation, 1, sizeof(Operation::STOP), f);
    fwrite(&data->timestamp, 1, sizeof(data->timestamp), f);

    const size_t exitCodeSize = data->exitCode.size();
    fwrite(&exitCodeSize, 1, sizeof(exitCodeSize), f);

    fwrite(data->exitCode.c_str(), 1, exitCodeSize, f);

    fclose(f);

    return 0;
}

int writeData(const Context& context, StartOperationData* data)
{
    FILE* f = fopen(context.outFilePath.c_str(), "ab");
    if(!f)
    {
        std::cerr << "Failed to open output file: " << context.outFilePath << std::endl;
        return -2;
    }

    fwrite(&context.operation, 1, sizeof(context.operation), f);

    fwrite(&data->timestamp, 1, sizeof(data->timestamp), f);

    const size_t noteSize = data->note.size();
    fwrite(&noteSize, 1, sizeof(noteSize), f);

    if (noteSize > 0)
    {
        fwrite(data->note.c_str(), 1, noteSize, f);
    }

    fclose(f);

    return 0;
}

int start(Context& context)
{
    StartOperationData* data = (StartOperationData *) context.additionalOperationData;
    data->timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    return writeData(context, data);
}

int stop(Context& context)
{
    StopOperationData* data = (StopOperationData *) context.additionalOperationData;
    data->timestamp = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    return writeData(context, data);
}

int readBuildTimerData(Context& ctx, StatOperationData& data)
{
    FILE* f = fopen(ctx.outFilePath.c_str(), "rb");
    if (!f)
    {
        std::cerr << "Failed to open input file: " << ctx.outFilePath << std::endl;
        return -2;
    }

    int i = 0;
    Operation op;
    fread((void*) &op, 1, sizeof(op), f);
    while(feof(f) == 0)
    {
        ++i;
        if (op == Operation::START)
        {
            StartOperationData *sd = new StartOperationData();
            fread((void*) &sd->timestamp, 1, sizeof(sd->timestamp), f);

            size_t noteSize = 0;
            fread((void*) &noteSize, 1, sizeof(noteSize), f);

            sd->note.resize(noteSize, '\0');
            fread((void*) sd->note.data(), 1, noteSize, f);
            //std::cout << "Start " << sd->timestamp << " note size: " << noteSize << " note: "<< sd->note << std::endl;
            data.ops.push_back(std::make_pair(op, sd));
        }
        else if (op == Operation::STOP)
        {
            StopOperationData* sd = new StopOperationData();
            fread((void*) &sd->timestamp, 1, sizeof(sd->timestamp), f);

            size_t exitCodeSize;
            fread((void*) &exitCodeSize, 1, sizeof(exitCodeSize), f);

            sd->exitCode.resize(exitCodeSize, '\0');
            fread((void*) sd->exitCode.data(), 1, exitCodeSize, f);

            //std::cout << "Stop " << sd->timestamp << " exit code: " << sd->exitCode << std::endl;
            data.ops.push_back(std::make_pair(op, sd));
        }
        fread((void*) &op, 1, sizeof(op), f);
    }
    fclose(f);

    return 0;
}

void printBuildStats(const StatOperationData& data)
{
    std::cout << "Build stats: " << std::endl;
    std::cout << "    Total build time: "
              << (int64_t) (data.totalBuildTime / 1000.0 / 3600.0 / 24.0) << " days, "
              << ((data.totalBuildTime / 1000 / 3600) % 24) << " hours, "
              << ((data.totalBuildTime / 1000 / 60) % 60) << " minutes, "
              << (data.totalBuildTime / 1000 % 60) << " seconds, "
              << data.totalBuildTime % 1000 << " milliseconds. "
              << "(" << data.totalBuildTime << " ms total)" << std::endl;
    std::cout << "    Avg build time: "
              << (int64_t) (data.avgBuildTime / 1000.0 / 3600.0 / 24.0) << " days, "
              << ((int64_t) (data.avgBuildTime / 1000 / 3600) % 24) << " hours, "
              << ((int64_t) (data.avgBuildTime / 1000 / 60) % 60) << " minutes, "
              << ((int64_t) (data.avgBuildTime / 1000) % 60) << " seconds, "
              << (int64_t) data.avgBuildTime % 1000 << " milliseconds. "
              << "(" << data.avgBuildTime << " ms total)" << std::endl;
    std::cout << "    Last build time: "
              << (int64_t) (data.lastBuildTime / 1000.0 / 3600.0 / 24.0) << " days, "
              << ((data.lastBuildTime / 1000 / 3600) % 24) << " hours, "
              << ((data.lastBuildTime / 1000 / 60) % 60) << " minutes, "
              << (data.lastBuildTime / 1000 % 60) << " seconds, "
              << data.lastBuildTime % 1000 << " milliseconds. "
              << "(" << data.lastBuildTime << " ms total)" << std::endl;
    std::cout << "    Total build count: " << data.totalBuildCount << std::endl;
    std::cout << "    Successful build count: " << data.successfulBuildCount << std::endl;
}

void drawBuildTimeGraph(const StatOperationData& data)
{
    if (data.buildGraphData.buildDates.size() < 1)
    {
        return;
    }

    double minAvgBuildTime = data.buildGraphData.avgBuildTimes[0];
    double maxAvgBuildTime = data.buildGraphData.avgBuildTimes[0];
    int64_t minTotalBuildTime = data.buildGraphData.totalBuildTimes[0];
    int64_t maxTotalBuildTime = data.buildGraphData.totalBuildTimes[0];
    for (int i = 0; i < data.buildGraphData.buildDates.size(); ++i)
    {
        if (minAvgBuildTime > data.buildGraphData.avgBuildTimes[i])
        {
            minAvgBuildTime = data.buildGraphData.avgBuildTimes[i];
        }
        if (maxAvgBuildTime < data.buildGraphData.avgBuildTimes[i])
        {
            maxAvgBuildTime = data.buildGraphData.avgBuildTimes[i];
        }

        if (minTotalBuildTime > data.buildGraphData.totalBuildTimes[i])
        {
            minTotalBuildTime = data.buildGraphData.totalBuildTimes[i];
        }
        if (maxTotalBuildTime < data.buildGraphData.totalBuildTimes[i])
        {
            maxTotalBuildTime = data.buildGraphData.totalBuildTimes[i];
        }
    }
    const double maxHeight = 11.0;
    const double maxWidth = data.buildGraphData.buildDates.size();

    const int64_t tsNow = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    const int64_t tsOld = (tsNow - (maxWidth * 24 * 60 * 60));

    // Draw avg build time graph
    std::cout << "Average build times for the last " << maxWidth << " days (" << computeDateStr(tsOld) << " - " << computeDateStr(tsNow) << "):" << std::endl;
    std::cout << "            ";
    for (int j = maxHeight - 1; j >= 0; --j)
    {
        for (int i = maxWidth - 1; i >= 0; --i)
        {
            const int columnHeight = data.buildGraphData.avgBuildTimes[i] * maxHeight / (double)maxAvgBuildTime;
            if (j <= columnHeight)
            {
                std::cout << "*";
            }
            else
            {
                std::cout << " ";
            }

            if (i == 0)
            {
                std::cout << "| ";
                if (j == 0)
                {
                    std::cout << minAvgBuildTime / 1000.0 << " s";
                }
                if (j == (int)(maxHeight / 2))
                {
                    std::cout << (minAvgBuildTime + maxAvgBuildTime) / 2000.0 << " s";
                }
                if (j == maxHeight - 1)
                {
                    std::cout << maxAvgBuildTime / 1000.0 << " s";
                }
            }
        }
        std::cout << std::endl;
        std::cout << "            ";
    }
    for (int i = maxWidth - 1; i >= 0; --i)
    {
        std::cout << "-";
    }
    std::cout << std::endl;
    std::cout << std::endl;


    // Draw max build time graph
    std::cout << "Total build times for the last " << maxWidth << " days (" << computeDateStr(tsOld) << " - " << computeDateStr(tsNow) << "):" << std::endl;
    std::cout << "            ";
    for (int j = maxHeight - 1; j >= 0; --j)
    {
        for (int i = maxWidth - 1; i >= 0; --i)
        {
            const int columnHeight = data.buildGraphData.totalBuildTimes[i] * maxHeight / (double)maxTotalBuildTime;
            if (j <= columnHeight)
            {
                std::cout << "*";
            }
            else
            {
                std::cout << " ";
            }

            if (i == 0)
            {
                std::cout << "| ";
                if (j == 0)
                {
                    std::cout << minTotalBuildTime / 1000.0 << " s";
                }
                if (j == (int)(maxHeight / 2))
                {
                    std::cout << (minTotalBuildTime + maxTotalBuildTime) / 2000.0 << " s";
                }
                if (j == maxHeight - 1)
                {
                    std::cout << maxTotalBuildTime / 1000.0 << " s";
                }
            }
        }
        std::cout << std::endl;
        std::cout << "            ";
    }
    for (int i = maxWidth - 1; i >= 0; --i)
    {
        std::cout << "-";
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

int stat(Context& context)
{
    StatOperationData data = {};
    if(readBuildTimerData(context, data) != 0)
    {
        std::cerr << "Failed to read build timer data!" << std::endl;
        return -33;
    }
    // std::cout << "Ops read from storage: " << data.ops.size() << std::endl;

    

    const int daysToCheck = 120;
    std::vector<std::pair<std::string, int64_t>> buildDays;
    buildDays.reserve(daysToCheck);
    const int64_t tsNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    for (int i = 0; i < daysToCheck; ++i)
    {
        const int64_t tsAux = (tsNow / 1000.0 - (i * 24 * 60 * 60)) ;
        data.buildGraphData.buildDates.push_back(computeDateStr(tsAux));
        data.buildGraphData.avgBuildTimes.push_back(0);
        data.buildGraphData.totalBuildTimes.push_back(0);
    }

    int i;
    for (i = 1; i < data.ops.size(); ++i)
    {
        if (data.ops[i - 1].first == Operation::START && data.ops[i].first == Operation::STOP)
        {
            StartOperationData* startData = (StartOperationData *) (data.ops[i - 1].second);
            StopOperationData* stopData = (StopOperationData *) (data.ops[i].second);
            size_t buildTime = stopData->timestamp - startData->timestamp;
            if (buildTime >= 0)
            {
                if (stopData->exitCode == "0")
                {
                    ++(data.successfulBuildCount);
                    ++(data.totalBuildCount);
                    data.totalBuildTime += buildTime;
                }
                else
                {
                    ++(data.totalBuildCount);
                }
            }
            else
            {
                ++(data.totalBuildCount);
            }

            const int64_t tsAux = startData->timestamp / 1000.0;
            const std::string dmy = computeDateStr(tsAux);
            for (int k = 0; k < data.buildGraphData.buildDates.size(); ++k)
            {
                if (dmy == data.buildGraphData.buildDates[k] && stopData->exitCode == "0")
                {
                    data.buildGraphData.totalBuildTimes[k] += buildTime;
                    data.buildGraphData.avgBuildTimes[k] += 1;
                }
            }

            if (stopData->exitCode == "0")
            {
                data.lastBuildTime = buildTime;
            }
        }
        else
        {
            if (i + 1 < data.ops.size() &&
                data.ops[i].first == Operation::START &&
                data.ops[i + 1].first == Operation::STOP)
            {
                // ignore, next iteration will count it..
            }
            else
            {
                ++(data.totalBuildCount);
            }
        }
    }
    std::cout << data.avgBuildTime << "-" <<data.totalBuildTime << "+"<< i << std::endl;
    data.avgBuildTime = data.totalBuildTime / data.successfulBuildCount;

    for (int i = 0; i < data.buildGraphData.buildDates.size(); ++i)
    {
        data.buildGraphData.avgBuildTimes[i] = data.buildGraphData.avgBuildTimes[i] != 0 ? data.buildGraphData.totalBuildTimes[i] / (double)data.buildGraphData.avgBuildTimes[i] : 0 ;
    }


    drawBuildTimeGraph(data);
    printBuildStats(data);

    return 0;
}

int takeDump(Context& context)
{
    StatOperationData data = {};
    if(readBuildTimerData(context, data) != 0)
    {
        std::cerr << "Failed to read build timer data!" << std::endl;
        return -33;
    }

    std::cout << "INDEX|OPERATION TYPE|TIMESTAMP|[Note/Exit Code]" << std::endl;
    for(int i = 0; i < data.ops.size(); ++i)
    {
        if (data.ops[i].first == Operation::START)
        {
            StartOperationData* startData = (StartOperationData *) (data.ops[i].second);
            std::cout << i << "|" << (int)data.ops[i].first << "|" << startData->timestamp << "|" << startData->note << std::endl;
        }
        else if (data.ops[i].first == Operation::STOP)
        {
            StopOperationData* stopData = (StopOperationData *) (data.ops[i].second);
            std::cout << i << "|" << (int)data.ops[i].first << "|" << stopData->timestamp << "|" << stopData->exitCode << std::endl;
        }
    }

    return 0;
}

int execute(Context& context)
{
    if (context.operation == Operation::START)
    {
        return start(context);
    }
    else if (context.operation == Operation::STOP)
    {
        return stop(context);
    }
    else if (context.operation == Operation::STAT)
    {
        return stat(context);
    }
    else if (context.operation == Operation::DUMP)
    {
        return takeDump(context);
    }

    std::cerr << "Can't execute command, unkown type!" << std::endl;
    return -1;
}
} // namespace pdrain


int main(int argc, const char** argv)
{
    std::vector<std::pair<std::string, std::string>> arguments = pdrain::ArgParser::parseArguments(argc - 1, argv + 1);
    if (arguments.size() < 1)
    {
        std::cerr << "Failed parsing arguments! Add -h for help." << std::endl;
        return -1;
    }

    pdrain::Context ctx;
    if(!init(arguments, ctx))
    {
        std::cerr << "Init failed!" << std::endl;
        return -2;
    }

    return execute(ctx);
}
