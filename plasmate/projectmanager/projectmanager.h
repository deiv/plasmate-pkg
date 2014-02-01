/*
 *   Copyright 2009 Lim Yuen Hoe <yuenhoe@hotmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <KDialog>

#include <mainwindow.h>

class QListWidget;
class QListWidgetItem;

class KUrl;
class KPushButton;
class KMenu;

namespace Ui
{
class ProjectManager;
}

class ProjectManager : public KDialog
{
    Q_OBJECT
public:
    ProjectManager(QWidget *parent);

    void addProject(QListWidgetItem *item);
    void clearProjects();

    static bool exportPackage(const KUrl &toExport, const KUrl &targetFile);
    static bool importPackage(const KUrl &toImport, const KUrl &targetLocation);

public Q_SLOTS:
    void addRecentProject(const QString &path);
    void setRecentProjects(const QStringList &paths);

signals:
    void projectSelected(const QString &name);
    void requestRefresh();

private:
    void deleteProject(const KUrl &projectLocation);

private Q_SLOTS:
   void emitProjectSelected();
   void removeSelectedProjects(bool deleteFromDisk);
   void confirmRemoveFromDisk();
   void confirmRemoveFromList();
   void checkButtonState();

private:
//TODO: Implement:
//      search/filterbar,
//      project multi-export,
//      project multi-import.
    QListWidget *m_projectList;
    KPushButton *m_loadButton;
    KPushButton *m_removeMenuButton;
    KMenu *m_removeMenu;
    bool m_destroyFlag;
    MainWindow* m_mainWindow;
};

#endif // PROJECTMANAGER_H
