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

#ifndef LYRA_CURL_H
#define LYRA_CURL_H

#include <string>
#include <functional>

typedef std::string CUrl;
#define CURL_CANCEL_DOWNLOAD 1
#define CURL_CONTINUE_DOWNLOAD 0

/**
 * Progress callback.
 * @param[in] total download/upload size total
 * @param[in] now downlod/upload size by now
 * @return 1 - stop download, 0 - continue download
 */
typedef std::function<int (double total, double now)> ProgressReport;

/**
 * Extract URL to redirect to.
 * @param[in] url input http(s) address
 * @param[out] redirect output http(s) address
 * @param[out] error brief description on fail
 * @return true - success, false - fail
 */
bool CURLGetRedirect(const CUrl& url, CUrl& redirect, std::string& error);

/**
 * Download content to the memory buffer.
 * @param[in] url input http(s) address
 * @param[in] fn progress report function
 * @param[out] buff output buffer
 * @param[out] error brief description on fail
 * @return true - success, false - fail
 */
bool CURLDownloadToMem(const CUrl& url, ProgressReport fn, std::string& buff, std::string& error);

/**
 * Download content to the file by given path.
 * @param[in] url input http(s) address
 * @param[in] path path to the file where to save content
 * @param[in] fn progress report function
 * @param[out] error brief description on fail
 * @return true - success, false - fail
 */
bool CURLDownloadToFile(const CUrl& url, const std::string& path, ProgressReport fn, std::string& error);

#endif // LYRA_CURL_H
