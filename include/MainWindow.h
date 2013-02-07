/*
 * Copyright 2013 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Martin Preisler <mpreisle@redhat.com>
 */

#ifndef SCAP_WORKBENCH_MAIN_WINDOW_H_
#define SCAP_WORKBENCH_MAIN_WINDOW_H_

#include "ForwardDecls.h"

#include <QMainWindow>
#include <QThread>

extern "C"
{
#include <xccdf_benchmark.h>
}

#include "ui_MainWindow.h"

/**
 * The central "singleton without global access" class representing
 * aplication's main window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(QWidget* parent = 0);
        virtual ~MainWindow();

    public slots:
        /**
         * @brief Clears everything produced during the scan
         */
        void clearResults();

        /**
         * @brief Opens a specific file
         */
        void openFile(const QString& path);

        /**
         * @brief Opens a file dialog and makes user select a file or exit the app
         *
         * The file dialog keeps opening until a file is chosen or user
         * pressed Cancel.
         */
        void openFileDialog();

        /**
         * @brief Starts scanning in a separate thread and returns
         *
         * @see MainWindow::cancelScanAsync()
         *
         * This method asserts that session has already been loaded
         * and that scanning is not running currently (in other words:
         * scanning has to end or be canceled before scanAsync can be
         * called again).
         */
        void scanAsync();

        /**
         * @brief Cancels scanning in separate thread
         *
         * This method asserts that session has already been loaded.
         * It is not recommended but you can call this method even if scan
         * is not running at the time. The reason why this is handled is to
         * deal with scan finished/canceled race that could happen (at least in theory).
         */
        void cancelScanAsync();

    private:
        /**
         * @brief Closes currently opened file (if any) and resets the interface
         *
         * If you want to make the editor close current file and make the
         * user open a new one, use MainWindow::openFileDialog, this method is
         * intended to be used internally.
         *
         * @see MainWindow::openFileDialog
         */
        void closeFile();

        /**
         * @brief Reloads the session, datastream split is potentially done again
         *
         * The main purpose of this method is to allow to reload the session when
         * parameters that affect "loading" of the session change. These parameters
         * are mainly datastream ID and component ID.
         */
        void reloadSession();

        /**
         * @brief Refreshes items of the profile combobox with data from the session
         *
         * @note This method does attempt to "keep" the previous selection if possible.
         */
        void refreshProfiles();

        /**
         * @brief Destroys the scanning thread and associated data
         *
         * Also resets UI to a state where scanning is not running.
         */
        void cleanupScanThread();

        /// UI designed in Qt Designer
        Ui_MainWindow mUI;

        /// This is our central point of interaction with openscap
        struct xccdf_session* mSession;

        /// Thread that handles scanning, NULL if no scanning is underway
        QThread* mScanThread;
        /**
         * This is a scanner suitable for scanning target as specified by user
         * @see Scanner
         */
        Scanner* mScanner;

        /// Qt Dialog that displays results and allows user to save them
        ResultViewer* mResultViewer;

    signals:
        /**
         * @brief This is signaled when scanning is canceled
         *
         * Qt handles thread messaging for us via the slot & signal mechanism.
         * The event loop of MainWindow runs in one thread, the event loop of
         * the scanner runs in another thread. Both are basically synchronisation
         * queues. This is why we emit this signal instead of calling scanner's
         * methods directly.
         *
         * Instead of emitting this signal directly, please use
         * MainWindow::cancelScanAsync()
         */
        void cancelScan();

    private slots:
        /// Checklist changed, we might have to reload session
        void checklistComboboxChanged(const QString& text);
        /// Profile change, we simply change the profile id in the session
        void profileComboboxChanged(int index);

        /**
         * @brief This slot gets triggered by the scanner to notify of a new result
         *
         * Used for progress estimation.
         */
        void scanProgressReport(const QString& rule_id, const QString& result);

        /**
         * @brief Scanner triggers this after cancelation is complete
         */
        void scanCanceled();

        /**
         * @brief Scanner triggers this after scan successfuly finishes
         */
        void scanFinished();

        /**
         * @brief When triggered, the ResultViewer is shown as a modal dialog
         *
         * Sole application control is passed to ResultViewer until user closes
         * it. It is not destroyed upon closing, just hidden.
         */
        void showResults();
};

#endif