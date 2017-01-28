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


#ifndef ONF_STEPFITANDFILTERCYLINDERSINSECTIONS_H
#define ONF_STEPFITANDFILTERCYLINDERSINSECTIONS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"

class CT_AbstractSingularItemDrawable;
class CT_Line;
class CT_LineData;
class CT_CircleData;
class CT_Circle;
class CT_Cylinder;
class CT_PointCluster;

class ONF_StepFitAndFilterCylindersInSections : public CT_AbstractStep
{
    struct StepFitAndFilterCylinderDebugInfo
    {
        QList<CT_AbstractSingularItemDrawable*>   refP;
        CT_LineData                 *lineData;
        CT_Line                     *line;
        CT_CircleData               *circleData;
        CT_Circle                   *circle;
        CT_Cylinder                 *cylinder;
        CT_PointCluster             *pC;
        DocumentInterface           *currentDocument;
    };

    // IMPORTANT in order to obtain step name
    Q_OBJECT

public:
    /*! \brief Step constructor
     *
     * Create a new instance of the step
     *
     * \param dataInit Step parameters object
     */
    ONF_StepFitAndFilterCylindersInSections(CT_StepInitializeData &dataInit);

    // CT_AbstractStep non obligatoire :
    void setDocuments(QList<DocumentInterface*> docList);

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

    QStringList getStepRISCitations() const;

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

    // CT_AbstractStep non obligatoire :
    void preWaitForAckIfInDebugMode();
    void postWaitForAckIfInDebugMode();

private:

    CT_AutoRenameModels     _outCylinderModelName;
    double                  _max_error;                 /*!< Erreur maximale sur l'ajustement de cylindres*/
    double                  _max_relative_error;        /*!< Erreur maximale sur l'ajustement de cylindres en % du diamètre*/
    double                  _min_radius;                /*!< Rayon minimal des cylindres ajustés*/
    double                  _max_radius;                /*!< Rayon maximal des cylindres ajustés*/
    bool                    _activeFiltering;           /*!< Faut-il filter les cylindres sur l'erreur ?*/
    bool                    _activeFilteringRelative;   /*!< Faut-il filter les cylindres sur l'erreur (erreur en %) ?*/
    bool                    _activeFilteringVerticality;/*!< Faut-il filter les cylindres sur la verticalité ?*/
    double                  _phi_max;                   /*!< Angle zénithal maximal*/

    StepFitAndFilterCylinderDebugInfo   _debugInfo;
};

#endif // ONF_STEPFITANDFILTERCYLINDERSINSECTIONS_H
