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


#ifndef ONF_STEPEXTRACTDIAMETERSFROMCYLINDERS_H
#define ONF_STEPEXTRACTDIAMETERSFROMCYLINDERS_H

#include "ct_step/abstract/ct_abstractstep.h"
#include "ct_tools/model/ct_autorenamemodels.h"

/*!
 * \class ONF_StepExtractDiametersFromCylinders
 * \ingroup Steps_OE
 * \brief <b>Adjust a Diameter for each section, based on fitted cylinders.</b>
 *
 * \param _h Height od the diameter estimation
 * \param _hmin Minimum height of considered cylinders
 * \param _hmax Maximum height of considered cylinders
 * \param _deltaDMax Maximum metric decreasing in diameter
 * \param _minCylinderNumber Minimum number of cylinder to consider
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - CT_StandardItemGroup (Section)... \n
 *          - CT_ReferencePoint (DTM coordinate) \n
 *          - CT_StandardItemGroup (Cylinder group)... \n
 *              - CT_Cylinder (Cylinder) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *  *      (...)
 *      - CT_StandardItemGroup (Section)... \n
 *          - <em>cpy CT_ReferencePoint (DTM coordinate)</em> \n
 *          - <b>CT_Circle (Diameter)</b> \n
 *          - CT_StandardItemGroup (Cylinder group)... \n
 *              - <em>cpy CT_Cylinder (Cylinder)</em> \n
 *
 */
class ONF_StepExtractDiametersFromCylinders : public CT_AbstractStep
{
    // IMPORTANT in order to obtain step name
    Q_OBJECT

public:

    /*! \brief Step constructor
     *
     * Create a new instance of the step
     *
     * \param dataInit Step parameters object
     */
    ONF_StepExtractDiametersFromCylinders(CT_StepInitializeData &dataInit);

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

    CT_AutoRenameModels     _outCircleModelName;


    double _h;                            /*!< Hauteur d'estimation du diamètre*/
    double _hmin;                         /*!< Hauteur plancher de la zone d'estimation*/
    double _hmax     ;                    /*!< Hauteur plafond de la zone d'estimation*/
    double _deltaDMax     ;               /*!< Decroissance métrique maximale*/
    int    _minCylinderNumber;            /*!< Nombre de cylindres minimum pour ajuster un diamètre*/

};

#endif // ONF_STEPEXTRACTDIAMETERSFROMCYLINDERS_H
