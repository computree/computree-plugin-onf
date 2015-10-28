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

#ifndef ONF_STEPMERGECLUSTERSFROMPOSITIONS_H
#define ONF_STEPMERGECLUSTERSFROMPOSITIONS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_itemdrawable/ct_point2d.h"
#include "ct_itemdrawable/ct_pointcluster.h"
#include "ct_tools/model/ct_autorenamemodels.h"

class CT_AbstractItemGroup;

/*!
 * \class ONF_StepMergeClustersFromPositions
 * \ingroup Steps_PB
 * \brief <b>Regroupe des clusters à partir de positions 2D.</b>
 *
 * Cette étape permet de générer une scène pour chaque positions 2D fournie.
Chaque cluster d'entrée est affilié à la position la plus proche (en 2D XY).

Ensuite une action interactive permet de corriger cette attribution automatique.
 *
 * \param _interactiveMode 
 *
 */

class ONF_StepMergeClustersFromPositions: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepMergeClustersFromPositions(CT_StepInitializeData &dataInit);

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

//    void preProcessCreateOutResultModelListProtected();


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

    void initManualMode();
    void useManualMode(bool quit);
private:

    CT_AutoRenameModels     _outSceneModelName;
//    QString                 _outclustergroupname;

    QMap<CT_PointCluster*, CT_AbstractItemGroup*> _clustersGroups;


    // Step parameters
    bool    _interactiveMode;
    QList<CT_AbstractItemDrawable*>                             m_itemDrawableSelected;
    DocumentInterface                                           *m_doc;
    int                                                         m_status;

    QMap<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > _positionsData;

    static void addPointsToScenes(QPair<CT_PointCloudIndexVector *, QList<const CT_PointCluster *> *> &pair);
};

#endif // ONF_STEPMERGECLUSTERSFROMPOSITIONS_H
