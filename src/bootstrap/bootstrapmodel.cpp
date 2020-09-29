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

#include "bootstrapmodel.h"
#include "tinyformat.h"
#include "chainparams.h"
#include "finally.h"
#include "ziputil.h"
#include "util.h"
#include "curl.h"

#include <chrono>
#include <fstream>
#include <iterator>
#include <type_traits>
#include <openssl/sha.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/all.hpp>

/** name of bootstrap file in the data directory */
static const char BOOTSTRAP_FILE_NAME[] = "bootstrap.zip" ;

/** name of bootstrap subdir in the data directory */
static const char BOOTSTRAP_DIR_NAME[] = "bootstrap" ;

/** name of bootstrap verified file */
static const char BOOTSTRAP_VERIFIED[] = "verified";

/** count number of instances of the model */
std::atomic<int> BootstrapModel::instanceNumber_(0);

/**
 * Implementation notes:
 * - probably better to protect workerThread_/instanceNumber_ by mutex to prevent possible race condition;
 */

/** convert size to the human readable string */
static std::string HumanReadableSize(int64_t size, bool si)
{
    const int unit = si ? 1000 : 1024;
    const char* units1[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    const char* units2[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    const char** units = si ? units1 : units2;

    static_assert((sizeof(units1) / sizeof(units1[0])) == (sizeof(units2) / sizeof(units2[0])), "Number of elements in units1 and units2 must be equal.");

    int i = 0;
    while (size > unit) {
       size /= unit;
       i += 1;
    }

    if (size <= 0 || i >= sizeof(units1) / sizeof(units1[0])) {
        return "0";
    } else {
        return strprintf("%.*f %s", i, size, units[i]);
    }
}

/** compute the 256-bit hash of the ifstream */
inline std::string Hash(std::ifstream& ifs)
{
    const std::size_t CHUNK = 4096;

    if (!ifs.is_open() || ifs.eof()) {
        assert(false);
        return "0000000000000000000000000000000000000000000000000000000000000000";
    } else {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);

        std::string input;
        input.resize(CHUNK, 0);
        while (!ifs.eof()) {
            ifs.read(&input[0], CHUNK);
            if (ifs.gcount() <= CHUNK) {
                SHA256_Update(&sha256, input.c_str(), ifs.gcount());
            } else {
                assert(false);
            }
        }
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }

        return ss.str();
    }
}

BootstrapModel::BootstrapModel() : bootstrapMode_(BootstrapMode::cloud),
                                   progress_(0),
                                   cancel_(false),
                                   configMerged_(false)
{
    if (instanceNumber_ > 0) {
        throw std::logic_error(strprintf("%s: only single instance of the BootstrapModel can be created", __func__));
    }

    if (!ParamsSelected()) {
        throw std::logic_error(strprintf("%s: network params has not been selected", __func__));
    }

    datadirPath_ = GetDataDir();
    if (!boost::filesystem::exists(datadirPath_)) {
        throw std::logic_error(strprintf("%s: data dir does not exist: %s", __func__, datadirPath_.string()));
    }

    instanceNumber_ += 1;
}

BootstrapModel::~BootstrapModel()
{
    instanceNumber_ -= 1;
    Cancel();
    Wait();
}

void BootstrapModel::ResetState()
{
    progress_ = 0;
    cancel_ = false;
    configMerged_ = false;
    latestRunError_.clear();
}

bool BootstrapModel::IsBootstrapRunning() const
{
    return workerThread_ && workerThread_->joinable() && !workerThread_->try_join_for(boost::chrono::milliseconds(1));
}

int BootstrapModel::GetBootstrapProgress() const
{
    return progress_;
}

BootstrapMode BootstrapModel::GetBootstrapMode() const
{
    return bootstrapMode_;
}

bool BootstrapModel::SetBootstrapMode(BootstrapMode mode, std::string& err)
{
    if (IsBootstrapRunning()) {
        err = "Bootstrap is running";
        return false;
    } else {
        bootstrapMode_ = mode;
        NotifyModelChanged();
        return true;
    }
}

boost::filesystem::path BootstrapModel::GetBootstrapFilePath() const
{
    return bootstrapFilePath_;
}

bool BootstrapModel::SetBootstrapFilePath(const boost::filesystem::path& p, std::string& err)
{
    if (IsBootstrapRunning()) {
        err = "Bootstrap is running";
        return false;
    } else {
        bootstrapFilePath_ = p;
        NotifyModelChanged();
        return true;
    }
}

bool BootstrapModel::BootstrapFilePathOk() const
{
    return boost::filesystem::exists(GetBootstrapFilePath());
}

bool BootstrapModel::IsConfigMerged() const
{
    return configMerged_;
}

bool BootstrapModel::RunStageI(std::string& err)
{
    try {
        if (IsBootstrapRunning()) {
            err = "Bootstrap is running";
            return false;
        } else if (!RunStageIPossible(err)) {
            return false;
        } else if (!CleanUp(err)) {
            return false;
        } else if (bootstrapMode_ == BootstrapMode::file) {
            workerThread_.reset(new boost::thread([this](){ RunFromFileThread(); }));
            return true;
        } else if (bootstrapMode_ == BootstrapMode::cloud) {
            workerThread_.reset(new boost::thread([this](){ RunFromCloudThread(); }));
            return true;
        } else {
            err = "Unsupported bootstrap mode";
            return false;
        }
    } catch (const boost::exception& e) {
        err = boost::diagnostic_information(e);
    } catch (const std::exception& e) {
        err = e.what();
    } catch (...) {
        err = "unexpected error";
    }

    return error("%s : %s", __func__, err);
}

bool BootstrapModel::RunStageIPossible(std::string& err) const
{
    try {
        if (IsBootstrapRunning()) {
            err = "Bootstrap is running";
            return false;
        } else if (!FreeSpaceOk(err)) {
            return false;
        } else {
            return true;
        }
    } catch (const boost::exception& e) {
        err = boost::diagnostic_information(e);
    } catch (const std::exception& e) {
        err = e.what();
    } catch (...) {
        err = "unexpected error";
    }

    return error("%s : %s", __func__, err);
}

bool BootstrapModel::RunStageII(std::string& err)
{
    try {
        if (IsBootstrapRunning()) {
            err = "Bootstrap is running";
            return false;
        } else if (!RunStageIIPossible(err)) {
            return false;
        } else {
            workerThread_.reset(new boost::thread([this](){ RunStageIIThread(); }));
            return true;
        }
    } catch (const boost::exception& e) {
        err = boost::diagnostic_information(e);
    } catch (const std::exception& e) {
        err = e.what();
    } catch (...) {
        err = "unexpected error";
    }

    return error("%s : %s", __func__, err);
}

bool BootstrapModel::RunStageIIPrepared() const
{
    return boost::filesystem::exists(datadirPath_ / BOOTSTRAP_DIR_NAME / BOOTSTRAP_VERIFIED);
}

bool BootstrapModel::RunStageIIPossible(std::string& err) const
{
    try {
        if (IsBootstrapRunning()) {
            err = "Bootstrap is running";
            return false;
        } else if (!FreeSpaceOk(err)) {
            return false;
        } else {
            return true;
        }
    } catch (const boost::exception& e) {
        err = boost::diagnostic_information(e);
    } catch (const std::exception& e) {
        err = e.what();
    } catch (...) {
        err = "unexpected error";
    }

    return error("%s : %s", __func__, err);
}

bool BootstrapModel::CleanUp(std::string& err) const
{
    try {
        if (IsBootstrapRunning()) {
            err = "Bootstrap is running";
            return false;
        } else {
            return CleanUpImpl(err);
        }
    } catch (const boost::exception& e) {
        err = boost::diagnostic_information(e);
    } catch (const std::exception& e) {
        err = e.what();
    } catch (...) {
        err = "unexpected error";
    }

    return error("%s : %s", __func__, err);
}

bool BootstrapModel::CleanUpImpl(std::string& err) const
{
    remove_all(datadirPath_ / BOOTSTRAP_DIR_NAME);
    remove(datadirPath_ / BOOTSTRAP_FILE_NAME);
    return true;
}

bool BootstrapModel::IsLatestRunSuccess(std::string& err) const
{
    Wait();
    err = latestRunError_;
    return err.empty();
}

void BootstrapModel::Wait() const
{
    if (workerThread_ && workerThread_->joinable()) {
        workerThread_->join();
    }
}

void BootstrapModel::Cancel()
{
    cancel_ = true;
}

bool BootstrapModel::IsCancelled() const
{
    return cancel_;
}

bool BootstrapModel::FreeSpaceOk(std::string& err) const
{
    boost::filesystem::space_info si = boost::filesystem::space(datadirPath_);
    if (si.available < GetBlockChainSize() * 2) {
        err = strprintf("Not enough free space avaialable in the directory: %s, required at least: %lld", datadirPath_.string(), GetBlockChainSize() * 2);
        return false;
    } else {
        return true;
    }
}

void BootstrapModel::RunFromFileThread()
{
    NotifyModelChanged(); /** model running state changed */

    std::string thName = strprintf("galilel-%s", __func__);
    RenameThread(thName.c_str());
    LogPrintf("%s thread start\n", thName);

    try {
        ResetState();
        RunFromFileImpl(GetBootstrapFilePath(), latestRunError_);
    } catch (const std::exception& e) {
        latestRunError_ = e.what();
    } catch (...) {
        latestRunError_ = "unexpected error";
    }

    if (!latestRunError_.empty()) {
        error("%s : %s", __func__, latestRunError_);
    }

    NotifyBootstrapCompletedI(latestRunError_.empty(), latestRunError_);
    NotifyModelChanged(); /** model running state changed */
    LogPrintf("%s thread exit\n", thName);
}

void BootstrapModel::RunFromCloudThread()
{
    NotifyModelChanged(); /** model running state changed */

    std::string thName = strprintf("galilel-%s", __func__);
    RenameThread(thName.c_str());
    LogPrintf("%s thread start\n", thName);

    try {
        ResetState();
        RunFromCloudImpl(latestRunError_);
    } catch (const std::exception& e) {
        latestRunError_ = e.what();
    } catch (...) {
        latestRunError_ = "unexpected error";
    }

    if (!latestRunError_.empty()) {
        error("%s : %s", __func__, latestRunError_);
    }

    NotifyBootstrapCompletedI(latestRunError_.empty(), latestRunError_);
    NotifyModelChanged(); /** model running state changed */
    LogPrintf("%s thread exit\n", thName);
}

void BootstrapModel::RunStageIIThread()
{
    NotifyModelChanged(); /** model running state changed */

    std::string thName = strprintf("galilel-%s", __func__);
    RenameThread(thName.c_str());
    LogPrintf("%s thread start\n", thName);

    try {
        ResetState();
        RunStageIIImpl(latestRunError_);
    } catch (const std::exception& e) {
        latestRunError_ = e.what();
    } catch (...) {
        latestRunError_ = "unexpected error";
    }

    if (!latestRunError_.empty()) {
        error("%s : %s", __func__, latestRunError_);
    }

    NotifyBootstrapCompletedII(latestRunError_.empty(), latestRunError_);
    NotifyModelChanged(); /** model running state changed */
    LogPrintf("%s thread exit\n", thName);
}

/** unzip archive and prepare verified file */
bool BootstrapModel::RunFromFileImpl(const boost::filesystem::path& zipPath, std::string& err)
{
    if (!boost::filesystem::exists(zipPath)) {
        err = strprintf("Path does not exist %s", zipPath.string());
        return error("%s : %s", __func__, err);
    }

    if (!VerifyZip(zipPath, err)) {
        return error("%s : %s", __func__, err);
    }

    if (!VerifySignature(zipPath, err)) {
        return error("%s : %s", __func__, err);
    }

    NotifyBootstrapProgress(strprintf("Unzipping %s...", zipPath.string()), 0);

    const boost::filesystem::path bootstrapDirPath = datadirPath_ / BOOTSTRAP_DIR_NAME;
    if (boost::filesystem::exists(bootstrapDirPath)) {
        assert(false); /** must not exist at this point */
        boost::filesystem::remove(bootstrapDirPath);
    }

    boost::filesystem::create_directory(bootstrapDirPath);
    if (!ZipExtract(zipPath.string(), bootstrapDirPath.string(), err)) {
        err = strprintf("Zip extract from %s to %s failed with error: %s", zipPath.string(), bootstrapDirPath.string(), err);
        return error("%s : %s", __func__, err);
    }

    NotifyBootstrapProgress(strprintf("Verifying %s...", bootstrapDirPath.string()), 0);

    if (!VerifyBootstrapFolder(bootstrapDirPath, err)) {
        return error("%s : %s", __func__, err);
    }

    if (!VerifyNetworkType(bootstrapDirPath, err)) {
        return error("%s : %s", __func__, err);
    }

    if (!BootstrapVerifiedCreate(zipPath, bootstrapDirPath / BOOTSTRAP_VERIFIED, err)) {
        return error("%s : %s", __func__, err);
    }

    return true;
}

/** download bootstrap and run RunFromFile() */
bool BootstrapModel::RunFromCloudImpl(std::string& err)
{
    const std::string url = Params().GetBootstrapUrl();
    if (url.empty()) {
        err = "Bootstrap URL is empty";
        return error("%s : %s", __func__, err);
    }

    boost::filesystem::path zipPath = datadirPath_ / BOOTSTRAP_FILE_NAME;
    if (boost::filesystem::exists(zipPath)) {
        assert(false); /** must not exist at this point */
        boost::filesystem::remove(zipPath);
    }

    const std::string tmpPath = strprintf("%s.tmp", zipPath.string());
    if (boost::filesystem::exists(tmpPath)) {
        assert(false); /** must not exist at this point */
        boost::filesystem::remove(tmpPath);
    }

    NotifyBootstrapProgress(strprintf("Downloading %s", url), 0);

    int speed = 0, bytes = 0;
    using time_point = std::chrono::system_clock::time_point;
    time_point tp1 = std::chrono::system_clock::now();
    bool success = CURLDownloadToFile(url, tmpPath, [this, &speed, &bytes, &tp1](double total, double now)->int {
        if (cancel_) {
            return CURL_CANCEL_DOWNLOAD;
        } else {
            time_point tp2 = std::chrono::system_clock::now();
            size_t sec = std::chrono::duration_cast<std::chrono::seconds>(tp2 - tp1).count();
            if (sec > 5) {
                speed = (now - bytes) / sec;
                bytes = now;
                tp1 = std::chrono::system_clock::now();
            }

            if (now > 0 && total > 0 && total >= now) {
                progress_ = static_cast<int>(100.0 * now / total);
            }

            std::string str = strprintf("Downloading %s (%s/s)",
                HumanReadableSize(static_cast<int>(now), false), HumanReadableSize(speed, false));

            NotifyBootstrapProgress(str, progress_);
            return CURL_CONTINUE_DOWNLOAD;
        }
    }, err);

    if (success) {
        boost::filesystem::rename(tmpPath, zipPath);
        if (!VerifyZip(zipPath, err)) {
            err = strprintf("%s\nTry to download bootstrap file manually: %s.", err, url);
            return error("%s : %s", __func__, err);
        } else {
            return RunFromFileImpl(zipPath, err);
        }
    } else {
        boost::filesystem::remove(tmpPath);
        return error("%s : %s", __func__, err);
    }
}

/** backup blockchain and replace it with bootstrap */
bool BootstrapModel::RunStageIIImpl(std::string& err)
{
    /**
     * We don't perform additional checks here because if bootstrap is
     * interrupted in the middle - it will be able to complete on the next run
     */
    const boost::filesystem::path bootstrapDirPath = datadirPath_ / BOOTSTRAP_DIR_NAME;
    if (!boost::filesystem::exists(bootstrapDirPath)) {
        err = strprintf("Path does not exist %s", bootstrapDirPath.string());
        return error("%s : %s", __func__, err);
    }

    /** replace blockchain files from bootstrap */
    const std::vector<boost::filesystem::path> dirList = GetBootstrapDirList(bootstrapDirPath);
    for (const boost::filesystem::path& dir : dirList) {
        const boost::filesystem::path originPath = datadirPath_ / dir.leaf();
        const boost::filesystem::path bakPath = originPath.string() + ".bak";

        /** remove old backup */
        if (boost::filesystem::exists(dir) && boost::filesystem::exists(originPath)) {
            remove_all(bakPath);
        }

        /** create new backup */
        if (boost::filesystem::exists(originPath) && !boost::filesystem::exists(bakPath)) {
            boost::filesystem::rename(originPath, bakPath);
        }

        /** move dir from bootstrap to origin */
        if (boost::filesystem::exists(dir)) {
            boost::filesystem::rename(dir, originPath);
        }
    }

    const boost::filesystem::path configPath = GetConfigFile();
    configMerged_ = MergeConfigFile(configPath, bootstrapDirPath / "galilel.conf");

    remove(datadirPath_ / "peers.dat");
    remove(datadirPath_ / "banlist.dat");

    return CleanUpImpl(err);
}

bool BootstrapModel::VerifyZip(const boost::filesystem::path& zipPath, std::string& err) const
{
    std::ifstream ifs(zipPath.string());
    if (!ifs.is_open()) {
        err = strprintf("Failed to open path %s", zipPath.string());
        return error("%s : %s", __func__, err);
    }

    /** auto-close */
    Finally ifsClose([&ifs](){ ifs.close(); });

    /** check it is .zip */
    char pk[3] = {0};
    ifs.read(pk, 2);
    if (std::string(pk) != "PK") {
        err = strprintf("File %s is not a valid .zip archive", zipPath.string());
        return error("%s : %s", __func__, err);
    }

    /** TODO implement signature later */
    return true;
}

bool BootstrapModel::VerifySignature(const boost::filesystem::path& zipPath, std::string& err) const
{
    /** TODO implement signature later */
    return true;
}

bool BootstrapModel::VerifyBootstrapFolder(const boost::filesystem::path& bootstrapDir, std::string& err) const
{
    const std::vector<boost::filesystem::path> dirList = GetBootstrapDirList(bootstrapDir);
    for (const boost::filesystem::path& dir : dirList) {
        if (!boost::filesystem::exists(dir)) {
            err = strprintf("Verification failed, %s does not exist", dir.string());
            return error("%s : %s", __func__, err);
        }
    }

    return true;
}

bool BootstrapModel::VerifyNetworkType(const boost::filesystem::path& bootstrapDir, std::string& err) const
{
    if (!VerifyGenesisBlock(bootstrapDir.string(), Params().HashGenesisBlock(), err)) {
        err = strprintf("Bootstrap verification failed with reason: %s", err);
        return error("%s : %s", __func__, err);
    } else {
        return true;
    }
}

bool BootstrapModel::BootstrapVerifiedCreate(const boost::filesystem::path& zipPath, const boost::filesystem::path& verifiedPath, std::string& err) const
{
    std::ofstream ofs(verifiedPath.string());
    if (ofs.is_open()) {
        Finally ofsClose([&ofs](){ ofs.close(); });

        std::string hash;
        std::ifstream ifs(zipPath.string());
        if (ifs.is_open()) {
            Finally ifsClose([&ifs](){ ifs.close(); });
            hash = Hash(ifs);
        }

        ofs << hash;
        return true;
    } else {
        err = strprintf("Unable to create %s", verifiedPath.string());
        return error("%s : %s", __func__, err);
    }
}

bool BootstrapModel::BootstrapVerifiedCheck(const boost::filesystem::path& verifiedPath, std::string& err) const
{
    if (!boost::filesystem::exists(verifiedPath)) {
        err = strprintf("Path %s does not exist", verifiedPath.string());
        return error("%s : %s", __func__, err);
    } else {
        return true;
    }
}

/** return true if merge success and false otherwise */
bool BootstrapModel::MergeConfigFile(const boost::filesystem::path& original, const boost::filesystem::path& bootstrap) const
{
    if (!boost::filesystem::exists(bootstrap)) {
        return false; /** nothing to merge */
    }

    if (!boost::filesystem::exists(original)) {
        boost::filesystem::copy(bootstrap, original);
        return true; /** just raw copy, return true */
    }

    const boost::filesystem::path originalBackup = original.string() + ".bak";

    /** remove old backup of the original config */
    boost::filesystem::remove(originalBackup);

    /** make new backup of the original config */
    boost::filesystem::rename(original, originalBackup);

    std::ifstream oldConf(originalBackup.string());
    if (!oldConf.is_open()) {
        error("%s : Failed to open %s", __func__, originalBackup.string());
        return false;
    }
    Finally oldConfClose([&oldConf](){ oldConf.close(); });

    std::ofstream newConf(original.string());
    if (!newConf.is_open()) {
        error("%s : Failed to open %s", __func__, original.string());
        return false;
    }
    Finally newConfClose([&newConf](){ newConf.close(); });

    /** read all lines from the original config except 'addnode' */
    std::string line;
    while (std::getline(oldConf, line)) {
        if (line.find("addnode") == std::string::npos) {
            newConf << line << std::endl;
        }
    }

    /** append all lines from the bootstrap config */
    std::ifstream bootstrapConf(bootstrap.string());
    Finally bootstrapConfClose([&bootstrapConf](){ bootstrapConf.close(); });
    while (std::getline(bootstrapConf, line)) {
        newConf << line << std::endl;
    }

    return true; /** merge success */
}

std::vector<boost::filesystem::path> BootstrapModel::GetBootstrapDirList(const boost::filesystem::path& bootstrapDir) const
{
    return {
        bootstrapDir / "blocks",
        bootstrapDir / "chainstate"
    };
}
