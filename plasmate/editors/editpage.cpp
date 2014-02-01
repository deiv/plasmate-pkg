/*

Copyright 2009-2010 Aaron Seigo <aseigo@kde.org>
Copyright 2009-2010 Artur Duque de Souza <asouza@kde.org>
Copyright 2009-2010 Laurent Montel <montel@kde.org>
Copyright 2009-2010 Shantanu Tushar Jha <shaan7in@gmail.com>
Copyright 2009-2010 Sandro Andrade <sandroandrade@kde.org>
Copyright 2009-2010 Lim Yuen Hoe <yuenhoe@hotmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "editpage.h"

#include <QHBoxLayout>
#include <QFile>
#include <QList>
#include <QPixmap>
#include <QStringList>
#include <KConfigGroup>
#include <kfiledialog.h>
#include <KIcon>
#include <KMessageBox>
#include <KInputDialog>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/JobClasses>
#include <KIO/MimetypeJob>
#include <KIO/Job>
#include <KUser>
#include <kmimetypetrader.h>

#include "packagemodel.h"

#include <qvarlengtharray.h>

EditPage::EditPage(QWidget *parent)
        : QTreeView(parent),
        m_metaEditor(0)
{
    setHeaderHidden(true);
    m_contextMenu = new KMenu(this);
    QAction *del = m_contextMenu->addAction(KIcon("window-close"), i18n("Delete"));
    connect(del, SIGNAL(triggered(bool)), this, SLOT(doDelete(bool)));

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(activated(const QModelIndex &)), this, SLOT(findEditor(const QModelIndex &)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showTreeContextMenu(const QPoint&)));
}

void EditPage::doDelete(bool)
{
    QModelIndexList items = selectedIndexes();
    if (items.empty()) {
        return; // this shouldn't happen
    }
    QModelIndex selected = items.at(0); // only one can be selected
    const QString path = selected.data(PackageModel::UrlRole).toString();
    const QString name = selected.data(Qt::DisplayRole).toString();
    const QString dialogText = i18n("Are you sure you want to delete the file \"%1\"?", name);
    int code = KMessageBox::warningContinueCancel(this, dialogText);
    if (code == KMessageBox::Continue) {
        QFile::remove(path);

        QDir dirPath(path);
        //our path points to a file, so go up one dir
        dirPath.cdUp();
        foreach(const QFileInfo& fileInfo, dirPath.entryInfoList(QDir::AllEntries)) {
            if (fileInfo.isFile()) {
                //if there is no file the foreach
                //will end and the directory will be deleted
                return;
            }
        }
        KIO::del(dirPath.path());
    }
}

void EditPage::showTreeContextMenu(const QPoint&)
{
    QModelIndexList items = selectedIndexes();
    if (items.empty()) {
        return;
    }

    const QStringList mimeTypeList = items.at(0).data(PackageModel::MimeTypeRole).toStringList();
    const QStringList pathList = items.at(0).data(PackageModel::UrlRole).toStringList();

    if (mimeTypeList.isEmpty() ||
        pathList.isEmpty() ||
        mimeTypeList.at(0).startsWith("[plasmate]/") ||
        !QFile::exists(pathList.at(0))) {
        return;
    }

    m_contextMenu->popup(QCursor::pos());
}

void EditPage::findEditor(const QModelIndex &index)
{
    const QStringList mimetypes = index.data(PackageModel::MimeTypeRole).toStringList();
    const QString packagePath = index.data(PackageModel::PackagePathRole).toString();
    const QString contentWithSubdir = index.data(PackageModel::ContentsWithSubdirRole).toString();

    foreach (const QString &mimetype, mimetypes) {
        QString target = index.data(PackageModel::UrlRole).toUrl().toString();
        if (mimetype == "[plasmate]/metadata") {
            emit loadMetaDataEditor(target);
            return;
        }

        if (mimetype == "[plasmate]/imageDialog") {
            imageDialog("*.png *.gif *.svg *.jpeg *.svgz", "contents/images/");
            return;
        }

        QString commonFilter = "*.svg *.svgz";

        const QString themeType = "[plasmate]/themeImageDialog/";
        if (mimetype.startsWith(themeType)) {
            const QString opts = "contents/" + mimetype.right(mimetype.size() - themeType.length());
            imageDialog(commonFilter, opts);
            return;
        }

        if (mimetype == "[plasmate]/imageViewer") {
            emit loadImageViewer(target);
            return;
        }

        if (mimetype == "[plasmate]/kconfigxteditor") {
            emit loadKConfigXtEditor(target);
            return;
        }

        bool createNewFile = false;
        QString nameOfNewFile;
        if (mimetype == "[plasmate]/mainconfigxml/new") {
            createNewFile = true;
            nameOfNewFile = "main.xml";
        }

        if (mimetype == "[plasmate]/mainconfigui") {
            createNewFile = true;
            nameOfNewFile = "config.ui";
        }

        if (createNewFile) {
            QString path = createContentWithSubdir(packagePath, contentWithSubdir);
            if (!path.isEmpty()) {
                if (!path.endsWith('/')) {
                    path.append('/');
                }

                target.clear();
                target.append(path + nameOfNewFile);
            }

            QFile f(target);
            f.open(QIODevice::ReadWrite); // create the file
            return;
        }

        if (mimetype == "[plasmate]/kcolorscheme") {
            QFile f(target);
            f.open(QIODevice::ReadWrite); // create the file
            return;
        }

        if (mimetype == "[plasmate]/new") {
            target = createContentWithSubdir(packagePath, contentWithSubdir);

            const QString dialogText = i18n("Enter a name for the new file:");
            QString file = KInputDialog::getText(QString(), dialogText);
            if (!file.isEmpty()) {
                kDebug() << target;
                if (!m_metaEditor) {
                    m_metaEditor = new MetaDataEditor(this);
                    m_metaEditor->setFilename(packagePath + "/metadata.desktop");
                }

                if (m_metaEditor->isValidMetaData()) {
                    const QString api = m_metaEditor->api();

                    //we don't need the m_metaEditor anymore
                    delete m_metaEditor;
                    m_metaEditor = 0;

                    if (!api.isEmpty()) {
                        if (!hasExtension(file)) {
                            if (api =="Ruby" && !file.endsWith(".rb")) {
                                file.append(".rb");
                            } else if (api =="Python" && !file.endsWith(".py")) {
                                file.append(".py");
                            } else if (api =="Javascript" && !file.endsWith(".js")) {
                                file.append(".js");
                            } else if (api =="declarativeappletscript" && !file.endsWith(".qml")) {
                                file.append(".qml");
                            }
                        }
                    }


                    file = target + file;

                    QFile fl(file);
                    fl.open(QIODevice::ReadWrite); // create the file
                    fl.close();
                }

            }

            return;
        }

        KService::List offers = KMimeTypeTrader::self()->query(mimetype, "KParts/ReadWritePart");
        //kDebug() << mimetype;
        if (offers.isEmpty()) {
            offers = KMimeTypeTrader::self()->query(mimetype, "KParts/ReadOnlyPart");
        }

        if (!offers.isEmpty()) {
            //create the part using offers.at(0)
            //kDebug() << offers.at(0);
            //offers.at(0)->createInstance(parentWidget);
            emit loadEditor(offers, KUrl(target));
            return;
        }
    }
}
void EditPage::imageDialog(const QString& filter, const QString& destinationPath)
{
    KUser user;
    KUrl homeDir = user.homeDir();
    const QList<KUrl> srcDir = KFileDialog::getOpenUrls(homeDir, filter, this);
    KConfigGroup cg(KGlobal::config(), "PackageModel::package");
    const KUrl destinationDir(cg.readEntry("lastLoadedPackage", QString()) + destinationPath);
    QDir destPath(destinationDir.pathOrUrl());
    if (!destPath.exists()) {
        destPath.mkpath(destinationDir.pathOrUrl());
    }

    if (!srcDir.isEmpty()) {
        foreach(const KUrl source, srcDir) {
            KIO::copy(source, destinationDir, KIO::HideProgressInfo);
        }
    }
}

QString EditPage::createContentWithSubdir(const QString& packagePath, const QString& contentWithSubdir) const
{
    //now create the content and the subdir directory.
    //Q: why now and not in the Packagemodel?
    //A: Because now the user want to create the subdir,
    //in the PackageModel he just clicked on the entry.
    QString target;
    QDir dir(packagePath);
    if (!dir.exists(contentWithSubdir)) {
        dir.mkpath(contentWithSubdir);
        target = packagePath + contentWithSubdir;
        if (!target.endsWith('/')) {
            target.append('/');
        }
    }

    return target;
}

bool EditPage::hasExtension(const QString& filename)
{
    QStringList list;
    list << ".rb" << ".js" << ".qml" << ".py" << ".xml";
    foreach (const QString &str, list) {
        if (filename.endsWith(str)) {
            return true;
        }
    }
    return false;
}


void EditPage::loadFile(const KUrl &path)
{
    m_path = path;

    kDebug() << "Loading file: " << path;

    KIO::JobFlags flags = KIO::HideProgressInfo;
    KIO::MimetypeJob *mjob = KIO::mimetype(path, flags);

    connect(mjob, SIGNAL(finished(KJob*)), this, SLOT(mimetypeJobFinished(KJob*)));
}

void EditPage::mimetypeJobFinished(KJob *job)
{
    KIO::MimetypeJob *mjob = qobject_cast<KIO::MimetypeJob *>(job);

    if (!job) {
        return;
    }

    if (mjob->error()) {
        return;
    }

    m_mimetype = mjob->mimetype();

    if (m_mimetype.isEmpty()) {
        kDebug() << "Could not detect the file's mimetype";
        return;
    }

    kDebug() << "loaded mimetype: " << m_mimetype;

    KService::List offers = KMimeTypeTrader::self()->query(m_mimetype, "KParts/ReadWritePart");
    if (offers.isEmpty()) {
        offers = KMimeTypeTrader::self()->query(m_mimetype, "KParts/ReadOnlyPart");
    }

    if (!offers.isEmpty()) {
        //create the part using offers.at(0)
        //kDebug() << offers.at(0);
        //offers.at(0)->createInstance(parentWidget);
        emit loadEditor(offers, m_path);
        return;
    }
    kDebug() << "loading" << m_path;
}
