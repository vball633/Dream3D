/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * @file H5VoxelToVtk.cpp
 * @brief This program is mainly a debugging program to ensure the .h5voxel file
 * has the correct data since at the current time there is no way to visualize
 * the .h5voxel in ParaView or anything else. This will convert the .h5voxel to a
 * VTK structured points data file with Grain IDs and IPS Colors.
 */

#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "MXA/MXA.h"
#include "MXA/Common/MXASetGetMacros.h"

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DataArray.hpp"
#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/VTKUtils/VTKWriterMacros.h"
#include "DREAM3DLib/VTKUtils/VTKFileWriters.hpp"

#include "DREAM3DLib/HDF5/H5VoxelReader.h"


typedef struct {
    int xpoints;
    int ypoints;
    int zpoints;
    float resx; float resy; float resz;
    int* grain_indicies;
    int* phases;
    float* euler1s;
    float* euler2s;
    float* euler3s;
    float* quats;
    Ebsd::CrystalStructure* crystruct;
} Test;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  std::cout << "Starting Conversion of H5Voxel to VTK with Grain ID and IPF Colors" << std::endl;
  if (argc < 3)
  {
    std::cout << "This program takes 2 arguments: Input .h5voxel file and output vtk file." << std::endl;
    return EXIT_FAILURE;
  }


  std::string iFile = argv[1];

  int err = 0;

  DataContainer::Pointer m = DataContainer::New();

  H5VoxelReader::Pointer h5Reader = H5VoxelReader::New();
  h5Reader->setFileName(iFile);

  int64_t dims[3];
  float spacing[3];
  float origin[3];
  std::cout << "Getting Size and Resolution" << std::endl;
  err = h5Reader->getSizeResolutionOrigin(dims, spacing, origin);
  if (err < 0)
  {
    std::cout << "Error Reading the Dimensions and Resolution from the File." << std::endl;
    return EXIT_FAILURE;
  }
  /* Sanity check what we are trying to load to make sure it can fit in our address space.
   * Note that this does not guarantee the user has enough left, just that the
   * size of the volume can fit in the address space of the program
   */
#if   (CMP_SIZEOF_SSIZE_T==4)
    int64_t max = std::numeric_limits<size_t>::max();
#else
    int64_t max = std::numeric_limits<int64_t>::max();
#endif
  if (dims[0] * dims[1] * dims[2] > max )
  {
    err = -1;
    std::stringstream s;
    s << "The total number of elements '" << (dims[0] * dims[1] * dims[2])
                << "' is greater than this program can hold. Try the 64 bit version.";
    std::cout << s.str() << std::endl;
    return EXIT_FAILURE;
  }

  if (dims[0] > max || dims[1] > max || dims[2] > max)
  {
    err = -1;
    std::stringstream s;
    s << "One of the dimensions is greater than the max index for this sysem. Try the 64 bit version.";
    s << " dim[0]="<< dims[0] << "  dim[1]="<<dims[1] << "  dim[2]=" << dims[2];
    std::cout << s.str() << std::endl;
    return EXIT_FAILURE;
  }
  /* ************ End Sanity Check *************************** */
  size_t dcDims[3] = {dims[0], dims[1], dims[2]};
  m->setDimensions(dcDims);
  m->setResolution(spacing);
  m->setOrigin(origin);

  std::vector<Ebsd::CrystalStructure> crystruct;
  std::vector<DREAM3D::Reconstruction::PhaseType> phaseType;

  DECLARE_WRAPPED_ARRAY(grain_indicies, m_GrainIndicies, int)
  DECLARE_WRAPPED_ARRAY(phases, m_Phases, int);
  DECLARE_WRAPPED_ARRAY(eulerangles, m_EulerAngles, float);

  m_GrainIndicies = DataArray<int>::CreateArray(0);
  m_Phases = DataArray<int>::CreateArray(0);
  m_EulerAngles = DataArray<float>::CreateArray(0);


  int64_t totalpoints = dims[0] * dims[1] * dims[2];
  grain_indicies = m_GrainIndicies->WritePointer(0, totalpoints);
  phases = m_Phases->WritePointer(0, totalpoints);
  eulerangles = m_EulerAngles->WritePointer(0, 3*totalpoints);

  std::cout << "Reading Voxel Data" << std::endl;
  err = h5Reader->readVoxelData(grain_indicies, phases, eulerangles, crystruct, phaseType, totalpoints);
  if (err < 0)
  {
    std::cout << "Error reading h5voxel file." << std::endl;
    return EXIT_FAILURE;
  }

  m->addCellData(DREAM3D::CellData::GrainIds, m_GrainIndicies);
  m->addCellData(DREAM3D::CellData::Phases, m_Phases);
  m->addCellData(DREAM3D::CellData::EulerAngles, m_EulerAngles);

  int64_t totalPoints = m->totalPoints();

  m->crystruct = crystruct;

  std::cout << "Writing VTK file" << std::endl;

  FILE* f = fopen(argv[2], "wb");

  WRITE_STRUCTURED_POINTS_HEADER("ASCII", m)

  VoxelIPFColorScalarWriter<DataContainer> ipfWriter(m.get());
  ipfWriter.m_WriteBinaryFiles = false;
  ipfWriter.writeScalars(f);

  WRITE_VTK_GRAIN_IDS_ASCII(m, DREAM3D::CellData::GrainIds)

  fclose(f);


  std::cout << "Done Converting" << std::endl;
  return EXIT_SUCCESS;
}
