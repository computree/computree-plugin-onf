/****************************************************************************
 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                         All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Alexandre PIBOULE (ONF)

 This file is part of PluginONF library.

 PluginONF is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginONF is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginONF.  If not, see <http://www.gnu.org/licenses/lgpl.html>.
*****************************************************************************/

#ifndef ONF_STEPLOADPLOTAREAS_H
#define ONF_STEPLOADPLOTAREAS_H

#include "ct_step/abstract/ct_abstractstepcanbeaddedfirst.h"
#include "ct_view/tools/ct_textfileconfigurationdialog.h"
#include "ct_tools/model/ct_autorenamemodels.h"


class ONF_StepLoadPlotAreas: public CT_AbstractStepCanBeAddedFirst
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepLoadPlotAreas(CT_StepInitializeData &dataInit);

    /*! \brief Step description
     * 
     * Return a description of the step function
     */
    QString getStepDescription() const;

    /*! \brief Step detailled description
     * 
     * Return a detailled description of the step function
     */
    QString getStepDetailledDescription() const;

    /*! \brief Step URL
     * 
     * Return a URL of a wiki for this step
     */
    QString getStepURL() const;

    /*! \brief Step copy
     * 
     * Step copy, used when a step is added by step contextual menu
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

protected:

    /*! \brief Input results specification
     * 
     * Specification of input results models needed by the step (IN)
     */
    void createInResultModelListProtected();

    /*! \brief Parameters DialogBox
     * 
     * DialogBox asking for step parameters
     */
    void createPostConfigurationDialog();

    void createOutResultModelListProtected();

    void compute();


private:

    // Step parameters
    QList<CT_TextFileConfigurationFields> _neededFields;

    QString _refFileName;
    QString _plotID;
    bool _refHeader;
    QString _refSeparator;
    QString _refDecimal;
    QLocale _refLocale;
    int _refSkip;
    QMap<QString, int> _refColumns;

    CT_AutoRenameModels _areaModelName;
};

#endif // ONF_STEPLOADPLOTAREAS_H
