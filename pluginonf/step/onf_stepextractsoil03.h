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


#ifndef ONF_STEPEXTRACTSOIL03_H
#define ONF_STEPEXTRACTSOIL03_H

#include "ct_step/abstract/ct_abstractstep.h"

/*!
 * \class ONF_StepExtractSoil03
 * \ingroup Steps_OE
 * \brief <b>Split vegetation and soil points and generate a DTM</b>
 *
 * This step first create a Zmin grid of a resolution specified by _gridsize.
 * All points beetween zMin of the cell, and zMin + _soilwidth are considered as soil points, to compute soil points density.
 * In this grid each cell with a density < _min_density is set to NA value.
 * A consistency check is done with neighbors cells in _dist neighborhood. Not coherent cell are affected to NA value.
 * So a DTM with NA values is computed.
 * If _interpol is true, NA values are interpolated using not NA Delaunay triangulation neighboors values (weighted by 1/distance).
 * If _smooth is true, values ares smooth by neighborhood meaning _smoothDist surrounding cells.
 *
 * \param _min_density Minimum value of point density to be accepted as soil cell (pts/m²)
 * \param _gridsize DTM grid resolution (in m)
 * \param _soilwidth Soil width (in m)
 * \param _dist Distance in cells to test neighborhood consistency
 * \param _interpol If true, DTM is interpolated
 * \param _smooth If true, DTM is smoothed
 * \param _smoothDist Distance in cells for smoothing
 *
 *
 * <b>Input Models:</b>
 *
 *  - CT_ResultGroup \n
 *      (...)
 *      - CT_StandardItemGroup \n
 *          - CT_Scene(Scene) \n
 *
 * <b>Output Models:</b>
 *
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Scene(Vegetation points) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Scene(Soil points) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Triangulation2D(2D triangulatin object) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Grid2DXY<Float>(DTM) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Grid2DXY<Float>(DSM = zMax grid) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Grid2DXY<Float>(Canopy Height Model) \n
 *  - CT_ResultGroup \n
 *      - CT_StandardItemGroup \n
 *          - CT_Grid2DXY<Int>(Point Density raster) \n
 */
class ONF_StepExtractSoil03 : public CT_AbstractStep
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
    ONF_StepExtractSoil03(CT_StepInitializeData &dataInit);

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

    double  _min_density;                       /*!< Densite minimum pour considerer que c'est du sol*/
    double  _gridsize;                          /*!< Taille de  la grille MNT en m*/
    double  _soilwidth;                         /*!< Epaisseur du sol en m*/
    double  _dist;                              /*!< Distance en case pour la recherche des points isoles*/
    bool    _interpol;                          /*!< Faut-il interpoler ?*/
    bool    _smooth;                            /*!< Faut-il lisser ?*/
    double  _smoothDist;                              /*!< Distance en case pour lle lissage*/

};

#endif // ONF_STEPEXTRACTSOIL03_H
