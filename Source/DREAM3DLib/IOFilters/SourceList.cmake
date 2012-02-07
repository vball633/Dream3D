#--////////////////////////////////////////////////////////////////////////////
#--
#--  Copyright (c) 2011, Michael A. Jackson. BlueQuartz Software
#--  Copyright (c) 2011, Michael Groeber, US Air Force Research Laboratory
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--
#--////////////////////////////////////////////////////////////////////////////

SET (IOFilters_FILTERS_HDRS
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DataContainerReader.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DataContainerWriter.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/FieldDataCSVWriter.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkRectilinearGridWriter.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DxWriter.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DxReader.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/PhWriter.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/PhReader.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkGrainIdReader.h
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkGrainIdWriter.h
)


#-- Add in any addition NON Filter classes here
SET (DREAM3DLib_IOFilters_HDRS
      ${IOFilters_FILTERS_HDRS}
)

SET (DREAM3DLib_IOFilters_SRCS
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DxWriter.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DxReader.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/PhWriter.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/PhReader.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkGrainIdReader.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkGrainIdWriter.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DataContainerReader.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/DataContainerWriter.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/FieldDataCSVWriter.cpp
    ${DREAM3DLib_SOURCE_DIR}/IOFilters/VtkRectilinearGridWriter.cpp
)
cmp_IDE_SOURCE_PROPERTIES( "DREAM3DLib/IOFilters" "${DREAM3DLib_IOFilters_HDRS}" "${DREAM3DLib_IOFilters_SRCS}" "0")
if ( ${PROJECT_INSTALL_HEADERS} EQUAL 1 )
    INSTALL (FILES ${DREAM3DLib_IO_HDRS}
            DESTINATION include/DREAM3D/IO
            COMPONENT Headers   )
endif()

