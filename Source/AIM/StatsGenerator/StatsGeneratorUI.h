/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _STATSGENERATOR_H_
#define _STATSGENERATOR_H_

#include <ui_StatsGeneratorUI.h>

#include <QtCore/QObject>

#include <QtGui/QMainWindow>

class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotMarker;
/**
 * @class StatsGeneratorUI StatsGeneratorUI.h AIM/StatsGenerator/StatsGeneratorUI.h
 * @brief
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Jan 4, 2011
 * @version 1.0
 */
class StatsGeneratorUI : public QMainWindow, private Ui::StatsGeneratorUI
{
  Q_OBJECT
    ;

  public:
    StatsGeneratorUI(QWidget *parent = 0);
    virtual ~StatsGeneratorUI();

    void plotSizeDistribution();


    protected slots:

      /* Menu Slots */
      void on_actionClose_triggered();
      void on_actionExit_triggered();


      void on_m_Mu_SizeDistribution_textChanged(const QString &text);
      void on_m_Sigma_SizeDistribution_textChanged(const QString &text);
      void on_m_SigmaCutOff_SizeDistribution_textChanged(const QString &text);


  protected:

    /**
     * @brief Implements the CloseEvent to Quit the application and write settings
     * to the preference file
     */
    void closeEvent(QCloseEvent *event);

    /**
     * @brief Reads the Preferences from the users pref file
     */
    void readSettings();

    /**
     * @brief Writes the preferences to the users pref file
     */
    void writeSettings();

    /**
     * @brief Initializes some of the GUI elements with selections or other GUI related items
     */
    void setupGui();

    /**
     * @brief Checks the currently open file for changes that need to be saved
     * @return
     */
    qint32 checkDirtyDocument();

    /**
     * @brief Enables or Disables all the widgets in a list
     * @param b
     */
    void setWidgetListEnabled(bool b);

    /**
     * @brief Verifies that a path exists on the file system.
     * @param outFilePath The file path to check
     * @param lineEdit The QLineEdit object to modify visuals of (Usually by placing a red line around the QLineEdit widget)
     */
    bool verifyPathExists(QString outFilePath, QLineEdit* lineEdit);

    /**
     * @brief Verifies that a parent path exists on the file system.
     * @param outFilePath The parent file path to check
     * @param lineEdit The QLineEdit object to modify visuals of (Usually by placing a red line around the QLineEdit widget)
     */
    bool verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit);

    /**
     * @brief Opens an Image file
     * @param imageFile The path to the image file to open.
     */
    void openFile(QString imageFile);

    /**
     * @brief Enables or disables the various PlotWidgetTabs
     * @param b Enable or disable the plotwidgets
     */
    void setTabsPlotTabsEnabled(bool b);


private slots:

    /**
     * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
     * should be connected to the Signal QRecentFileList->fileListChanged
     * @param file The newly added file.
     */
    void updateRecentFileList(const QString &file);

    /**
     * @brief Qt Slot that fires in response to a click on a "Recent File' Menu entry.
     */
    void openRecentFile();

  private:
    QList<QWidget*> m_WidgetList;

    QwtPlotCurve* m_SizeDistributionCurve;
    QwtPlotMarker* m_CutOffMin;
    QwtPlotMarker* m_CutOffMax;

//    QwtPlotZoomer* m_zoomer;
//    QwtPlotPicker* m_picker;
//    QwtPlotPanner* m_panner;
    QwtPlotGrid*   m_grid;

    QString m_OpenDialogLastDirectory;  // Must be last in the list
    StatsGeneratorUI(const StatsGeneratorUI&); // Copy Constructor Not Implemented
    void operator=(const StatsGeneratorUI&); // Operator '=' Not Implemented
};

#endif /* _STATSGENERATOR_H_ */
