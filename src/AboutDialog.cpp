#include "AboutDialog.h"

#include "DBAccess.h"
#include "MetricAggregator.h"
#include <QtSql>
#include <boost/version.hpp>

#ifndef GC_VERSION
#define GC_VERSION "(developer build)"
#endif


#define GCC_VERSION QString("%1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__)

#ifdef GC_HAVE_QWTPLOT3D
#include "qwt3d_global.h"
#endif

#ifdef GC_HAVE_ICAL
#include "ICalendar.h"
#endif

#ifdef GC_HAVE_D2XX
#include "D2XX.h"
#endif

#ifdef GC_HAVE_LIBOAUTH
#include <oauth.h>
#endif

#ifdef GC_HAVE_LUCENE
#include "Lucene.h"
#endif

AboutDialog::AboutDialog(MainWindow *mainWindow, QDir home) : mainWindow(mainWindow), home(home)
{
    setWindowTitle(tr("About GoldenCheetah"));

    aboutPage = new AboutPage(mainWindow, home);
    versionPage = new VersionPage(mainWindow, home);
    contributorsPage = new ContributorsPage(mainWindow, home);

    tabWidget = new QTabWidget;
    tabWidget->addTab(aboutPage, tr("About"));
    tabWidget->addTab(versionPage, tr("Version"));
    tabWidget->addTab(contributorsPage, tr("Contributors"));

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);

    setFixedSize(QSize(800, 600));
}


//
// About page
//
AboutPage::AboutPage(MainWindow *main, QDir home) : main(main), home(home)
{
    QLabel *text;
    text=new QLabel(this);
    text->setText(
              tr(
                "<center>"
                "<h2>GoldenCheetah</h2>"
                "Cycling Power Analysis Software<br>for Linux, Mac, and Windows"
                "<p>GoldenCheetah is licensed under the<br>"
                "<a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU General "
                "Public License</a>."
                "<p>Source code can be obtained from<br>"
                "<a href=\"http://goldencheetah.org/\">"
                "http://goldencheetah.org/</a>."
                "<p>Activity files and other data are stored in<br>"
                "<a href=\"%1\">%2</a>"
                "<p>Trademarks used with permission<br>"
                "TSS, NP, IF courtesy of <a href=\"http://www.peaksware.com\">"
                "Peaksware LLC</a>.<br>"
                "BikeScore, xPower courtesy of <a href=\"http://www.physfarm.com\">"
                "Physfarm Training Systems</a>.<br>"
                "Virtual Speed courtesy of Tom Compton <a href=\"http://www.analyticcycling.com\">"
                "Analytic Cycling</a>."
                "</center>"
                )
                .arg(QString(QUrl::fromLocalFile(main->home.absolutePath()).toEncoded()))
                .arg(main->home.absolutePath().replace(" ", "&nbsp;"))
    );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(text);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

//
// Version page
//
VersionPage::VersionPage(MainWindow *main, QDir home) : main(main), home(home)
{
    // -- OS ----
    QString os = "";

    #ifdef Q_OS_LINUX
    os = "Linux";
    #endif

    #ifdef WIN32
    os = "Win";
    #endif

    #ifdef Q_OS_MAC
    os = QString("Mac OS X 10.%1").arg(QSysInfo::MacintoshVersion - 2);
    if (QSysInfo::MacintoshVersion == QSysInfo::MV_SNOWLEOPARD)
        os += " Snow Leopard";
    else if (QSysInfo::MacintoshVersion == QSysInfo::MV_LION)
        os += " Lion";
    else if (QSysInfo::MacintoshVersion == 10)
        os += " Mountain Lion";

    #endif

    // -- SCHEMA VERSION ----
    int schemaVersionInt = main->metricDB->db()->getDBVersion();
    QString schemaVersion = QString("%1").arg(schemaVersionInt);

    // -- SRMIO ----
    QString srmio = "none";

    #ifdef GC_HAVE_SRMIO
    srmio = "yes";
    #endif

    // -- D2XX ----
    QString d2xx = "none";

    #ifdef GC_HAVE_D2XX
    d2xx = "yes";
    #endif

    // -- LIBOAUTH ----
    QString oauth = "none";

    #ifdef GC_HAVE_LIBOAUTH
    oauth = LIBOAUTH_VERSION;
    #endif

    // -- QWTPLOT3D ----
    QString qwtplot3d = "none";

    #ifdef GC_HAVE_QWTPLOT3D
    qwtplot3d = QString::number(QWT3D_MAJOR_VERSION) + "."
            + QString::number(QWT3D_MINOR_VERSION) + "."
            + QString::number(QWT3D_PATCH_VERSION);
    #endif

    // -- KML ----
    QString kml = "none";

    #ifdef GC_HAVE_KML
    kml = "yes";
    #endif

    // -- ICAL ----
    QString ical = "none";

    #ifdef GC_HAVE_ICAL
    ical = ICAL_VERSION;
    #endif

    // -- USBXPRESS ----
    QString usbxpress = "none";
    #ifdef GC_HAVE_USBXPRESS
    usbxpress = "yes";
    #endif

    // -- LIBUSB ----
    QString libusb = "none";
    #ifdef GC_HAVE_LIBUSB
    libusb = "yes";
    #endif

    // -- VLC ----
    QString vlc = "none";
    #ifdef GC_HAVE_VLC
    vlc = "yes";
    #endif

    // -- LUCENE ----
    QString clucene = "none";
    #ifdef GC_HAVE_LUCENE
    clucene = _CL_VERSION;
    #endif

    // -- LION SUPPORT ----
    #ifdef Q_OS_MAC
    QString lionSupport = "no";
    #ifdef GC_HAVE_LION
    lionSupport = "yes";
    #endif
    #endif

    QString gc_version = tr(
            "<h2>GoldenCheetah</h2>"
            "<p>Build date: %1 %2"
            "<p>Version: %3"
            "<p>DB Schema: %4"
            "<p>OS: %5"
            "<p>")
            .arg(__DATE__)
            .arg(__TIME__)
            .arg(GC_VERSION)
            .arg(schemaVersion)
            .arg(os);

    QString lib_version = tr(
            "<table width=300>"
            "<tr><td color=\"#770077\" colspan=\"2\">Versions</td></tr>"
            "<tr><td>QT</td><td>%1</td></tr>"
            "<tr><td>QWT</td><td>%2</td></tr>"
            "<tr><td>BOOST</td><td>%3</td></tr>"
            "<tr><td>GCC</td><td>%4</td></tr>"
            "<tr><td>SRMIO</td><td>%5</td></tr>"
            "<tr><td>OAUTH</td><td>%6</td></tr>"
            "<tr><td>F2XX</td><td>%7</td></tr>"
            "<tr><td>QWTPLOT3D</td><td>%8</td></tr>"
            "<tr><td>KML</td><td>%9</td></tr>"
            "<tr><td>ICAL</td><td>%10</td></tr>"
            "<tr><td>USBXPRESS</td><td>%11</td></tr>"
            "<tr><td>LIBUSB</td><td>%12</td></tr>"
            "<tr><td>VLC</td><td>%13</td></tr>"
            "<tr><td>LUCENE</td><td>%14</td></tr>"
            #ifdef Q_OS_MAC
            "<tr><td>LION SUPPORT</td><td>%15</td></tr>"
            #endif
            "</table>"
            )
            .arg(QT_VERSION_STR)
            .arg(QWT_VERSION_STR)
            .arg(BOOST_LIB_VERSION)
            .arg(GCC_VERSION)
            .arg(srmio)
            .arg(oauth)
            .arg(d2xx)
            .arg(qwtplot3d)
            .arg(kml)
            .arg(ical)
            .arg(usbxpress)
            .arg(libusb)
            .arg(vlc)
            .arg(clucene)
            #ifdef Q_OS_MAC
            .arg(lionSupport)
            #endif
            ;

    QLabel *text;
    text=new QLabel(this);
    text->setText("<center>"  +
                  gc_version  +
                  lib_version +
                  "</center>");


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(text);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

//
// Contributors page
//
ContributorsPage::ContributorsPage(MainWindow *main, QDir home) : main(main), home(home)
{
    QStringList contributors;
    contributors.append("Alejandro Martinez");
    contributors.append("Andrew Bryson");
    contributors.append("Andy Froncioni");
    contributors.append("Austin Roach");
    contributors.append("Berend De Schouwer");
    contributors.append("Bruno Assis");
    contributors.append("Chris Cleeland");
    contributors.append("Claus Assmann");
    contributors.append("Dag Gruneau");
    contributors.append("Damien Grauser");
    contributors.append("Darren Hague");
    contributors.append("Eric Brandt");
    contributors.append("Eric Murray");
    contributors.append("Frank Zschockelt");
    contributors.append("Gareth Coco");
    contributors.append("Greg Lonnon");
    contributors.append("Ilja Booij");
    contributors.append("Jamie Kimberley");
    contributors.append("Jim Ley");
    contributors.append("John Ehrlinger");
    contributors.append("Josef Gebel");
    contributors.append("Julian Baumgartner");
    contributors.append("Julian Simioni");
    contributors.append("Justin Knotzke");
    contributors.append("Keisuke Yamaguchi");
    contributors.append("Ken Sallot");
    contributors.append("Luke NRG");
    contributors.append("Magnus Gille");
    contributors.append("Marc Boudreau");
    contributors.append("Mark Liversedge");
    contributors.append("Mark Rages");
    contributors.append("Mitsukuni Sato");
    contributors.append("Ned Harding");
    contributors.append("Patrick McNerthney");
    contributors.append("Rainer Clasen");
    contributors.append("Robb Romans");
    contributors.append("Robert Carlsen");
    contributors.append("Roberto Massa");
    contributors.append("Ron Alford");
    contributors.append("Sean Rhea");
    contributors.append("Steven Gribble");
    contributors.append("Thomas Weichmann");
    contributors.append("Tom Weichmann");
    contributors.append("Walter B&#252;rki");

    QString contributorsTable = "<center><table width=600><tr>";
    for (int i=0;i<contributors.count();i++){
        contributorsTable.append("<td>"+contributors.at(i)+"</td>");
        if ((i+1) % 3 == 0)
            contributorsTable.append("</tr><tr>");
    }
    contributorsTable.append("</tr></table></center>");

    QLabel *text;
    text=new QLabel(this);
    text->setText(contributorsTable);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(text);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

