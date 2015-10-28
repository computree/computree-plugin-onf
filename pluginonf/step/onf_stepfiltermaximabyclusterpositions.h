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

#ifndef ONF_STEPFILTERMAXIMABYCLUSTERPOSITIONS_H
#define ONF_STEPFILTERMAXIMABYCLUSTERPOSITIONS_H

#ifdef USE_OPENCV
#include "ct_step/abstract/ct_abstractstep.h"

// Inclusion of auto-indexation system
#include "ct_tools/model/ct_autorenamemodels.h"


class ONF_StepFilterMaximaByClusterPositions: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepFilterMaximaByClusterPositions(CT_StepInitializeData &dataInit);

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

    /*! \brief Output results specification
     * 
     * Specification of output results models created by the step (OUT)
     */
    void createOutResultModelListProtected();

    /*! \brief Algorithm of the step
     * 
     * Step computation, using input results, and creating output results
     */
    void compute();

private:

    QStringList _fileNameSearchRadii;
    QStringList _fileNameExclusionRadii;

    // Declaration of autoRenames Variables (groups or items added to In models copies)
    CT_AutoRenameModels    _filteredMaxima_ModelName;
    CT_AutoRenameModels    _attMaximaItem_ModelName;
    CT_AutoRenameModels    _attMaxima_ModelName;
    CT_AutoRenameModels    _attClusterID_ModelName;

    double getRadius(double height, const QMap<double, double> &radii);


    void readRadii(QString fileName, QMap<double, double> &radii);
};
#endif

#endif // ONF_STEPFILTERMAXIMABYCLUSTERPOSITIONS_H
