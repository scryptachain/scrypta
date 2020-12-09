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

#ifndef GALI_BOOTSTRAPDIALOG_H
#define GALI_BOOTSTRAPDIALOG_H

#include <QDialog>

#include <memory>

class QLabel;
class QRadioButton;
class QPushButton;
class QLineEdit;
class QProgressBar;

class BootstrapModel;
typedef std::shared_ptr<BootstrapModel> BootstrapModelPtr;

class BootstrapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BootstrapDialog(BootstrapModelPtr model, QWidget *parent = nullptr);

    virtual ~BootstrapDialog();

    /**
      * Notify dialog that it is running at the app startup
     */
    void SetStartupFlag();

    /**
      * Initiate bootstrap process (intended to use at the app startup).
      * On success perform two stages of the bootstrap algorithm (see BootstrapModel for details).
      * If this method failed - wallet should be restarted.
     */
    static void bootstrapBlockchain(BootstrapModelPtr model);

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

    void ShowError(const std::string& msg);

private:
    void setupUI();
    void setupLayout();
    void updateUI();
    bool cancel();

private slots:
    void onButtonSelectFile();
    void onButtonRun();
    void onButtonHide();
    void onButtonCancel();
    void onButtonRadioCloud();
    void onButtonRadioFile();
    void onEditFilePathChanged(const QString& text);
    void onBootstrapModelChanged();
    void onBootstrapProgress(const QString& title, int nProgress);
    void onBootstrapCompleted(bool success, const QString& error);

private:
    BootstrapModelPtr model_;
    bool startup_ = false;

    struct {
        QLabel *labelTitle;
        QRadioButton *radioCloud, *radioFile;
        QLineEdit *editFilePath;
        QPushButton *btnSelectFile;
        QPushButton *btnRun, *btnHide, *btnCancel;
        QProgressBar *progress;
        QLabel *labelLink;
    } ui;
};

#endif /** GALI_BOOTSTRAPDIALOG_H */
