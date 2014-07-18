/*
Copyright 2009 Riccardo Iaconelli <riccardo@kde.org>

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KParts/MainWindow>
#include <KLibLoader>
#include <KAction>
#include <KService>
#include <KShortcut>
#include <QSize>

class QSvgWidget;
class QKeyEvent;
class QModelIndex;

namespace Ui
{
}

class QDockWidget;
class QStringList;

class EditPage;
class PackageModel;
class StartPage;
class Sidebar;
class TimeLine;
class MetaDataEditor;
class ImageViewer;
class TextEditor;
class KConfigXtEditor;
class KonsolePreviewer;

// our own previewer
class Previewer;
class DocBrowser;
class Publisher;

namespace KTextEditor
{
    class Document;
    class View;
} // namespace KTextEditor

class MainWindow : public KParts::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QStringList recentProjects();
    void checkMetafile(const QString &path);
    void updateSideBar();

public Q_SLOTS:
    void quit();
    void loadProject(const QString &name);
    void checkProjectrc();

Q_SIGNALS:
    void newSavePointClicked();
    void refreshRequested();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void setupActions();
    void initTimeLine();
    QString projectFilePath(const QString &filename);

private Q_SLOTS:
    void saveProjectState();
    void closeProject();
    void saveEditorData();
    void saveAndRefresh();
    void refreshNotes();
    void selectSavePoint();
    void selectPublish();

    void setNotesVisible(const bool visible);
    void setFileListVisible(const bool visible);

    void loadRequiredEditor(const KService::List offers, KUrl target);
    void loadMetaDataEditor(KUrl target);
    void loadImageViewer(const KUrl& target);
    void loadKConfigXtEditor(const KUrl& target);
    void updateActions();
    void toggleDocumentation();
    void toggleTimeLine();
    void togglePreview();
    void toggleNotes();
    void toggleFileList();
    void installPackage();
    void toggleKonsolePreviewer();
    void toggleActions();

    // refreshes editor, killing all local changes
    void editorDestructiveRefresh();

private:
    // QMainWindow takes control of and DELETES the previous centralWidget
    // whenever a new one is set - this is bad because we want to preserve
    // the state of the previous centralWidget for when it becomes active again.
    // This class is a workaround - we set an instance as the permanent
    // centralWidget, and use it to do graceful, non-destructive widget-switching.
    class CentralContainer : public QWidget
    {
    public:
        enum SwitchMode {
            Preserve,
            DeleteAfter
        };
        CentralContainer(QWidget* parent);
        void switchTo(QWidget* newWidget, SwitchMode mode = Preserve);
    private:
        SwitchMode m_curMode;
        QLayout *m_layout;
        QWidget *m_curWidget;
    };

    enum WorkflowTabs { StartPageTab = 0,
                        EditTab,
                        SavePoint,
                        PublishTab,
                        DocsTab,
                        PreviewTab
                      };

    void showDocumentation(bool show);
    void createMenus();
    void setupTextEditor(KTextEditor::Document *editorPart, KTextEditor::View *view);
    void loadNotesEditor(QDockWidget *container);
    Previewer* createPreviewerFor(const QString& projectType);
    KonsolePreviewer* createKonsoleFor(const QString& projectType, const QString &projectPath = 0);

    KAction *addAction(QString text, const char * icon,const  char *slot, const char *name, const KShortcut &shortcut = KShortcut());

    StartPage *m_startPage;
    Sidebar *m_sidebar;
    TimeLine    *m_timeLine;
    Previewer *m_previewerWidget;
    MetaDataEditor *m_metaEditor;
    Publisher *m_publisher;
    DocBrowser *m_browser;
    QDockWidget *m_notesWidget;
    TextEditor *m_textEditor;
    KConfigXtEditor *m_kconfigXtEditor;
    QScopedPointer<KonsolePreviewer> m_konsoleWidget;

    QDockWidget *m_filelist;
    EditPage *m_editPage;
    ImageViewer *m_imageViewer;
    PackageModel *m_model;
    int m_oldTab;
    bool m_docksCreated;
    bool m_isPlasmateCreatedPackage;
    CentralContainer *m_central;
    QString m_currentProject;
    QString m_packageType;
    QString m_packagePath;
    KService::Ptr m_partService;
    KParts::ReadOnlyPart *m_part;
    KParts::ReadOnlyPart *m_notesPart;

    QSvgWidget *m_svgWidget;
    bool checkImageSize(const QSize& size);
};

#endif // MAINWINDOW_H
