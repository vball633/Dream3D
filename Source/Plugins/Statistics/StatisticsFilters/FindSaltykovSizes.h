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


#ifndef _findsaltykovsizes_h_
#define _findsaltykovsizes_h_

#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/AbstractFilter.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/IDataArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

/**
 * @class FindSaltykovSizes FindSaltykovSizes.h DREAM3DLib/GenericFilters/FindSaltykovSizes.h
 * @brief
 * @author Joseph C. Tucker
 * @date Dec 9, 2014
 * @version 5.2
 */
class FindSaltykovSizes : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    SIMPL_SHARED_POINTERS(FindSaltykovSizes)
    SIMPL_STATIC_NEW_MACRO(FindSaltykovSizes)
    SIMPL_TYPE_MACRO_SUPER(FindSaltykovSizes, AbstractFilter)

    virtual ~FindSaltykovSizes();

    SIMPL_FILTER_PARAMETER(DataArrayPath, EquivalentDiametersArrayPath)
    Q_PROPERTY(DataArrayPath EquivalentDiametersArrayPath READ getEquivalentDiametersArrayPath WRITE setEquivalentDiametersArrayPath)

    SIMPL_FILTER_PARAMETER(DataArrayPath, SaltykovEquivalentDiametersArrayPath)
    Q_PROPERTY(DataArrayPath SaltykovEquivalentDiametersArrayPath READ getSaltykovEquivalentDiametersArrayPath WRITE setSaltykovEquivalentDiametersArrayPath)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getCompiledLibraryName();

    /**
     * @brief getBrandingString Returns the branding string for the filter, which is a tag
     * used to denote the filter's association with specific plugins
     * @return Branding string
    */
    virtual const QString getBrandingString();

    /**
     * @brief getFilterVersion Returns a version string for this filter. Default
     * value is an empty string.
     * @return
     */
    virtual const QString getFilterVersion();

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getGroupName();

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getSubGroupName();

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    virtual const QString getHumanLabel();

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void setupFilterParameters();

    /**
     * @brief writeFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
     * @brief readFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
     * @brief execute Reimplemented from @see AbstractFilter class
     */
    virtual void execute();

    /**
    * @brief preflight Reimplemented from @see AbstractFilter class
    */
    virtual void preflight();

  signals:
    /**
     * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
     * be pushed from a user-facing control (such as a widget)
     * @param filter Filter instance pointer
     */
    void updateFilterParameters(AbstractFilter* filter);

    /**
     * @brief parametersChanged Emitted when any Filter parameter is changed internally
     */
    void parametersChanged();

    /**
     * @brief preflightAboutToExecute Emitted just before calling dataCheck()
     */
    void preflightAboutToExecute();

    /**
     * @brief preflightExecuted Emitted just after calling dataCheck()
     */
    void preflightExecuted();

  protected:
    FindSaltykovSizes();
    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();

    /**
     * @brief find_saltykov_sizes
     */
    void find_saltykov_sizes();

    /**
     * @brief do_saltykov
     * @param nA
     * @param Dmax
     * @param k
     * @return
     */
    int do_saltykov(std::vector<int> nA, float Dmax, int k);

    /**
     * @brief forward_difference
     * @param fx
     * @param f1
     * @param f0
     * @param x1
     * @param x0
     * @return
     */
    int forward_difference(int fx, int f1, int f0, int x1, int x0);

    /**
     * @brief round_to_nearest_int
     * @param x
     * @return
     */
    template<class T>
    int round_to_nearest_int(T x);

  private:
    DEFINE_DATAARRAY_VARIABLE(float, EquivalentDiameters)
    DEFINE_DATAARRAY_VARIABLE(float, SaltykovEquivalentDiameters)

    FindSaltykovSizes(const FindSaltykovSizes&); // Copy Constructor Not Implemented
    void operator=(const FindSaltykovSizes&); // Operator '=' Not Implemented
};

#endif /* FindSaltykovSizes_H_ */



