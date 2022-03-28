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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "StatsToolbox/StatsToolboxDLLExport.h"

/**
 * @brief The FindSurfaceAreaToVolume class. See [Filter documentation](@ref findsurfaceareatovolume) for details.
 */
class StatsToolbox_EXPORT FindSurfaceAreaToVolume : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(FindSurfaceAreaToVolume SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(FindSurfaceAreaToVolume)
  PYB11_FILTER_NEW_MACRO(FindSurfaceAreaToVolume)
  PYB11_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)
  PYB11_PROPERTY(DataArrayPath NumCellsArrayPath READ getNumCellsArrayPath WRITE setNumCellsArrayPath)
  PYB11_PROPERTY(QString SurfaceAreaVolumeRatioArrayName READ getSurfaceAreaVolumeRatioArrayName WRITE setSurfaceAreaVolumeRatioArrayName)
  PYB11_PROPERTY(QString SphericityArrayName READ getSphericityArrayName WRITE setSphericityArrayName)
  PYB11_PROPERTY(bool CalculateSphericity READ getCalculateSphericity WRITE setCalculateSphericity)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = FindSurfaceAreaToVolume;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;

  /**
   * @brief Returns a NullPointer wrapped by a shared_ptr<>
   * @return
   */
  static Pointer NullPointer();

  /**
   * @brief Creates a new object wrapped in a shared_ptr<>
   * @return
   */
  static Pointer New();

  /**
   * @brief Returns the name of the class for FindSurfaceAreaToVolume
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for FindSurfaceAreaToVolume
   */
  static QString ClassName();

  ~FindSurfaceAreaToVolume() override;

  /**
   * @brief Setter property for FeatureIdsArrayPath
   */
  void setFeatureIdsArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for FeatureIdsArrayPath
   * @return Value of FeatureIdsArrayPath
   */
  DataArrayPath getFeatureIdsArrayPath() const;
  Q_PROPERTY(DataArrayPath FeatureIdsArrayPath READ getFeatureIdsArrayPath WRITE setFeatureIdsArrayPath)

  /**
   * @brief Setter property for NumCellsArrayPath
   */
  void setNumCellsArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for NumCellsArrayPath
   * @return Value of NumCellsArrayPath
   */
  DataArrayPath getNumCellsArrayPath() const;
  Q_PROPERTY(DataArrayPath NumCellsArrayPath READ getNumCellsArrayPath WRITE setNumCellsArrayPath)

  /**
   * @brief Setter property for SurfaceAreaVolumeRatioArrayName
   */
  void setSurfaceAreaVolumeRatioArrayName(const QString& value);
  /**
   * @brief Getter property for SurfaceAreaVolumeRatioArrayName
   * @return Value of SurfaceAreaVolumeRatioArrayName
   */
  QString getSurfaceAreaVolumeRatioArrayName() const;
  Q_PROPERTY(QString SurfaceAreaVolumeRatioArrayName READ getSurfaceAreaVolumeRatioArrayName WRITE setSurfaceAreaVolumeRatioArrayName)

  /**
   * @brief Setter property for SphericityArrayName
   */
  void setSphericityArrayName(const QString& value);
  /**
   * @brief Getter property for SphericityArrayName
   * @return Value of SphericityArrayName
   */
  QString getSphericityArrayName() const;
  Q_PROPERTY(QString SphericityArrayName READ getSphericityArrayName WRITE setSphericityArrayName)

  /**
   * @brief Setter property for CalculateSphericity
   */
  void setCalculateSphericity(bool value);
  /**
   * @brief Getter property for CalculateSphericity
   * @return Value of CalculateSphericity
   */
  bool getCalculateSphericity() const;
  Q_PROPERTY(bool CalculateSphericity READ getCalculateSphericity WRITE setCalculateSphericity)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  QUuid getUuid() const override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;

protected:
  FindSurfaceAreaToVolume();
  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

private:
  std::weak_ptr<DataArray<int32_t>> m_FeatureIdsPtr;
  int32_t* m_FeatureIds = nullptr;
  std::weak_ptr<DataArray<int32_t>> m_NumCellsPtr;
  int32_t* m_NumCells = nullptr;
  std::weak_ptr<DataArray<float>> m_SurfaceAreaVolumeRatioPtr;
  float* m_SurfaceAreaVolumeRatio = nullptr;
  std::weak_ptr<DataArray<float>> m_SphericityPtr;
  float* m_Sphericity = nullptr;

  DataArrayPath m_FeatureIdsArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, SIMPL::CellData::FeatureIds};
  DataArrayPath m_NumCellsArrayPath = {SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellFeatureAttributeMatrixName, SIMPL::FeatureData::NumElements};
  QString m_SurfaceAreaVolumeRatioArrayName = {SIMPL::FeatureData::SurfaceAreaVol};
  QString m_SphericityArrayName = {"Sphericity"};
  bool m_CalculateSphericity = {true};

public:
  FindSurfaceAreaToVolume(const FindSurfaceAreaToVolume&) = delete;            // Copy Constructor Not Implemented
  FindSurfaceAreaToVolume(FindSurfaceAreaToVolume&&) = delete;                 // Move Constructor Not Implemented
  FindSurfaceAreaToVolume& operator=(const FindSurfaceAreaToVolume&) = delete; // Copy Assignment Not Implemented
  FindSurfaceAreaToVolume& operator=(FindSurfaceAreaToVolume&&) = delete;      // Move Assignment Not Implemented
};