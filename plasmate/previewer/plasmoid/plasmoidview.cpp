/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007 Aaron Seigo <aseigo@kde.org>
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 * Copyright 2009 Artur Duque de Souza <morpheuz@gmail.com>
 *
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

#include "plasmoidview.h"
#include "previewcontainment.h"

#include <QFileInfo>
#include <QDir>
#include <KServiceTypeTrader>
#include <Plasma/Containment>

PlasmoidView::PlasmoidView(QWidget *parent)
    : QGraphicsView(parent),
      m_containment(0),
      m_applet(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setScene(&m_corona);
    connect(&m_corona, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(sceneRectChanged(QRectF)));
    setAlignment(Qt::AlignCenter);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_containment = m_corona.addContainment("studiopreviewer");
    m_containment->setFormFactor(Plasma::Planar);
    m_containment->setLocation(Plasma::Floating);
    m_containment->setAspectRatioMode(Plasma::IgnoreAspectRatio);
    m_containment->setWallpaper("color");

    connect(m_containment, SIGNAL(appletRemoved(Plasma::Applet*)),
            this, SLOT(appletRemoved(Plasma::Applet*)));

    // we do a two-way connect here to allow the previewer containment
    // and main window to tell each other to save/refresh
    connect(m_containment, SIGNAL(refreshClicked()), parent, SLOT(emitRefreshRequest()));
    connect(m_containment, SIGNAL(showKonsole()), parent, SLOT(emitShowKonsole()));
    connect(parent, SIGNAL(refreshView()), m_containment, SLOT(refreshApplet()));

    setScene(&m_corona);
}

PlasmoidView::~PlasmoidView()
{
    if (!m_currentPath.isEmpty() && m_applet) {
        KConfigGroup cg;
        m_applet->save(cg);
        cg = m_applet->config();
        KConfigGroup storage = storageGroup();
        storage.deleteGroup();
        cg.copyTo(&storage);
    }

    if (m_containment) {
        KConfigGroup cg;
        m_containment->save(cg);
        cg = m_containment->config();
        KConfigGroup storage = containmentStorageGroup();
        storage.deleteGroup();
        cg.copyTo(&storage);
        storage.deleteGroup("Applets");
        m_containment->destroy(false);
    }

    KGlobal::config()->sync();
    m_corona.saveLayout();
}

bool PlasmoidView::hasStorageGroup() const
{
    if (m_currentPath.isEmpty()) {
        return false;
    }

    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredApplets");
    return stored.groupList().contains(m_currentPath);
}

KConfigGroup PlasmoidView::storageGroup() const
{
    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredApplets");
    return KConfigGroup(&stored, m_currentPath);
}

bool PlasmoidView::hasContainmentStorageGroup() const
{
    if (m_currentPath.isEmpty()) {
        return false;
    }

    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredContainments");
    return stored.groupList().contains(m_currentPath);
}

KConfigGroup PlasmoidView::containmentStorageGroup() const
{
    KConfigGroup stored = KConfigGroup(KGlobal::config(), "StoredContainments");
    return KConfigGroup(&stored, m_currentPath);
}

void PlasmoidView::addApplet(const QString &name, const QString &containment, const QVariantList &args)
{
    if (!containment.isEmpty()) {
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Containment", "[X-KDE-PluginInfo-Name] == '" + containment + "'");
        if (offers.count() > 0) {
            m_containment = m_corona.addContainment(containment);
            setScene(m_containment->scene());
        }
    }

    if (m_applet) {
        if (m_applet->pluginName() == name) {
            return;
        }

        delete m_applet;
        m_applet = 0;
    }

    QFileInfo info(name);
    if (!info.isAbsolute()) {
        info = QFileInfo(QDir::currentPath() + "/" + name);
    }

    m_currentPath = info.absoluteFilePath();
    // load from package if we have a path
    if (info.exists()) {
        m_applet = Applet::loadPlasmoid(m_currentPath);
    }

    if (m_applet) {
        if (hasContainmentStorageGroup()) {
            KConfigGroup cg = m_containment->config();
            KConfigGroup storage = containmentStorageGroup();
            cg.deleteGroup();
            storage.copyTo(&cg);
            cg.deleteGroup("Applets");
            m_containment->configChanged();
        }

        m_containment->addApplet(m_applet, QPointF(-1, -1), false);
        if (hasStorageGroup()) {
            KConfigGroup cg = m_applet->config();
            KConfigGroup storage = storageGroup();
            cg.deleteGroup();
            storage.copyTo(&cg);
            m_applet->configChanged();
        }
    } else {
        m_currentPath.clear();
        m_applet = m_containment->addApplet(name, args, QRectF(0, 0, -1, -1));
    }

    if (m_applet) {
        m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);
    }

    //resize(m_applet->preferredSize().toSize());
}

void PlasmoidView::clearApplets()
{
    m_containment->clearApplets();
}

void PlasmoidView::appletRemoved(Plasma::Applet *applet)
{
    if (applet == m_applet) {
        m_applet = 0;
    }
}

void PlasmoidView::sceneRectChanged(const QRectF &rect)
{
    Q_UNUSED(rect);
    if (m_containment) {
        setSceneRect(m_containment->geometry());
    }
}

void PlasmoidView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);

    // if we do not have any applet being shown
    // there is no need to do all this stuff
    //if (!m_applet || m_applet->aspectRatioMode() != Plasma::KeepAspectRatio) {
    if (m_applet) {
        m_applet->setMaximumWidth(size().width());
        m_containment->resize(size());
        return;
    }
    m_containment->resize(event->size());
    centerOn(m_containment);
    return;
}

