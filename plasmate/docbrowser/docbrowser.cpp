/*
Copyright 2009 Lim Yuen Hoe <yuenhoe@hotmail.com>

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

#include "docbrowser.h"

#include <QWebView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <KPushButton>
#include <KLineEdit>
#include <KLocalizedString>
#include <KUrl>

#include "packagemodel.h"

DocBrowser::DocBrowser(const PackageModel *package, QWidget *parent)
    : QDockWidget(i18n("Documentation"), parent),
      m_package(package)
{
    QWidget *widget = new QWidget(this);
    m_view = new QWebView(this);
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(focusSearchField()));

    QHBoxLayout *topbar = new QHBoxLayout();
    QHBoxLayout *btmbar = new QHBoxLayout();

    KPushButton *linkButton = new KPushButton(KIcon("favorites"), i18n("Tutorials"), this);
    connect(linkButton, SIGNAL(clicked()), this, SLOT(showTutorial()));
    topbar->addWidget(linkButton);

    linkButton = new KPushButton(KIcon("favorites"), i18n("API Reference"), this);
    connect(linkButton, SIGNAL(clicked()), this, SLOT(showApi()));
    topbar->addWidget(linkButton);

    linkButton = new KPushButton(KIcon("plasmagik"), i18n("Plasmate"), this);
    connect(linkButton, SIGNAL(clicked()), this, SLOT(showHelp()));
    topbar->addWidget(linkButton);

    m_searchLabel = new QLabel(i18n("Find:"));
    btmbar->addWidget(m_searchLabel);

    // TODO: should probably respond to the common 'Ctrl-F' by
    //       highlight-focusing the search field
    m_searchField = new KLineEdit(this);
    connect(m_searchField, SIGNAL(textChanged(const QString&)), this, SLOT(findText(const QString&)));
    connect(m_searchField, SIGNAL(returnPressed()), this, SLOT(findNext()));
    btmbar->addWidget(m_searchField);

    KPushButton *searchButton = new KPushButton(i18n("Next"), this);
    connect(searchButton, SIGNAL(clicked()), this, SLOT(findNext()));
    btmbar->addWidget(searchButton);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(topbar);
    layout->addWidget(m_view);
    layout->addLayout(btmbar);
    widget->setLayout(layout);
    setWidget(widget);
    showTutorial();
}

void DocBrowser::findText(const QString& toFind)
{
    if (!m_view->findText(toFind, QWebPage::FindWrapsAroundDocument)) {
        m_searchLabel->setText(i18n("(Text not found)"));
    } else {
        m_searchLabel->setText(i18n("Find:"));
    }
}

void DocBrowser::findNext()
{
    m_view->findText(m_searchField->text(), QWebPage::FindWrapsAroundDocument);
}

KUrl DocBrowser::currentPage() const
{
    return KUrl(m_view->url());
}

void DocBrowser::load(KUrl page)
{
    m_view->load(page);
}

void DocBrowser::setPackage(const PackageModel *package)
{
    m_package = package;
}

const PackageModel *DocBrowser::package() const
{
    return m_package;
}

void DocBrowser::showApi()
{
    m_view->setHtml("<center><h3>" + 
                      i18n("Loading API reference...") + 
                      "</h3></center>");
    QUrl url("http://api.kde.org/4.x-api/kdelibs-apidocs/plasma/html/annotated.html");

    if (m_package) {
        //TODO: API specific to each kind of package!
        //kDebug() << m_package->implementationApi() << m_package->packageType();
        if (m_package->packageType() == "Plasma/Applet") {
            if (m_package->implementationApi().toLower() == "javascript") {
                url = QUrl("http://techbase.kde.org/Development/Tutorials/Plasma/JavaScript/API");
            }
        }
    }

    kDebug() << "loading" << url;
    m_view->load(url);
}

void DocBrowser::showTutorial()
{
    m_view->setHtml("<center><h3>" + 
                      i18n("Loading tutorials...") + 
                      "</h3></center>");
    m_view->load(QUrl("http://techbase.kde.org/Development/Tutorials/Plasma"));
}

void DocBrowser::showHelp()
{
    m_view->setHtml("<center><h3>" + 
                      i18n("Loading Plasmate Guide...") + 
                      "</h3></center>");
    m_view->load(QUrl("http://userbase.kde.org/Plasmate"));
}

void DocBrowser::focusSearchField()
{
    m_searchField->setFocus(Qt::OtherFocusReason);
    m_searchField->selectAll();
}
