/*
 * Copyright (c) 2020 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _GC_GenericChart_h
#define _GC_GenericChart_h 1

#include "GoldenCheetah.h"
#include "Settings.h"
#include "Context.h"
#include "Athlete.h"
#include "Colors.h"
#include "RCanvas.h"

#include "GenericSelectTool.h"
#include "GenericLegend.h"
#include "GenericPlot.h"

// keeping track of the series info
class GenericSeriesInfo {

    public:

        GenericSeriesInfo(QString name, QVector<double> xseries, QVector<double> yseries, QString xname, QString yname,
                      QStringList labels, QStringList colors,
                      int line, int symbol, int size, QString color, int opacity, bool opengl) :
                      name(name), xseries(xseries), yseries(yseries), xname(xname), yname(yname),
                      labels(labels), colors(colors),
                      line(line), symbol(symbol), size(size), color(color), opacity(opacity), opengl(opengl)
                      {}

        // properties, from setCurve(...)
        QString name;
        QVector<double> xseries;
        QVector<double> yseries;
        QString xname;
        QString yname;
        QStringList labels;
        QStringList colors;
        int line;
        int symbol;
        int size;
        QString color;
        int opacity;
        bool opengl;
};

// keeping track of all our plots
class GenericPlotInfo {

    public:

        // when working out what to do with existing plots
        enum { init, active, matched, deleteme } state;

        // initial
        GenericPlotInfo(QString xaxis) : state(init), plot(NULL), xaxis(xaxis) {}

        bool matches(const GenericPlotInfo &other) const {
            // we need to have same series and same xaxis
            if (other.xaxis != xaxis) return false;

            // same number of series
            if (other.series.count() != series.count()) return false;

            // all my series are in their series?
            foreach(GenericSeriesInfo info, series) {
                bool found=false;
                // is in other?
                foreach(GenericSeriesInfo oinfo, other.series) {
                    if (oinfo.name == info.name && oinfo.yname == info.yname && oinfo.xname == info.xname)
                        found=true;
                }
                if (found == false) return false;
            }
            return true;
        }

        static int findPlot(const QList<GenericPlotInfo>list, const GenericPlotInfo findme) {
            for(int i=0; i<list.count(); i++) {
                // find a plot with same xaxis and series
                if (list[i].matches(findme))
                    return i;
            }
            return -1;
        }

        // the plot object created- and then matched against
        GenericPlot *plot;

        // axes
        QString xaxis;

        QList<GenericSeriesInfo> series;
        QList<GenericAxisInfo> axes;
};

// general axis info
class GenericAxisInfo {
public:
        GenericAxisInfo(QString name, bool visible, int align, double min, double max,
                      int type, QString labelcolor, QString color, bool log, QStringList categories) :
                      type(static_cast<AxisInfoType>(type)),
                      name(name), align(static_cast<Qt::AlignmentFlag>(align)),
                      minx(min), maxx(max), visible(visible), log(log),
                      labelcolorstring(labelcolor), axiscolorstring(color),  categories(categories)
                      {}

        enum axisinfoType { CONTINUOUS=0,                 // Continious range
                            DATERANGE=1,                  // Date
                            TIME=2,                       // Duration, Time
                            CATEGORY=3                // labelled with categories
                          };
        typedef enum axisinfoType AxisInfoType;

        static int findAxis(QList<GenericAxisInfo>infos, QString name) {
            for (int i=0; i<infos.count(); i++)
                if (infos[i].name == name)
                    return i;
            return -1; // not found
        }

        GenericAxisInfo(Qt::Orientations orientation, QString name) : name(name), orientation(orientation) {
            miny=maxy=minx=maxx=0;
            fixed=log=false;
            visible=minorgrid=majorgrid=true;
            type=CONTINUOUS;
            axiscolor=labelcolor=GColor(CPLOTMARKER);
        }

        void point(double x, double y) {
            if (fixed) return;
            if (x>maxx) maxx=x;
            if (x<minx) minx=x;
            if (y>maxy) maxy=y;
            if (y<miny) miny=y;
        }

        double min() {
            if (orientation == Qt::Horizontal) return minx;
            else return miny;
        }
        double max() {
            if (orientation == Qt::Horizontal) return maxx;
            else return maxy;
        }

        Qt::AlignmentFlag locate() {
            return align;
        }

        // series we are associated with
        QList<QAbstractSeries*> series;

        // data is all public to avoid tedious get/set
        AxisInfoType type; // what type of axis is this?
        QString name;
        Qt::Orientations orientation;
        Qt::AlignmentFlag align;
        double miny, maxy, minx, maxx; // updated as we see points, set the range
        bool visible,fixed, log, minorgrid, majorgrid; // settings
        QColor labelcolor, axiscolor; // aesthetics
        QString labelcolorstring, axiscolorstring;
        QStringList categories;
};

// the chart
class GenericChart : public QWidget {

    Q_OBJECT

    public:

        friend class GenericSelectTool;
        friend class GenericLegend;

        GenericChart(QWidget *parent, Context *context);

    public slots:

        // set chart settings
        bool initialiseChart(QString title, int type, bool animate, int legendpos, bool stack, int orientation);

        // add a curve, associating an axis
        bool addCurve(QString name, QVector<double> xseries, QVector<double> yseries, QString xname, QString yname,
                      QStringList labels, QStringList colors,
                      int line, int symbol, int size, QString color, int opacity, bool opengl);

        // configure axis, after curves added
        bool configureAxis(QString name, bool visible, int align, double min, double max,
                      int type, QString labelcolor, QString color, bool log, QStringList categories);

        // post processing clean up / add decorations / helpers etc
        void finaliseChart();

    protected:

        // legend and selector need acces to these
        QVBoxLayout *mainLayout;

        // chart settings
        QString title;
        int type;
        bool animate;
        int legendpos;
        bool stack; // stack series instead of on same chart
        int orientation; // layout horizontal or vertical

        // when we get new settings/calls we
        // collect together to prepare before
        // actually creating plots since we want
        // to see the whole picture- try to reuse
        // plots as much as possible to avoid
        // the cost of creating new ones.
        QList<GenericSeriesInfo> newSeries;
        QList<GenericAxisInfo> newAxes;

        // active plots
        QList<GenericPlotInfo> currentPlots;

    private:
        Context *context;
};
#endif
