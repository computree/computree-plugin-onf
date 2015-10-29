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

#ifndef ONF_STEPCOMPUTECROWNPROJECTION_H
#define ONF_STEPCOMPUTECROWNPROJECTION_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

// Inclusion of auto-indexation system
#include "ct_tools/model/ct_autorenamemodels.h"



class ONF_StepComputeCrownProjection: public CT_AbstractStep
{
    Q_OBJECT

    struct level
    {
    public:
        level(double zmin, double zmax, double zlevel)
        {
            _zmin = zmin;
            _zmax = zmax;
            _zlevel = zlevel;
        }

        double  _zmin;
        double  _zmax;
        double  _zlevel;
        QList<Eigen::Vector2d*> _pointList;
    };

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepComputeCrownProjection(CT_StepInitializeData &dataInit);

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

    // Declaration of autoRenames Variables (groups or items added to In models copies)
    CT_AutoRenameModels    _convexHull_ModelName;
    CT_AutoRenameModels    _grpSlice_ModelName;
    CT_AutoRenameModels    _scliceCvx_ModelName;
    CT_AutoRenameModels    _directionalHull_ModelName;
    CT_AutoRenameModels    _scliceDir_ModelName;

    CT_ResultGroup*        _rscene;

    // Step parameters
    bool        _computeSlices;
    double       _spacing;
    double      _thickness;
    bool        _computeDirs;
    int         _nbDir;

    double    _zmin;
    double    _zmax;

    void computeConvexHullForOneSceneGroup(CT_StandardItemGroup *group) const;
};

#endif // ONF_STEPCOMPUTECROWNPROJECTION_H
