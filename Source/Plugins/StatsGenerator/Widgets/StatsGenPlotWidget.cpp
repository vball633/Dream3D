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


#include "StatsGenPlotWidget.h"

#include "SIMPLib/Math/SIMPLibMath.h"

//-- C++ Includes
#include <iostream>
#include <limits>

//-- Qt Includes
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QAbstractItemDelegate>
#include <QtWidgets/QMenu>
#include <QtWidgets/QTableView>

//-- Qwt Includes
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_abstract_scale_draw.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_interval.h>
#include <qwt_point_3d.h>
#include <qwt_compat.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_item.h>
#include <qwt_text_label.h>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataArrays/StatsDataArray.h"

#include "StatsGenerator/Widgets/TableModels/SGBetaTableModel.h"
#include "StatsGenerator/Widgets/TableModels/SGLogNormalTableModel.h"
#include "StatsGenerator/Widgets/TableModels/SGPowerLawTableModel.h"
#include "StatsGenerator/Widgets/Presets/AbstractMicrostructurePreset.h"

#include "OrientationLib/Texture/StatsGen.hpp"

#define SG_ERROR_CHECK(name)\
  qDebug() << "Error writing HDF5 data to " << name << "\n";\
  qDebug() << "  File: " << __FILE__ << "\n";\
  qDebug() << "  Line: " << __LINE__ << "\n";

// Include the MOC generated CPP file which has all the QMetaObject methods/data
#include "moc_StatsGenPlotWidget.cpp"

namespace UIA
{
  const static int Alpha = 255;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenPlotWidget::StatsGenPlotWidget(QWidget* parent) :
  QWidget(parent),
  m_Mu(1.0f),
  m_Sigma(0.1f),
  m_MinCutOff(3.0f),
  m_MaxCutOff(3.0f),
  m_BinStep(1.0f),
  m_PhaseIndex(-1),
  m_DistributionType(SIMPL::DistributionType::UnknownDistributionType),
  m_TableModel(NULL),
  m_grid(NULL),
  m_StatsType(SIMPL::StatisticsType::UnknownStatisticsGroup),
  m_UserUpdatedData(false)
{
  this->setupUi(this);
  this->setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenPlotWidget::~StatsGenPlotWidget()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setStatisticsType(unsigned int distributionType)
{
  m_StatsType = distributionType;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setPlotTitle(QString title)
{
  QwtText qwtTitle(title);
  qwtTitle.setColor(Qt::white);
 // qwtTitle.setRenderFlags(Qt::AlignHCenter | Qt::AlignTop);
  //title.setFont(font);
  m_PlotView->setTitle(qwtTitle);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setSizeDistributionValues(float mu, float sigma,
                                                   float minCutOff, float maxCutOff,
                                                   float binStepSize)
{
  m_Mu = mu;
  m_Sigma = sigma;
  m_MinCutOff = minCutOff;
  m_MaxCutOff = maxCutOff;
  m_BinStep = binStepSize;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenPlotWidget::extractStatsData(int index,
                                         QVector<float>&  binNumbers,
                                         VectorOfFloatArray arrays)
{
  int err = 0;
  if (m_StatsType == SIMPL::StatisticsType::UnknownStatisticsGroup)
  {
    QMessageBox::critical(this, tr("StatsGenerator"),
                          tr("This Plot has not been assigned a Statistics Group. This should be happening from within the program. Contact the developer."),
                          QMessageBox::Ok,
                          QMessageBox::Ok);
    return -1;
  }

  if (m_DistributionType == SIMPL::DistributionType::UnknownDistributionType)
  {
    QMessageBox::critical(this, tr("StatsGenerator"),
                          tr("The 'Distribution Type' was of an unknown type."),
                          QMessageBox::Ok,
                          QMessageBox::Ok);
    return -1;
  }


  // Remove all the current rows in the table model
  m_TableModel->removeRows(0, m_TableModel->rowCount());

  // This will force a reset of the table model creating a new table model
  setDistributionType(m_DistributionType); // This makes sure the combo box is set correctly

  qint32 count = binNumbers.count();
  QVector<QColor> colors = AbstractMicrostructurePreset::GenerateColors(count, 160, 255);

  QVector<QVector<float> > data;

  for(VectorOfFloatArray::size_type i = 0; i < arrays.size(); ++i)
  {
    QVector<float> col(static_cast<int>(arrays[i]->getNumberOfTuples()));
    if (arrays[i]->getNumberOfTuples() > 0)
    {
      ::memcpy( &(col.front()), arrays[i]->getVoidPointer(0), sizeof(float)*col.size() );
    }
    data.push_back(col);
  }

  m_TableModel->setTableData(binNumbers, data, colors);

  m_TableView->resizeColumnsToContents();
  m_TableView->scrollToBottom();
  m_TableView->setFocus();
  updatePlotCurves();

  m_UserUpdatedData = true;
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
VectorOfFloatArray StatsGenPlotWidget::getStatisticsData()
{
  VectorOfFloatArray data;
  if (m_StatsType == SIMPL::StatisticsType::UnknownStatisticsGroup)
  {
    QMessageBox::critical(this, tr("StatsGenerator"),
                          tr("This Plot has not been assigned a Statistics Group. This should be happening from within the program. Contact the developer."),
                          QMessageBox::Ok,
                          QMessageBox::Ok);
    return data;
  }

  if (m_DistributionType == SIMPL::DistributionType::UnknownDistributionType)
  {
    QMessageBox::critical(this, tr("StatsGenerator"),
                          tr("This Plot has not been assigned a known Distribution Type. This should be happening from within the program. Contact the developer."),
                          QMessageBox::Ok,
                          QMessageBox::Ok);
    return data;
  }

  FloatArrayType::Pointer col0;
  FloatArrayType::Pointer col1;
  FloatArrayType::Pointer col2;

  QVector<float> v0;
  QVector<float> v1;
  QVector<float> v2;
  Q_ASSERT(m_PhaseIndex >= 0);

  // Create a new Table Model
  switch(m_DistributionType)
  {
    case SIMPL::DistributionType::Beta:
      v0 = m_TableModel->getData(SGBetaTableModel::Alpha);
      v1 = m_TableModel->getData(SGBetaTableModel::Beta);
      col0 = FloatArrayType::FromQVector(v0, SIMPL::StringConstants::Alpha);
      col1 = FloatArrayType::FromQVector(v1, SIMPL::StringConstants::Beta);
      data.push_back(col0);
      data.push_back(col1);
      break;
    case SIMPL::DistributionType::LogNormal:
      v0 = m_TableModel->getData(SGLogNormalTableModel::Average);
      v1 = m_TableModel->getData(SGLogNormalTableModel::StdDev);
      col0 = FloatArrayType::FromQVector(v0, SIMPL::StringConstants::Average);
      col1 = FloatArrayType::FromQVector(v1, SIMPL::StringConstants::StandardDeviation);
      data.push_back(col0);
      data.push_back(col1);
      break;
//    case SIMPL::DistributionType::Power:
//      v0 = m_TableModel->getData(SGPowerLawTableModel::Alpha);
//      v1 = m_TableModel->getData(SGPowerLawTableModel::K);
//      v2 = m_TableModel->getData(SGPowerLawTableModel::Beta);
//      col0 = FloatArrayType::FromQVector(v0, SIMPL::StringConstants::Alpha);
//      col1 = FloatArrayType::FromQVector(v1, SIMPL::StringConstants::Exp_k);
//      col2 = FloatArrayType::FromQVector(v2, SIMPL::StringConstants::Beta);
//      data.push_back(col0);
//      data.push_back(col1);
//      data.push_back(col2);
//      break;

    default:
      return data;
  }
  return data;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::resetTableModel()
{
  QVector<float> bins;
  // Get a copy of the bins from the current TableModel if available
  if (NULL != m_TableModel)
  {
    bins = m_TableModel->getBinNumbers();
  }

  // Create a new Table Model
  switch(m_DistributionType)
  {
    case SIMPL::DistributionType::Beta:
      m_TableModel = new SGBetaTableModel;
      break;
    case SIMPL::DistributionType::LogNormal:
      m_TableModel = new SGLogNormalTableModel;
      break;
//    case SIMPL::DistributionType::Power:
//      m_TableModel = new SGPowerLawTableModel;
//      break;

    default:
      return;
  }

  QAbstractItemModel* model = m_TableView->model();
  m_TableView->setModel(m_TableModel);
  delete model; // Clean up this memory
  QAbstractItemDelegate* aid = m_TableModel->getItemDelegate();

  m_TableView->setItemDelegate(aid);

  connect(m_TableModel, SIGNAL(layoutChanged()),
          this, SLOT(updatePlotCurves()));
  connect(m_TableModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
          this, SLOT(updatePlotCurves()));
  connect(aid, SIGNAL(commitData(QWidget*)),
          this, SLOT(userCommittedData(QWidget*)));

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::userCommittedData(QWidget* w)
{
  m_UserUpdatedData = true;
  emit userEditedData();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setDistributionType(unsigned int curveType, bool updatePlots)
{
  m_DistributionType = curveType;
  distributionTypeCombo->setCurrentIndex(m_DistributionType);
  resetTableModel();
  if (updatePlots)
  {
    // Update the plots
    updatePlotCurves();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::on_distributionTypeCombo_currentIndexChanged(int index)
{
  m_DistributionType = static_cast<unsigned int>(distributionTypeCombo->currentIndex());
  resetTableModel();
  // Update the plots
  updatePlotCurves();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::blockDistributionTypeChanges(bool block)
{
  distributionTypeCombo->setEnabled(!block);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setXAxisName(QString name)
{
  m_PlotView->setAxisTitle(QwtPlot::xBottom, name);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setYAxisName(QString name)
{
  m_PlotView->setAxisTitle(QwtPlot::yLeft, name);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::initQwtPlot(QString xAxisName, QString yAxisName, QwtPlot* plot)
{

  QPalette  pal;
  pal.setColor(QPalette::Text, Qt::white);
  pal.setColor(QPalette::Foreground, Qt::white);
  pal.setColor(QPalette::Window, Qt::black);

  plot->setPalette( pal );

  plot->plotLayout()->setAlignCanvasToScales( true );
  for ( int axis = 0; axis < QwtPlot::axisCnt; axis++ )
  {
      plot->axisWidget( axis )->setMargin( 0 );
      plot->axisWidget(axis)->setPalette(pal);
  }
  QwtPlotCanvas *canvas = new QwtPlotCanvas();

  canvas->setAutoFillBackground( false );
  canvas->setFrameStyle( QFrame::NoFrame );
  canvas->setPalette(pal);

  plot->setCanvas( canvas );

//  QwtPlotMagnifier* plotMag =  new QwtPlotMagnifier( canvas );
//  plotMag->setWheelModifiers(Qt::KeyboardModifiers(Qt::Key_Shift));

  //(void) new QwtPlotPanner(canvas);

  QFont font;
  font.setBold(true);

  QwtText xAxis(xAxisName);
  xAxis.setColor(Qt::white);
  xAxis.setRenderFlags( Qt::AlignHCenter | Qt::AlignTop );
  xAxis.setFont(font);
  plot->setAxisTitle(QwtPlot::xBottom, xAxis);

  QwtText yAxis(yAxisName);
  yAxis.setColor(Qt::white);
  yAxis.setRenderFlags( Qt::AlignHCenter | Qt::AlignTop );
  yAxis.setFont(font);
  plot->setAxisTitle(QwtPlot::yLeft, yAxis);


  QwtText title("");
  title.setColor(Qt::white);
 // title.setRenderFlags(Qt::AlignHCenter | Qt::AlignTop);
  title.setFont(font);
  plot->setTitle(title);

  const int margin = 1;
  plot->setContentsMargins( margin, margin, margin, margin );


  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->enableYMin(true);
#if (QWT_VERSION > 0x060000)
  m_grid->setMajorPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinorPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
#else
  m_grid->setMajPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinPen(QPen(Qt::lightGray, 0, Qt::DotLine));
#endif



 // m_grid->attach(m_PlotView);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::showDataWindow(bool b)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
 void StatsGenPlotWidget::requestContextMenu(const QPoint& pos)
 {
   QPoint mapped = mapToGlobal(pos);
   m_ContextMenuPoint = mapped;
   QMenu menu;
   QAction* editData = new QAction(QString("Edit Data"), &menu);
   connect(editData, SIGNAL(triggered(bool)),
           this, SLOT(showDataWindow(bool)));
   menu.addAction(editData);
   menu.exec(mapped);

 }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setupGui()
{



//  distributionTypeCombo->blockSignals(true);
//  distributionTypeCombo->addItem(SIMPL::StringConstants::BetaDistribution.toLatin1().data());
//  distributionTypeCombo->addItem(SIMPL::StringConstants::LogNormalDistribution.toLatin1().data());
//  distributionTypeCombo->addItem(SIMPL::StringConstants::PowerLawDistribution.toLatin1().data());
//  distributionTypeCombo->blockSignals(false);


  // Setup the TableView and Table Models
  QHeaderView* headerView = new QHeaderView(Qt::Horizontal, m_TableView);
  headerView->sectionResizeMode(QHeaderView::Interactive);
  m_TableView->setHorizontalHeader(headerView);
  headerView->show();

  // Setup the Qwt Plot Widget
  initQwtPlot("", "", m_PlotView);

  resetTableModel();
  if (NULL != m_TableModel)
  {
    // Update the plots
    updatePlotCurves();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::updatePlotCurves()
{
// qDebug() << "StatsGenPlotWidget::updatePlotCurves" << "\n";
  //Loop over each entry in the table
  QwtPlotCurve* curve = NULL;

  // Delete all the plots
  qint32 nRows = m_PlotCurves.count();
  for (qint32 r = nRows - 1; r >= 0; --r)
  {
    curve = m_PlotCurves[r];
    curve->detach();
    m_PlotCurves.remove(r);
    delete curve;
  }

  nRows = m_TableModel->rowCount();
  float xMax = 0.0;
  float yMax = 0.0;

  for (qint32 r = 0; r < nRows; ++r)
  {
    if (r == m_PlotCurves.size())
    {
      curve = new QwtPlotCurve("");
      curve->setRenderHint(QwtPlotItem::RenderAntialiased);
      QColor c = QColor(m_TableModel->getColor(r));
      c.setAlpha(UIA::Alpha);
      curve->setPen(QPen(c));
      curve->attach(m_PlotView);
      m_PlotCurves.append(curve);
    }
    else
    {
      curve = m_PlotCurves[r];
      QColor c = QColor(m_TableModel->getColor(r));
      c.setAlpha(UIA::Alpha);
      curve->setPen(QPen(c));
    }

    switch(m_DistributionType)
    {
      case SIMPL::DistributionType::Beta:
        createBetaCurve(r, xMax, yMax);
        break;
      case SIMPL::DistributionType::LogNormal:
        createLogNormalCurve(r, xMax, yMax);
        break;
//      case SIMPL::DistributionType::Power:
//        createPowerCurve(r, xMax, yMax);
//        break;
      default:
        break;
    }
  }
  m_PlotView->replot();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool StatsGenPlotWidget::userUpdatedData()
{
  return m_UserUpdatedData;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::createBetaCurve(int tableRow, float& xMax, float& yMax)
{
  QwtPlotCurve* curve = m_PlotCurves[tableRow];
  int err = 0;
  float alpha = m_TableModel->getDataValue(SGBetaTableModel::Alpha, tableRow);
  float beta = m_TableModel->getDataValue(SGBetaTableModel::Beta, tableRow);
  int size = 256;
  QwtArray<float > x;
  QwtArray<float > y;

  err = StatsGen::GenBetaPlotData<QwtArray<float > > (alpha, beta, x, y, size);
  if (err == 1)
  {
    //TODO: Present Error Message
    return;
  }

  QwtArray<double> xD(size);
  QwtArray<double> yD(size);
  for (int i = 0; i < size; ++i)
  {
    //   qDebug() << x[i] << "  " << y[i] << "\n";
    if (x[i] > xMax)
    {
      xMax = x[i];
    }
    if (y[i] > yMax)
    {
      yMax = y[i];
    }
    xD[i] = static_cast<double>(x[i]);
    yD[i] = static_cast<double>(y[i]);
  }
#if QWT_VERSION >= 0x060000
  curve->setSamples(xD, yD);
#else
  curve->setData(xD, yD);
#endif

  m_PlotView->setAxisScale(QwtPlot::yLeft, 0.0, yMax);
  m_PlotView->setAxisScale(QwtPlot::xBottom, 0.0, xMax);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::createLogNormalCurve(int tableRow, float& xMax, float& yMax)
{
  QwtPlotCurve* curve = m_PlotCurves[tableRow];
  int err = 0;
  float avg = m_TableModel->getDataValue(SGLogNormalTableModel::Average, tableRow);
  float stdDev = m_TableModel->getDataValue(SGLogNormalTableModel::StdDev, tableRow);
  int size = 256;
  QwtArray<float > x;
  QwtArray<float > y;

  err = StatsGen::GenLogNormalPlotData<QwtArray<float > > (avg, stdDev, x, y, size);
  if (err == 1)
  {
    //TODO: Present Error Message
    return;
  }

  QwtArray<double> xD(size);
  QwtArray<double> yD(size);
  for (int i = 0; i < size; ++i)
  {
    //   qDebug() << x[i] << "  " << y[i] << "\n";
    if (x[i] > xMax)
    {
      xMax = x[i];
    }
    if (y[i] > yMax)
    {
      yMax = y[i];
    }
    xD[i] = static_cast<double>(x[i]);
    yD[i] = static_cast<double>(y[i]);
  }
#if QWT_VERSION >= 0x060000
  curve->setSamples(xD, yD);
#else
  curve->setData(xD, yD);
#endif

  m_PlotView->setAxisScale(QwtPlot::yLeft, 0.0, yMax);
  m_PlotView->setAxisScale(QwtPlot::xBottom, 0.0, xMax);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::createPowerCurve(int tableRow, float& xMax, float& yMax)
{
  QwtPlotCurve* curve = m_PlotCurves[tableRow];
  int err = 0;
  float alpha = m_TableModel->getDataValue(SGPowerLawTableModel::Alpha, tableRow);
  float k = m_TableModel->getDataValue(SGPowerLawTableModel::K, tableRow);
  float beta = m_TableModel->getDataValue(SGPowerLawTableModel::Beta, tableRow);
  int size = 256;
  QwtArray<float> x;
  QwtArray<float> y;

  err = StatsGen::GenPowerLawPlotData<QwtArray<float > > (alpha, k, beta, x, y, size);
  if (err == 1)
  {
    //TODO: Present Error Message
    return;
  }

  QwtArray<double> xD(size);
  QwtArray<double> yD(size);
  for (int i = 0; i < size; ++i)
  {
    //   qDebug() << x[i] << "  " << y[i] << "\n";
    if (x[i] > xMax)
    {
      xMax = x[i];
    }
    if (y[i] > yMax)
    {
      yMax = y[i];
    }
    xD[i] = static_cast<double>(x[i]);
    yD[i] = static_cast<double>(y[i]);
  }
#if QWT_VERSION >= 0x060000
  curve->setSamples(xD, yD);
#else
  curve->setData(xD, yD);
#endif

  m_PlotView->setAxisScale(QwtPlot::yLeft, 0.0, yMax);
  m_PlotView->setAxisScale(QwtPlot::xBottom, 0.0, xMax);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setRowOperationEnabled(bool b)
{
  addRowBtn->setVisible(b);
  deleteRowBtn->setVisible(b);
}
#if 0
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::setBins(QVector<float>& binNumbers)
{


#if 1
  if (m_UserUpdatedData == true)
  {
    QString msg = "The Statistics Group '";

    switch(m_StatsType)
    {
      case DREAM3D::Reconstruction::Feature_SizeVBoverA:
        msg.append(SIMPL::StringConstants::Feature_SizeVBoverA_Distributions.toLatin1().data());
        break;
      case DREAM3D::Reconstruction::Feature_SizeVCoverA:
        msg.append(SIMPL::StringConstants::Feature_SizeVCoverA_Distributions.toLatin1().data());
        break;
      case DREAM3D::Reconstruction::Feature_SizeVNeighbors:
        msg.append(SIMPL::StringConstants::Feature_SizeVNeighbors_Distributions.toLatin1().data());
        break;
      case DREAM3D::Reconstruction::Feature_SizeVOmega3:
        msg.append(SIMPL::StringConstants::Feature_SizeVOmega3_Distributions.toLatin1().data());
        break;
      default:
        msg.append("Unknown");
    }
    msg.append("' has user modified data. Do you want to over write the data with the new bin values?");



    int r = QMessageBox::critical(this,
                                  tr("Stats Generator"),
                                  msg, QMessageBox::Ok
                                  | QMessageBox::Cancel, QMessageBox::Ok);
    if (r != QMessageBox::Ok)
    {
      return;
    }
  }
#endif
  m_UserUpdatedData = false;
  m_TableModel->setBinNumbers(binNumbers);
  m_TableView->resizeColumnsToContents();
  m_TableView->scrollToBottom();
  m_TableView->setFocus();

  updatePlotCurves();
}
#endif

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::on_addRowBtn_clicked()
{
  if (!m_TableModel->insertRow(m_TableModel->rowCount())) { return; }
  m_TableView->resizeColumnsToContents();
  m_TableView->scrollToBottom();
  m_TableView->setFocus();
  QModelIndex index = m_TableModel->index(m_TableModel->rowCount() - 1, 0);
  m_TableView->setCurrentIndex(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenPlotWidget::on_deleteRowBtn_clicked()
{
  QItemSelectionModel* selectionModel = m_TableView->selectionModel();
  if (!selectionModel->hasSelection()) { return; }
  QModelIndex index = selectionModel->currentIndex();
  if (!index.isValid()) { return; }
  m_TableModel->removeRow(index.row(), index.parent());
  if (m_TableModel->rowCount() > 0)
  {
    m_TableView->resizeColumnsToContents();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SGAbstractTableModel* StatsGenPlotWidget::tableModel()
{
  return m_TableModel;
}
