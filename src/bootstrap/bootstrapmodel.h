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

#ifndef LYRA_BOOTSTRAPMODEL_H
#define LYRA_BOOTSTRAPMODEL_H

#include <atomic>
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>

/**
 * @brief The BootstrapMode enumerates all possible ways of bootstrapping
 */
enum class BootstrapMode
{
    cloud,  /** official url */
    file    /** official .zip archive */
};

/**
 * @brief The BootstrapModel class is responsible for bootstraping blockchain from snapshot (.zip archive of datadir).
 * Additionally peers.dat and banlist.dat are being removed to refresh connections to the network.
 * Config file galilel.conf is merged if provided in snapshot.
 *
 * Blockchain snapshot is .zip archive that contains all required files and directories, for now it is:
 * /blocks
 * /chainstate
 * galilel.conf (optional)
 *
 * Optionally .zip archive can contain signed checksum in the end of the file.
 * TODO: more info about data format and algorithm.
 *
 * Bootstrap process consists of two stages, first stage is initiated by user and goes through the next steps:
 * - remove /bootstrap folder and .zip archive if exists in data directory
 * - make sure there is enough free space
 * - download .zip archive from the cloud and save into data directory or select .zip file in the filesystem
 * - verify .zip archive checksum and signature
 * - unpack .zip archive into the data directory into /bootstrap folder
 * - verify content of the /bootstrap folder (there are all required subdirectories)
 * - verify that network is correct (testnet or mainnet)
 * - merge .conf file: remove all -addnode entries, append content from the bootstrap .conf
 * - create file /bootstrap/verified
 * - restart wallet or ask user to restart now or later
 *
 * Second stage steps perform at the wallet startup:
 * - if there is no /bootstrap folder - proceed wallet startup as usual
 * - if there is /bootstrap folder and there is no file named 'verified' - remove /bootstrap folder and proceed wallet startup as usual
 * - if there is file named /bootstrap/verified - perform steps below
 * - if directory exists in in both places (data directory and /bootstrap folder) perform next operations on it:
 *   - remove .bak directory or file
 *   - rename directories: blocks->blocks.bak, chainstate->chainstate.bak, .conf->conf.bak
 *   - move directory from /bootstrap=>data directory
 * - remove peers.dat and banlist.dat
 * - remove folder /bootstrap and .zip archive in data directory
 */
class BootstrapModel
{
public:
    BootstrapModel();
    ~BootstrapModel();

    bool IsBootstrapRunning() const;
    int GetBootstrapProgress() const;

    BootstrapMode GetBootstrapMode() const;
    bool SetBootstrapMode(BootstrapMode mode, std::string& err);

    boost::filesystem::path GetBootstrapFilePath() const;
    bool SetBootstrapFilePath(const boost::filesystem::path& p, std::string& err);

    /**
     * @brief Check bootstrap file exists at given path `SetBootstrapFilePath`
     *
     * @return true - ok, false - not ok
     */
    bool BootstrapFilePathOk() const;

    /**
     * @brief Check if config file was merged during bootstrap.
     *
     * @return true - merged, false - not merged
     */
    bool IsConfigMerged() const;

    /**
     * @brief Run stage I algorithm.
     *        Returns immediately, algorithm runs in background thread.
     * @param err error description on fail
     * @return true - success, false - failed
     */
    bool RunStageI(std::string& err);

    /**
     * @brief Check stage I pre-conditions
     * @param err error description on fail
     * @return true - yes, false - no,
     */
    bool RunStageIPossible(std::string& err) const;

    /**
     * @brief Run stage II algorithm.
     *        Returns immediately, algorithm runs in background thread.
     * @param err error description on fail
     * @return true - success, false - failed
     */
    bool RunStageII(std::string& err);

    /**
     * @brief Check stage II pre-conditions
     * @param err error description on fail
     * @return true - yes, false - no,
     */
    bool RunStageIIPossible(std::string& err) const;

    /**
     * @brief Check if verified file exists and is correct
     * @param err error description on fail
     * @return true - yes, false - no,
     */
    bool RunStageIIPrepared() const;

    /**
     * @brief Remove temporary files from previous try of bootstrap
     * @param err error description on fail
     * @return true - success, false - failed
     */
    bool CleanUp(std::string& err) const;

    /**
     * @brief Return status of the latest async operation RunStageI/RunStageII.
     *        If operation is in progress - wait for completion!!!
     * @param err error description on fail
     * @return true - success, false - failed
     */
    bool IsLatestRunSuccess(std::string& err) const;

    /**
     * @brief Block current thread until bootstrap task is finished.
     *        Returns immediately if task is not running.
     */
    void Wait() const;

    /**
     * @brief Request bootstrap task to cancel.
     *        Returns immediately, you have to wait until task is finished.
     */
    void Cancel();

    /**
     * @brief Return true if task was cancelled.
     */
    bool IsCancelled() const;

    /** Notify that model has been changed: running state, mode, path */
    boost::signals2::signal<void(void)> NotifyModelChanged;

    /** Notify running progress for downloading/extracting bootstrap */
    boost::signals2::signal<void(const std::string& title, int nProgress)> NotifyBootstrapProgress;

    /** Notify that latest async operation RunStageI has just completed */
    boost::signals2::signal<void(bool succes, const std::string& error)> NotifyBootstrapCompletedI;

    /** Notify that latest async operation RunStageII has just completed */
    boost::signals2::signal<void(bool succes, const std::string& error)> NotifyBootstrapCompletedII;

private:
    void ResetState();
    bool FreeSpaceOk(std::string& err) const;
    void RunFromFileThread();
    void RunFromCloudThread();
    void RunStageIIThread();
    bool RunFromFileImpl(const boost::filesystem::path& zipPath, std::string& err);
    bool RunFromCloudImpl(std::string& err);
    bool RunStageIIImpl(std::string& err);
    bool CleanUpImpl(std::string& err) const;
    bool VerifyZip(const boost::filesystem::path& zipPath, std::string& err) const;
    bool VerifySignature(const boost::filesystem::path& zipPath, std::string& err) const;
    bool VerifyBootstrapFolder(const boost::filesystem::path& bootstrapDir, std::string& err) const;
    bool VerifyNetworkType(const boost::filesystem::path& bootstrapDir, std::string& err) const;
    bool BootstrapVerifiedCreate(const boost::filesystem::path& zipPath, const boost::filesystem::path& verifiedPath, std::string& err) const;
    bool BootstrapVerifiedCheck(const boost::filesystem::path& verifiedPath, std::string& err) const;
    bool MergeConfigFile(const boost::filesystem::path& original, const boost::filesystem::path& bootstrap) const;
    std::vector<boost::filesystem::path> GetBootstrapDirList(const boost::filesystem::path& bootstrapDir) const;

private:
    BootstrapMode bootstrapMode_;               /** work mode selected by the user */
    boost::filesystem::path datadirPath_;       /** network specific data dir */
    boost::filesystem::path bootstrapFilePath_; /** path selected by the user */

    std::atomic<int> progress_;
    std::atomic<bool> cancel_;                  /** user can interrupt bootstrap task */
    std::atomic<bool> configMerged_;            /** true if config file was merged during bootstrap */
    std::string latestRunError_;
    std::unique_ptr<boost::thread> workerThread_;

    static std::atomic<int> instanceNumber_;    /** only single instance of this model is allowed */
};

#endif /** LYRA_BOOTSTRAPMODEL_H */
