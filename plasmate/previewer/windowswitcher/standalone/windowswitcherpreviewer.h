/********************************************************************
 * This file is part of the KDE project.
 *
 * Copyright (C) 2012 Antonis Tsiapaliokas <kok3rs@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/

#ifndef WINDOWSWITCHERPREVIEWER_H
#define WINDOWSWITCHERPREVIEWER_H

#include "../tabboxpreviewer.h"
#include <KUrlRequester>
#include <QVBoxLayout>
#include <KDialog>

class WindowSwitcherPreviewer : public KDialog {

    Q_OBJECT

public:

    WindowSwitcherPreviewer(QWidget *parent = 0);

public Q_SLOTS:

    void loadPreviewer(const QString& filePath);

private:

    QWidget *tmpWidget;
    QVBoxLayout *tmpLayout;
    TabBoxPreviewer *m_previewer;
    KUrlRequester *m_filePath;
};

#endif // WINDOWSWITCHERPREVIEWER_H