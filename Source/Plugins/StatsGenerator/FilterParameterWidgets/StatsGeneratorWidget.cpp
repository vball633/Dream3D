/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#include "StatsGeneratorWidget.h"


#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QVector>
#include <QtCore/QJsonDocument>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QProgressDialog>


#include "H5Support/H5Utilities.h"
#include "H5Support/H5Lite.h"
#include "H5Support/HDF5ScopedFileSentinel.h"

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/DataArrays/StatsDataArray.h"
#include "SIMPLib/StatsData/PrimaryStatsData.h"
#include "SIMPLib/StatsData/PrecipitateStatsData.h"
#include "SIMPLib/StatsData/TransformationStatsData.h"
#include "SIMPLib/StatsData/BoundaryStatsData.h"
#include "SIMPLib/StatsData/MatrixStatsData.h"

#include "SIMPLib/Common/FilterPipeline.h"
#include "SIMPLib/CoreFilters/DataContainerWriter.h"
#include "SIMPLib/CoreFilters/DataContainerReader.h"

#include "SVWidgetsLib/QtSupport/QtSApplicationAboutBoxDialog.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSHelpDialog.h"

#include "Applications/SIMPLView/SIMPLViewApplication.h"

#include "StatsGenerator/Widgets/EditPhaseDialog.h"
#include "StatsGenerator/FilterParameters/StatsGeneratorFilterParameter.h"
#include "StatsGenerator/StatsGeneratorFilters/StatsGeneratorFilter.h"


// Include the MOC generated CPP file which has all the QMetaObject methods/data
//#include "moc_StatsGeneratorWidget.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGeneratorWidget::StatsGeneratorWidget(FilterParameter* parameter, AbstractFilter* filter, QWidget* parent) :
  FilterParameterWidget(parameter, filter, parent)
{
  m_FilterParameter = dynamic_cast<StatsGeneratorFilterParameter*>(parameter);
  Q_ASSERT_X(m_FilterParameter != NULL, "NULL Pointer", "StatsGeneratorFilterWidget can ONLY be used with an StatsGeneratorFilterParameter object");

  m_Filter = dynamic_cast<StatsGeneratorFilter*>(filter);
  Q_ASSERT_X(m_Filter != NULL, "NULL Pointer", "StatsGeneratorFilterWidget can ONLY be used with an StatsGeneratorFilter filter");

  m_OpenDialogLastDirectory = QDir::homePath();
  setWidgetIsExpanding(true);
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGeneratorWidget::~StatsGeneratorWidget()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::setupGui()
{
  // Hide the debugging buttons
  updatePipelineBtn->hide();
  saveH5Btn->hide();
  saveJsonBtn->hide();
  //openStatsFile->hide();

  // Catch when the filter is about to execute the preflight
  connect(getFilter(), SIGNAL(preflightAboutToExecute()),
          this, SLOT(beforePreflight()));

  // Catch when the filter is finished running the preflight
  connect(getFilter(), SIGNAL(preflightExecuted()),
          this, SLOT(afterPreflight()));

  // Catch when the filter wants its values updated
  connect(getFilter(), SIGNAL(updateFilterParameters(AbstractFilter*)),
          this, SLOT(filterNeedsInputParameters(AbstractFilter*)));

  phaseTabs->clear();

  StatsDataArray::Pointer sda = m_Filter->getStatsDataArray();
  if( (sda && sda->getNumberOfTuples() == 0) || !sda)
  {
    PrimaryPhaseWidget* ppw = new PrimaryPhaseWidget;
    ppw->setPhaseIndex(1);
    ppw->setPhaseType(SIMPL::PhaseType::PrimaryPhase);
    ppw->setCrystalStructure(Ebsd::CrystalStructure::Cubic_High);
    ppw->setPhaseFraction(1.0);
    ppw->setTotalPhaseFraction(1.0);
    phaseTabs->addTab(ppw, "Primary Phase");

    connect(ppw, SIGNAL(phaseParametersChanged()),
            this, SIGNAL(parametersChanged()));
  }
  else
  {
    size_t ensembles = sda->getNumberOfTuples();
    QProgressDialog progress("Opening Stats File....", "Cancel", 0, ensembles, this);
    progress.setWindowModality(Qt::WindowModal);

    QVector<size_t> tDims(1, ensembles);
    AttributeMatrix::Pointer cellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::AttributeMatrixType::CellEnsemble);
    cellEnsembleAttrMat->addAttributeArray(sda->getName(), sda);
    UInt32ArrayType::Pointer phaseTypes = m_Filter->getPhaseTypes();
    cellEnsembleAttrMat->addAttributeArray(phaseTypes->getName(), phaseTypes);
    UInt32ArrayType::Pointer crystalStructures = m_Filter->getCrystalStructures();
    cellEnsembleAttrMat->addAttributeArray(crystalStructures->getName(), crystalStructures);

    for (size_t phase = 1; phase < ensembles; ++phase)
    {
      progress.setValue(phase);

      if (progress.wasCanceled())
      {
        return;
      }
      StatsData::Pointer statsData = sda->getStatsData(phase);

      if(phaseTypes->getValue(phase) == SIMPL::PhaseType::BoundaryPhase)
      {
        progress.setLabelText("Opening Boundaray Phase...");
        BoundaryPhaseWidget* w = new BoundaryPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
      }
      else if(phaseTypes->getValue(phase) == SIMPL::PhaseType::MatrixPhase)
      {
        progress.setLabelText("Opening Matrix Phase...");
        MatrixPhaseWidget* w = new MatrixPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
      }
      if(phaseTypes->getValue(phase) == SIMPL::PhaseType::PrecipitatePhase)
      {
        progress.setLabelText("Opening Precipitate Phase...");
        PrecipitatePhaseWidget* w = new PrecipitatePhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
      }
      if(phaseTypes->getValue(phase) == SIMPL::PhaseType::PrimaryPhase)
      {
        progress.setLabelText("Opening Primary Phase...");
        PrimaryPhaseWidget* w = new PrimaryPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
      }
      if(phaseTypes->getValue(phase) == SIMPL::PhaseType::TransformationPhase)
      {
        progress.setLabelText("Opening Transformation Phase...");
        TransformationPhaseWidget* w = new TransformationPhaseWidget(this);
        phaseTabs->addTab(w, w->getTabTitle());
        w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
      }
      else
      {

      }
    }
    progress.setValue(ensembles);

    // Now delete the first Phase from the Combo which was left over from something else
    phaseTabs->setCurrentIndex(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_updatePipelineBtn_clicked()
{
  emit parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::filterNeedsInputParameters(AbstractFilter* filter)
{
  StatsGeneratorFilter* statsGenFilter = dynamic_cast<StatsGeneratorFilter*>(filter);

  if (NULL != statsGenFilter)
  {
    DataContainerArray::Pointer dca = generateDataContainerArray();
    DataContainer::Pointer dc = dca->getDataContainer(SIMPL::Defaults::StatsGenerator);
    AttributeMatrix::Pointer cellEnsembleAttrMat = dc->getAttributeMatrix(SIMPL::Defaults::CellEnsembleAttributeMatrixName);
    IDataArray::Pointer iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::Statistics);

    StatsDataArray::Pointer statsDataArray = std::dynamic_pointer_cast<StatsDataArray>(iDataArray);

    iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
    UInt32ArrayType::Pointer crystalStructures = std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray);

    iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
    UInt32ArrayType::Pointer phaseTypes = std::dynamic_pointer_cast<UInt32ArrayType>(iDataArray);

    statsGenFilter->setStatsDataArray(statsDataArray);
    statsGenFilter->setCrystalStructures(crystalStructures);
    statsGenFilter->setPhaseTypes(phaseTypes);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::beforePreflight()
{
 for (int i = 0; i < phaseTabs->count(); i++)
 {
   SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->widget(i));
   if (!qobject_cast<MatrixPhaseWidget*>(sgwidget) &&
       !qobject_cast<BoundaryPhaseWidget*>(sgwidget) &&
       !qobject_cast<TransformationPhaseWidget*>(sgwidget))
   {
     if (!sgwidget->getDataHasBeenGenerated())
     {
       m_Filter->setErrorCondition(-1);
       QString ss = QObject::tr("Statistics data needs to be generated for phase %1 (%2)\n"
                                "Click the Create Data button to generate the statistics data").arg(sgwidget->getPhaseIndex()).arg(sgwidget->getTabTitle());
       m_Filter->notifyErrorMessage(m_Filter->getHumanLabel(), ss, m_Filter->getErrorCondition());
     }
     if (sgwidget->getBulkLoadFailure())
     {
       m_Filter->setErrorCondition(-1);
       QString ss = QObject::tr("A valid angles file is needed to bulk load orientaiton weights and spreads for phase %1 (%2)\n"
                                "Select an angles file and click the Load Data button to load the orientations").arg(sgwidget->getPhaseIndex()).arg(sgwidget->getTabTitle());
       m_Filter->notifyErrorMessage(m_Filter->getHumanLabel(), ss, m_Filter->getErrorCondition());
     }
   }
 }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::afterPreflight()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_addPhase_clicked()
{
  // Ensure the Current SGWidget has generated its data first:
  SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->currentWidget());
  if (false == sgwidget->getDataHasBeenGenerated())
  {
    int r = QMessageBox::warning(this, tr("StatsGenerator"),
                                 tr("Data for the current phase has NOT been generated.\nDo you want to generate it now?"),
                                 QMessageBox::Ok | QMessageBox::Cancel);
    if (r == QMessageBox::Ok)
    {
      // The user wants to generate the data. Generate it and move on
      sgwidget->on_m_GenerateDefaultData_clicked();
    }
    else if (r == QMessageBox::Cancel)
    {
      return;
    }
  }


  double phaseFractionTotal = 0.0;
  for(int p = 0; p < phaseTabs->count(); ++p)
  {
    SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->widget(p));
    phaseFractionTotal += sgwidget->getPhaseFraction();
  }

  EditPhaseDialog dialog;
  dialog.setEditFlag(true);
  dialog.setOtherPhaseFractionTotal(static_cast<float>(phaseFractionTotal));
  int r = dialog.exec();
  if (r == QDialog::Accepted)
  {
    if(dialog.getPhaseType() == SIMPL::PhaseType::PrimaryPhase)
    {
      PrimaryPhaseWidget* ppw = new PrimaryPhaseWidget();
      phaseTabs->addTab(ppw, "Primary");

      connect(ppw, SIGNAL(phaseParametersChanged()),
              this, SIGNAL(parametersChanged()));

      ppw->setPhaseIndex(phaseTabs->count());
      ppw->setPhaseType(SIMPL::PhaseType::PrimaryPhase);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      QString cName = ppw->getComboString();
      ppw->setObjectName(cName);
      ppw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == SIMPL::PhaseType::PrecipitatePhase)
    {
      PrecipitatePhaseWidget* ppw = new PrecipitatePhaseWidget();
      phaseTabs->addTab(ppw, "Precipitate");

      connect(ppw, SIGNAL(phaseParametersChanged()),
              this, SIGNAL(parametersChanged()));

      ppw->setPhaseIndex(phaseTabs->count());
      ppw->setPhaseType(SIMPL::PhaseType::PrecipitatePhase);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPptFraction(dialog.getPptFraction());
      QString cName = ppw->getComboString();
      ppw->setObjectName(cName);
      ppw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == SIMPL::PhaseType::TransformationPhase)
    {
      TransformationPhaseWidget* tpw = new TransformationPhaseWidget();
      phaseTabs->addTab(tpw, "Transformation");

      connect(tpw, SIGNAL(phaseParametersChanged()),
              this, SIGNAL(parametersChanged()));

      tpw->setPhaseIndex(phaseTabs->count());
      tpw->setPhaseType(SIMPL::PhaseType::TransformationPhase);
      tpw->setCrystalStructure(dialog.getCrystalStructure());
      tpw->setPhaseFraction(dialog.getPhaseFraction());
      tpw->setPhaseType(dialog.getPhaseType());
      tpw->setParentPhase(dialog.getParentPhase());
      QString cName = tpw->getComboString();
      tpw->setObjectName(cName);
      tpw->updatePlots();
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == SIMPL::PhaseType::MatrixPhase)
    {
      MatrixPhaseWidget* mpw = new MatrixPhaseWidget();
      phaseTabs->addTab(mpw, "Matrix");

      connect(mpw, SIGNAL(phaseParametersChanged()),
              this, SIGNAL(parametersChanged()));

      mpw->setPhaseIndex(phaseTabs->count());
      mpw->setPhaseType(SIMPL::PhaseType::MatrixPhase);
      mpw->setCrystalStructure(dialog.getCrystalStructure());
      mpw->setPhaseFraction(dialog.getPhaseFraction());
      mpw->setPhaseType(dialog.getPhaseType());
      QString cName = mpw->getComboString();
      mpw->setObjectName(cName);
      setWindowModified(true);
    }
    else if(dialog.getPhaseType() == SIMPL::PhaseType::BoundaryPhase)
    {
      BoundaryPhaseWidget* bpw = new BoundaryPhaseWidget();
      phaseTabs->addTab(bpw, "Boundary");

      connect(bpw, SIGNAL(phaseParametersChanged()),
              this, SIGNAL(parametersChanged()));

      bpw->setPhaseIndex(phaseTabs->count());
      bpw->setPhaseType(SIMPL::PhaseType::BoundaryPhase);
      bpw->setCrystalStructure(dialog.getCrystalStructure());
      bpw->setPhaseFraction(dialog.getPhaseFraction());
      bpw->setPhaseType(dialog.getPhaseType());
      QString cName = bpw->getComboString();
      bpw->setObjectName(cName);
      setWindowModified(true);
    }

    // Make sure the new tab is the active tab
    phaseTabs->setCurrentIndex(phaseTabs->count() - 1);
  }
  emit parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_editPhase_clicked()
{
  double phaseFractionTotal = 0.0;
  EditPhaseDialog dialog;
  dialog.setEditFlag(false);

  SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->currentWidget());

  for (int p = 0; p < phaseTabs->count(); ++p)
  {
    SGWidget* currentWidget = qobject_cast<SGWidget*>(phaseTabs->widget(p));
    if(sgwidget != currentWidget )
    {
      phaseFractionTotal += currentWidget->getPhaseFraction();
    }
    else
    {
      dialog.setPhaseFraction(currentWidget->getPhaseFraction());
    }
  }
  dialog.setPhaseType(sgwidget->getPhaseType());
  dialog.setOtherPhaseFractionTotal(static_cast<float>(phaseFractionTotal));
  dialog.setCrystalStructure(sgwidget->getCrystalStructure());

  if(dialog.getPhaseType() == SIMPL::PhaseType::PrimaryPhase)
  {
    //    PrimaryPhaseWidget* ppw = qobject_cast<PrimaryPhaseWidget*>(sgwidget);
  }
  else if(dialog.getPhaseType() == SIMPL::PhaseType::PrecipitatePhase)
  {
    PrecipitatePhaseWidget* ppw = qobject_cast<PrecipitatePhaseWidget*>(sgwidget);
    if (ppw) { dialog.setPptFraction(ppw->getPptFraction()); }
  }
  else if(dialog.getPhaseType() == SIMPL::PhaseType::TransformationPhase)
  {
    TransformationPhaseWidget* tpw = qobject_cast<TransformationPhaseWidget*>(sgwidget);
    if (tpw) { dialog.setParentPhase(tpw->getParentPhase()); }
  }
  else if(dialog.getPhaseType() == SIMPL::PhaseType::MatrixPhase)
  {
    //    MatrixPhaseWidget* mpw = qobject_cast<MatrixPhaseWidget*>(sgwidget);
  }
  else if(dialog.getPhaseType() == SIMPL::PhaseType::BoundaryPhase)
  {
    //    BoundaryPhaseWidget* bpw = qobject_cast<BoundaryPhaseWidget*>(sgwidget);
  }
  int r = dialog.exec();
  if(r == QDialog::Accepted)
  {
    if(dialog.getPhaseType() == SIMPL::PhaseType::PrimaryPhase)
    {
      PrimaryPhaseWidget* ppw = qobject_cast<PrimaryPhaseWidget*>(sgwidget);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      QString cName = ppw->getComboString();
      setWindowModified(true);
      ppw->updatePlots();
    }
    if(dialog.getPhaseType() == SIMPL::PhaseType::PrecipitatePhase)
    {
      PrecipitatePhaseWidget* ppw = qobject_cast<PrecipitatePhaseWidget*>(sgwidget);
      ppw->setCrystalStructure(dialog.getCrystalStructure());
      ppw->setPhaseFraction(dialog.getPhaseFraction());
      ppw->setPhaseType(dialog.getPhaseType());
      ppw->setPptFraction(dialog.getPptFraction());
      QString cName = ppw->getComboString();
      setWindowModified(true);
      ppw->updatePlots();
    }
    if(dialog.getPhaseType() == SIMPL::PhaseType::TransformationPhase)
    {
      TransformationPhaseWidget* tpw = qobject_cast<TransformationPhaseWidget*>(sgwidget);
      tpw->setCrystalStructure(dialog.getCrystalStructure());
      tpw->setPhaseFraction(dialog.getPhaseFraction());
      tpw->setPhaseType(dialog.getPhaseType());
      tpw->setParentPhase(dialog.getParentPhase());
      QString cName = tpw->getComboString();
      setWindowModified(true);
      tpw->updatePlots();
    }
    if(dialog.getPhaseType() == SIMPL::PhaseType::MatrixPhase)
    {
      MatrixPhaseWidget* mpw = qobject_cast<MatrixPhaseWidget*>(sgwidget);
      mpw->setCrystalStructure(dialog.getCrystalStructure());
      mpw->setPhaseFraction(dialog.getPhaseFraction());
      mpw->setPhaseType(dialog.getPhaseType());
      QString cName = mpw->getComboString();
      setWindowModified(true);
    }
    if(dialog.getPhaseType() == SIMPL::PhaseType::BoundaryPhase)
    {
      BoundaryPhaseWidget* bpw = qobject_cast<BoundaryPhaseWidget*>(sgwidget);
      bpw->setCrystalStructure(dialog.getCrystalStructure());
      bpw->setPhaseFraction(dialog.getPhaseFraction());
      bpw->setPhaseType(dialog.getPhaseType());
      QString cName = bpw->getComboString();
      setWindowModified(true);
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_phaseTabs_tabCloseRequested ( int index )
{
  if(phaseTabs->count() > 1)
  {
    // Remove the SGPhase object from the vector
    SGWidget* currentWidget = qobject_cast<SGWidget*>(phaseTabs->widget(index));
    phaseTabs->removeTab(index);
    currentWidget->deleteLater(); // Actually delete it

    // Reset the phase index for each SGPhase object
    for (int p = 0; p < phaseTabs->count(); ++p)
    {
      SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->widget(p));
      sgwidget->setPhaseIndex(p + 1);
      sgwidget->setObjectName(sgwidget->getComboString());
    }

#if 0
    SGWidget* widget = m_SGWidgets[0];

    verticalLayout_2->removeWidget(m_SGWidget);
    m_SGWidget->hide();
    m_SGWidget->deleteLater();
    m_SGWidget = widget;
    verticalLayout_2->addWidget(m_SGWidget);
    m_SGWidget->show();
#endif
  }
  setWindowModified(true);
  emit parametersChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_deletePhase_clicked()
{
  on_phaseTabs_tabCloseRequested(phaseTabs->currentIndex());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGeneratorWidget::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGeneratorWidget::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  if (false == fileinfo.exists())
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return fileinfo.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_actionSaveAs_triggered()
{
#if 0
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "Untitled";
  QString h5file = QFileDialog::getSaveFileName(this, tr("Save DREAM.3D File"),
                                                proposedFile,
                                                tr("DREAM.3D Files (*.dream3d)") );
  if ( true == h5file.isEmpty() ) { return;  }
  m_FilePath = h5file;
  QFileInfo fi (m_FilePath);
  QString ext = fi.suffix();
  m_OpenDialogLastDirectory = fi.path();
  m_FileSelected = true;
  on_saveH5Btn_clicked();
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DataContainerArray::Pointer StatsGeneratorWidget::generateDataContainerArray()
{
  int err = 0;
  int nPhases = phaseTabs->count() + 1;
  DataContainerArray::Pointer dca = DataContainerArray::New();
  DataContainer::Pointer m = DataContainer::New(SIMPL::Defaults::StatsGenerator);
  dca->addDataContainer(m);

  QVector<size_t> tDims(1, nPhases);
  AttributeMatrix::Pointer cellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::AttributeMatrixType::CellEnsemble);
  m->addAttributeMatrix(SIMPL::Defaults::CellEnsembleAttributeMatrixName, cellEnsembleAttrMat);

  StatsDataArray::Pointer statsDataArray = StatsDataArray::New();
  statsDataArray->resize(nPhases);
  cellEnsembleAttrMat->addAttributeArray(SIMPL::EnsembleData::Statistics, statsDataArray);

  QVector<size_t> cDims(1, 1);
  UInt32ArrayType::Pointer crystalStructures = UInt32ArrayType::CreateArray(tDims, cDims, SIMPL::EnsembleData::CrystalStructures);
  crystalStructures->setValue(0, Ebsd::CrystalStructure::UnknownCrystalStructure);
  cellEnsembleAttrMat->addAttributeArray(SIMPL::EnsembleData::CrystalStructures, crystalStructures);

  UInt32ArrayType::Pointer phaseTypes = UInt32ArrayType::CreateArray(tDims, cDims, SIMPL::EnsembleData::PhaseTypes);
  phaseTypes->setValue(0, SIMPL::PhaseType::UnknownPhaseType);
  cellEnsembleAttrMat->addAttributeArray(SIMPL::EnsembleData::PhaseTypes, phaseTypes);

  double phaseFractionTotal = 0.0;
  for(int p = 0; p < phaseTabs->count(); ++p)
  {
    SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->widget(p));
    phaseFractionTotal += sgwidget->getPhaseFraction();
  }

  // Loop on all the phases
  for (int i = 0; i < phaseTabs->count(); ++i)
  {
    SGWidget* sgwidget = qobject_cast<SGWidget*>(phaseTabs->widget(i));
    sgwidget->setTotalPhaseFraction(static_cast<float>(phaseFractionTotal));
    if (sgwidget->getPhaseType() == SIMPL::PhaseType::PrimaryPhase)
    {
      PrimaryStatsData::Pointer data = PrimaryStatsData::New();
      statsDataArray->setStatsData(i + 1, data);
    }
    if (sgwidget->getPhaseType() == SIMPL::PhaseType::PrecipitatePhase)
    {
      PrecipitateStatsData::Pointer data = PrecipitateStatsData::New();
      statsDataArray->setStatsData(i + 1, data);
    }
    if (sgwidget->getPhaseType() == SIMPL::PhaseType::TransformationPhase)
    {
      TransformationStatsData::Pointer data = TransformationStatsData::New();
      statsDataArray->setStatsData(i + 1, data);
    }
    if (sgwidget->getPhaseType() == SIMPL::PhaseType::MatrixPhase)
    {
      MatrixStatsData::Pointer data = MatrixStatsData::New();
      statsDataArray->setStatsData(i + 1, data);
    }
    if (sgwidget->getPhaseType() == SIMPL::PhaseType::BoundaryPhase)
    {
      BoundaryStatsData::Pointer data = BoundaryStatsData::New();
      statsDataArray->setStatsData(i + 1, data);
    }
    err = sgwidget->gatherStatsData(cellEnsembleAttrMat);
    if (err < 0)
    {
      QString  msg("Internal error gathering statistics from Statistics Widgets.\nError code ");
      msg.append(QString::number(err));
      QMessageBox::critical(this, QString("Gather StatsData Error"), msg , QMessageBox::Ok);
      return DataContainerArray::NullPointer();
    }
  }
  return dca;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_saveJsonBtn_clicked()
{

  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "Untitled.json";
  QString outFile = QFileDialog::getSaveFileName(this, tr("Save JSON File"),
                                                 proposedFile,
                                                 tr("JSON Files (*.json)") );

  if ( true == outFile.isEmpty() ) { return;  }
  QFileInfo fi (outFile);
  QString ext = fi.suffix();
  m_OpenDialogLastDirectory = fi.path();

  DataContainerArray::Pointer dca = generateDataContainerArray();
  AttributeMatrix::Pointer am  = dca->getAttributeMatrix(DataArrayPath(SIMPL::Defaults::StatsGenerator, SIMPL::Defaults::CellEnsembleAttributeMatrixName, ""));

  IDataArray::Pointer ida = am->getAttributeArray(SIMPL::EnsembleData::Statistics);
  StatsDataArray::Pointer sda = std::dynamic_pointer_cast<StatsDataArray>(ida);
  if(NULL == sda.get())
  {
    QMessageBox::critical(this, QString("JSON File Save Error"), QString("StatsDataArray was NULL or Invalid") , QMessageBox::Ok);
    return;
  }

  IDataArray::Pointer iDataArray = am->getAttributeArray(SIMPL::EnsembleData::CrystalStructures);
  //unsigned int* crystalStructures = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);

  QJsonObject jsonRoot;

  sda->writeToJson(jsonRoot, std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray));

  QJsonDocument doc(jsonRoot);

  QFile file(outFile);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QByteArray binary = doc.toJson();
  file.write(binary.data(), binary.length());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_saveH5Btn_clicked()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "Untitled.dream3d";

  QString h5file = QFileDialog::getSaveFileName(this, tr("Save DREAM.3D File"),
                                                proposedFile,
                                                tr("DREAM.3D Files (*.dream3d)") );

  if ( true == h5file.isEmpty() ) { return;  }

  QFileInfo fi (h5file);
  QString ext = fi.suffix();
  m_OpenDialogLastDirectory = fi.path();

  DataContainerArray::Pointer dca = generateDataContainerArray();

  DataContainerWriter::Pointer writer = DataContainerWriter::New();
  writer->setDataContainerArray(dca);
  writer->setOutputFile(h5file);
  writer->setWriteXdmfFile(false);
  writer->setWritePipeline(false);
  writer->execute();
  // Force the clean up of the writer by assigning a NULL pointer which will
  // have the effect of executing the destructor of the H5StatsWriter Class
  writer = DataContainerWriter::NullPointer();

  setWindowTitle(h5file + " - StatsGenerator");
  setWindowModified(false);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::on_openStatsFile_clicked()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "Untitled.dream3d";
  QString h5file = QFileDialog::getOpenFileName(this,
                                                tr("Open Statistics File"),
                                                proposedFile,
                                                tr("DREAM3D Files (*.dream3d);;H5Stats Files(*.h5stats);;HDF5 Files(*.h5 *.hdf5);;All Files(*.*)"));
  if(true == h5file.isEmpty())
  {
    return;
  }

  // Make sure the file path is not empty and does exist on the system
  if (true == h5file.isEmpty())
  {
    QString ss = QObject::tr("Input file was empty").arg(h5file);
    QMessageBox::critical(this, QString("File Open Error"), ss , QMessageBox::Ok);
    return;
  }

  QFileInfo fi(h5file);
  if (fi.exists() == false)
  {
    QString ss = QObject::tr("Input file does not exist").arg(fi.absoluteFilePath());
    QMessageBox::critical(this, QString("File Open Error"), ss , QMessageBox::Ok);
    return;
  }

  // Set the last directory that contains our file
  m_OpenDialogLastDirectory = fi.path();

  // Delete any existing phases from the GUI
  phaseTabs->clear();

  DataContainerArray::Pointer dca = DataContainerArray::New();
  DataContainer::Pointer m = DataContainer::New(SIMPL::Defaults::StatsGenerator);
  dca->addDataContainer(m);
  QVector<size_t> tDims(1, 0);
  AttributeMatrix::Pointer cellEnsembleAttrMat = AttributeMatrix::New(tDims, SIMPL::Defaults::CellEnsembleAttributeMatrixName, SIMPL::AttributeMatrixType::CellEnsemble);
  m->addAttributeMatrix(SIMPL::Defaults::CellEnsembleAttributeMatrixName, cellEnsembleAttrMat);

  hid_t fileId = QH5Utilities::openFile(h5file, true); // Open the file Read Only
  if(fileId < 0)
  {
    QString ss = QObject::tr(": Error opening input file '%1'").arg(h5file);
    QMessageBox::critical(this, QString("File Open Error"), ss , QMessageBox::Ok);
    return;
  }
  // This will make sure if we return early from this method that the HDF5 File is properly closed.
  HDF5ScopedFileSentinel scopedFileSentinel(&fileId, true);
  hid_t dcaGid = H5Gopen(fileId, SIMPL::StringConstants::DataContainerGroupName.toLatin1().constData(), 0);
  scopedFileSentinel.addGroupId(&dcaGid);

  hid_t dcGid = H5Gopen(dcaGid, SIMPL::Defaults::StatsGenerator.toLatin1().constData(), 0);
  if(dcGid < 0)
  {
    QString title = QObject::tr("Error Opening Data Container");
    QString msg = QObject::tr("Error opening default Data Container with name '%1'").arg(SIMPL::Defaults::StatsGenerator);
    QMessageBox::critical(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    return;
  }
  scopedFileSentinel.addGroupId(&dcGid);
  hid_t amGid = H5Gopen(dcGid, SIMPL::Defaults::CellEnsembleAttributeMatrixName.toLatin1().constData(), 0);
  scopedFileSentinel.addGroupId(&amGid);


  // We need to read one of the arrays to get the number of phases so that we can resize our attributeMatrix
  UInt32ArrayType::Pointer phases = UInt32ArrayType::CreateArray(1, SIMPL::EnsembleData::PhaseTypes);
  int err = phases->readH5Data(amGid);
  if (err < 0)
  {

    QString title = QObject::tr("Error Opening DataArray");
    QString msg = QObject::tr("Error opening 'PhaseTypes' data array at location '%1/%2/%3/%4'").arg(SIMPL::StringConstants::DataContainerGroupName)
        .arg(SIMPL::Defaults::StatsGenerator).arg(SIMPL::Defaults::CellEnsembleAttributeMatrixName).arg(SIMPL::EnsembleData::PhaseTypes);
    QMessageBox::critical(this, title, msg, QMessageBox::Ok, QMessageBox::Ok);
    return;
  }
  tDims[0] = phases->getNumberOfTuples();
  cellEnsembleAttrMat->resizeAttributeArrays(tDims);

  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "Statistics", false);
  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "CrystalStructures", false);
  cellEnsembleAttrMat->addAttributeArrayFromHDF5Path(amGid, "PhaseTypes", false);

  // Get the number of Phases
  size_t ensembles = cellEnsembleAttrMat->getNumTuples();

  QProgressDialog progress("Opening Stats File....", "Cancel", 0, ensembles, this);
  progress.setWindowModality(Qt::WindowModal);


  IDataArray::Pointer iDataArray = cellEnsembleAttrMat->getAttributeArray(SIMPL::EnsembleData::PhaseTypes);
  unsigned int* phaseTypes = std::dynamic_pointer_cast< UInt32ArrayType >(iDataArray)->getPointer(0);

  // We should iterate on all the phases here to start setting data and creating
  // all of the StatsGenPhase Objects
  for (size_t phase = 1; phase < ensembles; ++phase)
  {
    progress.setValue(phase);

    if (progress.wasCanceled())
    {
      return;
    }

    if(phaseTypes[phase] == SIMPL::PhaseType::BoundaryPhase)
    {
      progress.setLabelText("Opening Boundaray Phase...");
      BoundaryPhaseWidget* w = new BoundaryPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    else if(phaseTypes[phase] == SIMPL::PhaseType::MatrixPhase)
    {
      progress.setLabelText("Opening Matrix Phase...");
      MatrixPhaseWidget* w = new MatrixPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == SIMPL::PhaseType::PrecipitatePhase)
    {
      progress.setLabelText("Opening Precipitate Phase...");
      PrecipitatePhaseWidget* w = new PrecipitatePhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == SIMPL::PhaseType::PrimaryPhase)
    {
      progress.setLabelText("Opening Primary Phase...");
      PrimaryPhaseWidget* w = new PrimaryPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    if(phaseTypes[phase] == SIMPL::PhaseType::TransformationPhase)
    {
      progress.setLabelText("Opening Transformation Phase...");
      TransformationPhaseWidget* w = new TransformationPhaseWidget(this);
      phaseTabs->addTab(w, w->getTabTitle());
      w->extractStatsData(cellEnsembleAttrMat, static_cast<int>(phase));
    }
    else
    {

    }
  }
  progress.setValue(ensembles);

  // Now delete the first Phase from the Combo which was left over from something else
  phaseTabs->setCurrentIndex(0);

  // Set the window title correctly
  setWindowModified(false);
  QString windowTitle = QString("");
  setWindowTitle(windowTitle);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGeneratorWidget::displayDialogBox(QString title, QString text, QMessageBox::Icon icon)
{

  QMessageBox msgBox;
  msgBox.setText(title);
  msgBox.setInformativeText(text);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setDefaultButton(QMessageBox::Ok);
  msgBox.setIcon(icon);
  msgBox.exec();
}
