/*
  Copyright 2009 Lim Yuen Hoe <yuenhoe@hotmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <KDialog>
#include <KUrl>
#include <KProcess>

#include "ui_publisher.h"

class SigningWidget;

class Publisher : public KDialog
{
    Q_OBJECT;
public:
    Publisher(QWidget* parent, const KUrl &path, const QString& type);
    void setProjectName(const QString &name);

public slots:
    void doPlasmaPkg();

private slots:
    void doExport();
    void addSuffix();
    void doPublish();
    void doInstall();
    void doRemoteInstall();
    void checkInstallButtonState(int comboBoxCurrentIndex);

private:
    bool exportToFile(const KUrl& url);
    const QString tempPackagePath();

    //avoid duplication
    QString currentPackagePath() const;


    SigningWidget* m_signingWidget;
    KUrl m_projectPath;
    QString m_projectType;
    QString m_extension;
    QString m_projectName;
    int m_comboBoxIndex;

    Ui::Publisher m_ui;
};

#endif // PUBLISHER_H
