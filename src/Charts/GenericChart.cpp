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

#include "GenericChart.h"

#include "Colors.h"
#include "TabView.h"
#include "RideFileCommand.h"
#include "Utils.h"

#include <limits>

//
// The generic chart manages collections of genericplots
// if the stack option is selected we create a plot for
// each data series.
//
// If series do not have a common x-axis we create a
// separate plot, with series that share a common x-axis
// sharing the same plot
//
// The layout is horizontal or vertical in which case the
// generic chart will use vbox or hbox layouts. XXX TODO
//
// Charts have a minimum width and height that needs to be
// honoured too, since multiple series stacked can get
// cramped, so these are placed into a scroll area XXX TODO
//
GenericChart::GenericChart(QWidget *parent, Context *context) : QWidget(parent), context(context)
{
    // intitialise state info
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);
}

// set chart settings
bool
GenericChart::initialiseChart(QString title, int type, bool animate, int legendpos, bool stack, int orientation)
{
    // Remember the settings, we use them for every new plot
    this->title = title;
    this->type = type;
    this->animate = animate;
    this->legendpos = legendpos;
    this->stack = stack;
    this->orientation = orientation;

    // store info as its passed to us
    newSeries.clear();
    newAxes.clear();

    return true;
}

// add a curve, associating an axis
bool
GenericChart::addCurve(QString name, QVector<double> xseries, QVector<double> yseries, QString xname, QString yname,
                      QStringList labels, QStringList colors,
                      int line, int symbol, int size, QString color, int opacity, bool opengl)
{

    newSeries << GenericSeriesInfo(name, xseries, yseries, xname, yname, labels, colors, line, symbol, size, color, opacity, opengl);
    return true;
}

// configure axis, after curves added
bool
GenericChart::configureAxis(QString name, bool visible, int align, double min, double max,
                      int type, QString labelcolor, QString color, bool log, QStringList categories)
{
    newAxes << GenericAxisInfo(name, visible, align, min, max, type, labelcolor, color, log, categories);
    return true;
}

// post processing clean up / add decorations / helpers etc
void
GenericChart::finaliseChart()
{
    setUpdatesEnabled(false); // lets not open our kimono here

    // ok, so lets work out what we need, run through the curves
    // and allocate to a plot, where we have separate plots if
    // we have stacked set -or- for each xaxis, remembering to
    // add the axisinfo each time too.
    QList<GenericPlotInfo> newPlots;
    QStringList xaxes;
    for(int i=0; i<newSeries.count(); i++) {

        if (stack) {

            // super easy we just have a new plot for each series
            GenericPlotInfo add(newSeries[i].xname);
            add.series << newSeries[i];

            // xaxis info
            int ax=GenericAxisInfo::findAxis(newAxes, newSeries[i].xname);
            if (ax>=0) add.axes << newAxes[ax];

            // yaxis info
            ax=GenericAxisInfo::findAxis(newAxes, newSeries[i].yname);
            if (ax>=0) add.axes << newAxes[ax];

            // add to list
            newPlots << add;
            xaxes << newSeries[i].xname;

        } else {

            // otherwise add all series to the same plot
            // with a common x axis
            int index = xaxes.indexOf(newSeries[i].xname);
            if (index >=0) {
                // woop, we already got this xaxis
                newPlots[index].series << newSeries[i];

                // yaxis info
                int ax=GenericAxisInfo::findAxis(newAxes, newSeries[i].yname);
                if (ax>=0) newPlots[index].axes << newAxes[ax];

            } else {

                GenericPlotInfo add(newSeries[i].xname);
                add.series << newSeries[i];

                // xaxis info
                int ax=GenericAxisInfo::findAxis(newAxes, newSeries[i].xname);
                if (ax>=0) add.axes << newAxes[ax];

                // yaxis info
                ax=GenericAxisInfo::findAxis(newAxes, newSeries[i].yname);
                if (ax>=0) add.axes << newAxes[ax];

                // add to list
                newPlots << add;
                xaxes << newSeries[i].xname;
            }
        }
    }

    // lets find existing plots or create new ones
    // first we mark all existing plots as deleteme
    for(int i=0; i<currentPlots.count(); i++) currentPlots[i].state = GenericPlotInfo::deleteme;

    // match what we want with what we got
    for(int i=0; i<newPlots.count(); i++) {
        int index = GenericPlotInfo::findPlot(currentPlots, newPlots[i]);
        if (index <0) {
            // new one required
            newPlots[i].plot = new GenericPlot(this, context);
            mainLayout->addWidget(newPlots[i].plot);
            mainLayout->setStretchFactor(newPlots[i].plot, 10);// make them all the same
        } else {
            newPlots[i].plot = currentPlots[index].plot; // reuse
            currentPlots[index].state = GenericPlotInfo::matched; // don't deleteme !
        }
    }

    // delete all the deleteme plots
    for(int i=0; i<currentPlots.count(); i++) {
        if (currentPlots[i].state == GenericPlotInfo::deleteme) {
            delete currentPlots[i].plot; // will remove from layout too
        }
    }

    // now initialise all the newPlots
    for(int i=0; i<newPlots.count(); i++) {

        // set initial parameters
        newPlots[i].plot->initialiseChart(title, type, animate, legendpos);

        // add curves
        QListIterator<GenericSeriesInfo>s(newPlots[i].series);
        while(s.hasNext()) {
            GenericSeriesInfo p=s.next();
            newPlots[i].plot->addCurve(p.name, p.xseries, p.yseries, p.xname, p.yname, p.labels, p.colors, p.line, p.symbol, p.size, p.color, p.opacity, p.opengl);
        }

        // set axis
        QListIterator<GenericAxisInfo>a(newPlots[i].axes);
        while(a.hasNext()) {
            GenericAxisInfo p=a.next();
            newPlots[i].plot->configureAxis(p.name, p.visible, p.align,p.minx, p.maxx, p.type, p.labelcolorstring, p.axiscolorstring, p.log, p.categories);
        }

        newPlots[i].plot->finaliseChart();
        newPlots[i].state = GenericPlotInfo::active;
    }

    // set current and wipe rest
    currentPlots=newPlots;

    // and zap the state
    newAxes.clear();
    newSeries.clear();

    // and display
    setUpdatesEnabled(true);
}
