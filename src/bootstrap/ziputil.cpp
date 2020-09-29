/**
 * Copyright (c) 2015-2018 COLX Developers
 * Copyright (c) 2018-2019 Galilel Developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ziputil.h"
#include "tinyformat.h"

#include <minizip.h>

#include <cassert>
#include <exception>
#include <boost/filesystem.hpp>

static void AppendArg(const std::string& arg, std::string& arguments)
{
    arguments.append(arg);
    arguments.push_back('\0');
}

static std::vector<const char*> BuildArgv(const std::string& arguments)
{
    std::vector<const char*> argv;

    /** this algo skips two empty arguments followed by each other */
    for (size_t i = 0; i < arguments.size(); ++i) {
        argv.push_back(&arguments[i]);
        i = arguments.find('\0', i + 1);
    }

    return argv;
}

static std::string strError;
static void AppendStr(const char *s)
{
    assert(s);
    if (s) {
      strError.append(s);
    }
}

bool ZipExtract(const std::string& strInputPath, const std::string& strOutputPath, std::string& err)
{
    try {
        if (!boost::filesystem::exists(strInputPath)) {
            err = strprintf("Path %s does not exist", strInputPath);
            return false;
        }

        if (!boost::filesystem::exists(strOutputPath)) {
            err = strprintf("Path %s does not exist", strOutputPath);
            return false;
        }

        if (!boost::filesystem::is_directory(strOutputPath)) {
            err = strprintf("Path %s is not a directory", strOutputPath);
            return false;
        }

        std::string arguments;
        AppendArg("", arguments);
        AppendArg("-x", arguments);
        AppendArg("-o", arguments);
        AppendArg(strInputPath, arguments);
        AppendArg("-d", arguments);
        AppendArg(strOutputPath, arguments);

        std::vector<const char*> argv = BuildArgv(arguments);

        strError.clear();
        boost::filesystem::path cwd = boost::filesystem::current_path();
        int res = main_unzip(argv.size(), &argv[0], AppendStr);
        boost::filesystem::current_path(cwd);

        if (0 == res) {
            return true;
        } else {
            err = strError;
            return false;
        }
    } catch (const std::exception& e) {
        err = e.what();
        return false;
    }
}
