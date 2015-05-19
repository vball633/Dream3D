/* ============================================================================
* Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
* Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
* Copyright (c) 2012 Joseph B. Kleingers (Student Research Assistant)
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
* Neither the name of Michael A. Groeber, Michael A. Jackson, Joseph B. Kleingers,
* the US Air Force, BlueQuartz Software nor the names of its contributors may be
* used to endorse or promote products derived from this software without specific
* prior written permission.
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

#ifndef _DREAM3DSettings_H_
#define _DREAM3DSettings_H_

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStack>

#include <QtWidgets/QTreeWidget>

#include "DREAM3DWidgetsLib/Widgets/FilterLibraryTreeWidget.h"

#include "QtSupportLib/QtSupportLib.h"

struct DREAM3DSettingsGroup
{
  DREAM3DSettingsGroup(QString name, QJsonObject object)
  {
    groupName = name;
    group = object;
  }

  QString groupName;
  QJsonObject group;
};

class QtSupportLib_EXPORT DREAM3DSettings : public QObject
{
  Q_OBJECT

public:
  DREAM3DSettings(QObject * parent = 0);
  DREAM3DSettings(const QString &filePath, QObject * parent = 0);
  ~DREAM3DSettings();

  QString fileName();

  bool contains(const QString &key);

  bool beginGroup(const QString &prefix);
  void endGroup();

  QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
  QJsonObject value(const QString &key, const QJsonObject &defaultObject = QJsonObject());
  QStringList value(const QString &key, const QStringList &defaultList = QStringList());

  void setValue(const QString &key, const QVariant &value);
  void setValue(const QString &key, const QJsonObject &object);
  void setValue(const QString &key, const QStringList &list);

private:
  QString m_FilePath;

  QStack<DREAM3DSettingsGroup*> m_Stack;

  void openFile();
  void closeFile();
  void writeToFile();

  DREAM3DSettings(const DREAM3DSettings&);    // Copy Constructor Not Implemented
  void operator=(const DREAM3DSettings&);  // Operator '=' Not Implemented
};

#endif /* _DREAM3DSettings_H */
