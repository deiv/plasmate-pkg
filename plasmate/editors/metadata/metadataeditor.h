/*
  Copyright 2009 Richard Moore, <rich@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef METADATAEDITOR_H
#define METADATAEDITOR_H

#include <QWidget>
#include <QStringList>

namespace Ui
{
class MetaDataEditor;
}

namespace Plasma
{
class PackageMetadata;
}

class MetaDataEditor : public QWidget
{
    Q_OBJECT

public:
    MetaDataEditor(QWidget *parent = 0);
    ~MetaDataEditor();

    enum apiModes {
        coreApi,
        uiApi
    };

    void setFilename(const QString &filename);
    QString formatApi(const QString &api, apiModes apiMode);
    const QString filename();
    const QString api();
    bool isValidMetaData();
public slots:
    void readFile();
    void writeFile();

Q_SIGNALS:
    void apiChanged();
private slots:
    void serviceTypeChanged();

private:
    Ui::MetaDataEditor *m_view;
    QString m_filename;
    Plasma::PackageMetadata *m_metadata;
    QStringList m_apis;
    QStringList m_categories;

    void initCatergories(const QString& serviceType);
};

#endif // METADATAEDITOR__H
