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

#include "curl.h"
#include "util.h"
#include "finally.h"
#include "clientversion.h"

#include <stdlib.h>
#include <curl/curl.h>

/** private section */
static void CurlGlobalInit()
{
    static bool initialized = false;
    if (!initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        initialized = true;
    }
}

struct CurlScopeInit
{
    CurlScopeInit() {
        CurlGlobalInit();
        curl_ = curl_easy_init();
    }

    ~CurlScopeInit() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
    }

    inline CURL* instance() { return curl_; }
    inline operator CURL*() { return curl_; }

private:
    CurlScopeInit(const CurlScopeInit&);
    CurlScopeInit& operator = (const CurlScopeInit&);

private:
    CURL *curl_;
};

struct MemoryBuffer
{
    MemoryBuffer():
        memory(nullptr), size(0) {}

    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryBuffer *mem = (struct MemoryBuffer*)userp;

    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {

        /** out of memory! */
        error("%s: not enough memory (realloc returned NULL)\n", __func__);
        return 0;
    } else {
        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
        return realsize;
    }
}

static size_t WriteFileCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t written = fwrite(contents, size, nmemb, (FILE*)userp);
  return written;
}

struct ProgressData
{
    ProgressData():
        download(true), fn(nullptr) {}

    bool download;
    ProgressReport fn;
};

static int ProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (clientp) {
        ProgressData *pd = reinterpret_cast<ProgressData*>(clientp);
        if (pd->download) {
            return pd->fn(dltotal, dlnow);
        } else {
            return pd->fn(ultotal, ulnow);
        }
    } else {
        return 0;
    }
}

/** public API */
bool CURLGetRedirect(const CUrl& url, CUrl& redirect, std::string& error)
{
    CurlScopeInit curl;
    CURLcode res = CURLE_OK;
    char *location = nullptr;
    long response_code = 0;

    redirect.clear();
    error.clear();

    if (url.empty()) {
        error = "url is empty";
        return false;
    }

    if (!curl.instance()) {
        error = "curl init failed";
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /** disable peer and host verification because of issues on Mac and Win */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        error = strprintf("curl_easy_perform failed: %s", curl_easy_strerror(res));
        return false;
    }

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if ((res == CURLE_OK) && ((response_code / 100) != 3)) {

        /** a redirect implies a 3xx response code */
        error = "Not a redirect";
        return false;
    } else {
        res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &location);
        if ((res == CURLE_OK) && location) {

            /**
             * This is the new absolute URL that you could redirect to, even if
             * the Location: response header may have been a relative URL.
             */
            redirect = location;
            return true;
        } else {
            error = strprintf("curl_easy_getinfo failed: %s", curl_easy_strerror(res));
            return false;
        }
    }
}

bool CURLDownloadToMem(const CUrl& url, ProgressReport fn, std::string& buff, std::string& error)
{
    CurlScopeInit curl;
    CURLcode res;

    buff.clear();
    error.clear();

    if (url.empty()) {
        error = "url is empty";
        return false;
    }

    if (!curl.instance()) {
        error = "curl init failed";
        return false;
    }

    struct MemoryBuffer chunk;

    /** will be grown as needed by the realloc above */
    chunk.memory = (char*)malloc(1);

    /** no data at this point */
    chunk.size = 0;

    Finally chunkFree([&chunk](){ if (chunk.memory) free(chunk.memory); });

    /** specify URL to get */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /** send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /** we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    /** some servers don't like requests that are made without a user-agent */
    const std::string agent = FormatFullVersion();
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

    /** disable peer and host verification because of issues on Mac and Win */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /** follow max 3 redirects */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

    ProgressData pd;
    pd.download = true;
    pd.fn = fn;

    /** progress meter */
    if (fn) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &pd);
    }

    /** get it! */
    res = curl_easy_perform(curl);

    /** check for errors */
    if (res != CURLE_OK) {
        error = strprintf("Download %s failed with error: %s", url, curl_easy_strerror(res));
    } else {
        long response_code = 0;
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (res != CURLE_OK) {
            error = strprintf("Download %s failed with error: %s", url, curl_easy_strerror(res));
        } else if ((response_code / 100) != 2) {
            error = strprintf("Download %s failed with HTTP code: %ld.", url, response_code);
        } else {

            /**
             * Now, our chunk.memory points to a memory block that is chunk.size
             * bytes big and contains the remote file.
             */
            LogPrintf("%s: %lu bytes retrieved\n", __func__, (long)chunk.size);
            buff.assign(chunk.memory, chunk.size);
        }
    }

    return !buff.empty();
}

bool CURLDownloadToFile(const CUrl& url, const std::string& path, ProgressReport fn, std::string& error)
{
    CurlScopeInit curl;
    error.clear();

    if (url.empty()) {
        error = "url is empty";
        return false;
    }

    if (path.empty()) {
        error = "path is empty";
        return false;
    }

    if (!curl.instance()) {
        error = "curl init failed";
        return false;
    }

    /** specify URL to get */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /** send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);

    /** open the file */
    FILE *pagefile = fopen(path.c_str(), "wb");
    if (!pagefile) {
        error = strprintf("failed to create file: %s", path);
        return false;
    }

    /** close pagefile at function exit */
    Finally pagefileClose([&pagefile](){ fclose(pagefile); });

    /** write the page body to this file handle */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

    /** some servers don't like requests that are made without a user-agent */
    const std::string agent = FormatFullVersion();
    curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

    /** disable peer and host verification because of issues on Mac and Win */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    /** follow max 3 redirects */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

    ProgressData pd;
    pd.download = true;
    pd.fn = fn;

    /** progress meter */
    if (fn) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &pd);
    }

    /** get it! */
    CURLcode res = curl_easy_perform(curl);

    /** check for errors */
    if (res != CURLE_OK) {
        error = strprintf("Download %s failed with error: %s", url, curl_easy_strerror(res));
    } else {
        long response_code = 0;
        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (res != CURLE_OK) {
            error = strprintf("Download %s failed with error: %s", url, curl_easy_strerror(res));
        } else if ((response_code / 100) != 2) {
            error = strprintf("Download %s failed with HTTP code: %ld.", url, response_code);
        } else; /** success */
    }

    return CURLE_OK == res;
}
