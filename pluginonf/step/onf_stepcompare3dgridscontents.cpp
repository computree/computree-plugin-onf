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

#include "onf_stepcompare3dgridscontents.h"

#include "ct_itemdrawable/ct_grid3d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/outModel/ct_outresultmodelgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

// Alias for indexing models
#define DEFin_rgrid1 "rgrid1"
#define DEFin_grp1 "grp1"
#define DEFin_grid1 "grid1"

#define DEFin_rgrid2 "rgrid2"
#define DEFin_grp2 "grp2"
#define DEFin_grid2 "grid2"

#define DEFout_rgrid "rgrid"
#define DEFout_grp "grp"
#define DEFout_grid "grid"


// Constructor : initialization of parameters
ONF_StepCompare3DGridsContents::ONF_StepCompare3DGridsContents(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _threshold = 1;
}

// Step description (tooltip of contextual menu)
QString ONF_StepCompare3DGridsContents::getStepDescription() const
{
    return tr("Comparer deux grilles 3D");
}

// Step detailled description
QString ONF_StepCompare3DGridsContents::getStepDetailledDescription() const
{
    return tr("Il est préférable que les grilles aient la même résolution et le même calage spatial.<br>"
              "Considérant A = Grille initiale (avant), B = Grille finale (après)"
              "En sortie l'étape renvoie une grille contenant :<br>"
              "* 00 : A =  NA,    B =  NA<br>"
              "* 01 : A =  NA,    B <  Seuil<br>"
              "* 02 : A =  NA,    B >= Seuil<br>"
              "* 10 : A <  Seuil, B =  NA<br>"
              "* 11 : A <  Seuil, B <  Seuil<br>"
              "* 12 : A <  Seuil, B >= Seuil<br>"
              "* 20 : A >= Seuil, B =  NA<br>"
              "* 21 : A >= Seuil, B <  Seuil<br>"
              "* 22 : A >= Seuil, B >= Seuil<br>");
}

// Step URL
QString ONF_StepCompare3DGridsContents::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCompare3DGridsContents::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCompare3DGridsContents(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCompare3DGridsContents::createInResultModelListProtected()
{
    CT_InResultModelGroup *resIn_rgrid1 = createNewInResultModel(DEFin_rgrid1, tr("Grille A (avant)"), "", true);
    resIn_rgrid1->setRootGroup(DEFin_grp1, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rgrid1->addItemModel(DEFin_grp1, DEFin_grid1, CT_AbstractGrid3D::staticGetType(), tr("Grille A (avant)"));

    CT_InResultModelGroup *resIn_rgrid2 = createNewInResultModel(DEFin_rgrid2, tr("Grille B (après)"), "", true);
    resIn_rgrid2->setRootGroup(DEFin_grp2, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_rgrid2->addItemModel(DEFin_grp2, DEFin_grid2,CT_AbstractGrid3D::staticGetType(), tr("Grille B (après)"));

}

// Creation and affiliation of OUT models
void ONF_StepCompare3DGridsContents::createOutResultModelListProtected()
{
    CT_OutResultModelGroup *res_rgrid = createNewOutResultModel(DEFout_rgrid, tr("Grille"));
    res_rgrid->setRootGroup(DEFout_grp, new CT_StandardItemGroup(), tr("Groupe"));
    res_rgrid->addItemModel(DEFout_grp, DEFout_grid, new CT_Grid3D<short>(), tr("Grille"));

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCompare3DGridsContents::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble("Valeur minimum pour considérer une case remplie", "", -1e+09, 1e+09, 4, _threshold, 1);
}

void ONF_StepCompare3DGridsContents::compute()
{

    QList<CT_ResultGroup*> inResultList = getInputResults();
    CT_ResultGroup* resIn_rgrid1 = inResultList.at(0);
    CT_ResultGroup* resIn_rgrid2 = inResultList.at(1);


    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res_rgrid = outResultList.at(0);


    // IN results browsing
    CT_ResultGroupIterator itIn_grp1(resIn_rgrid1, this, DEFin_grp1);
    if (itIn_grp1.hasNext())
    {
        const CT_AbstractItemGroup* grpIn_grp1 = (CT_AbstractItemGroup*) itIn_grp1.next();
        const CT_AbstractGrid3D* grid1 = (CT_AbstractGrid3D*)grpIn_grp1->firstItemByINModelName(this, DEFin_grid1);
        if (grid1 != NULL)
        {
            CT_ResultGroupIterator itIn_grp2(resIn_rgrid2, this, DEFin_grp2);
            if (itIn_grp2.hasNext())
            {
                const CT_AbstractItemGroup* grpIn_grp2 = (CT_AbstractItemGroup*) itIn_grp2.next();
                const CT_AbstractGrid3D* grid2 = (CT_AbstractGrid3D*)grpIn_grp2->firstItemByINModelName(this, DEFin_grid2);
                if (grid2 != NULL)
                {
                    double xmin = grid1->minX();
                    double ymin = grid1->minY();
                    double zmin = grid1->minZ();
                    double xmax = grid1->maxX();
                    double ymax = grid1->maxY();
                    double zmax = grid1->maxZ();

                    double res = grid1->resolution();

                    if (grid2->minX() < xmin) {xmin = grid2->minX();}
                    if (grid2->minY() < ymin) {ymin = grid2->minY();}
                    if (grid2->minZ() < zmin) {zmin = grid2->minZ();}
                    if (grid2->maxX() > xmax) {xmax = grid2->maxX();}
                    if (grid2->maxY() > ymax) {ymax = grid2->maxY();}
                    if (grid2->maxZ() > zmax) {zmax = grid2->maxZ();}

                    if (grid2->resolution() < res) {res = grid2->resolution();}

                    // Création de la grille de sortie
                    CT_StandardItemGroup* grp_grp= new CT_StandardItemGroup(DEFout_grp, res_rgrid);
                    res_rgrid->addGroup(grp_grp);

                    CT_Grid3D<short>* gridOut = CT_Grid3D<short>::createGrid3DFromXYZCoords(DEFout_grid, res_rgrid, xmin, ymin, zmin, xmax, ymax, zmax, res, -1, -1, false);
                    grp_grp->addItemDrawable(gridOut);

                    gridOut->setDefaultColor(QColor(255, 255, 255));
                    gridOut->addColorForValue(00, QColor(255, 125, 125));
                    gridOut->addColorForValue(01, QColor(125, 0, 0));
                    gridOut->addColorForValue(02, QColor(0, 0, 125));
                    gridOut->addColorForValue(10, QColor(255, 255, 125));
                    gridOut->addColorForValue(11, QColor(255, 255, 0));
                    gridOut->addColorForValue(12, QColor(0, 255, 0));
                    gridOut->addColorForValue(20, QColor(125, 125, 255));
                    gridOut->addColorForValue(21, QColor(255, 0, 0));
                    gridOut->addColorForValue(22, QColor(0, 0, 255));

                    // Remplissage de la grille de sortie
                    for (size_t n = 0 ; n < gridOut->nCells() ; n++)
                    {
                        Eigen::Vector3d center;
                        gridOut->getCellCenterCoordinates(n, center);

                        double val1, val2;
                        size_t index1, index2;

                        if (grid1->indexAtXYZ(center(0), center(1), center(2), index1))
                        {
                            val1 = grid1->valueAtIndexAsDouble(index1);
                        } else {
                            val1 = NAN;
                        }

                        if (grid2->indexAtXYZ(center(0), center(1), center(2), index2))
                        {
                            val2 = grid2->valueAtIndexAsDouble(index2);
                        } else {
                            val2 = NAN;
                        }

                        if (std::isnan(val1)) {
                            if (std::isnan(val2)) {
                                gridOut->setValueAtIndex(n, 00);
                            } else if (val2 < _threshold) {
                                gridOut->setValueAtIndex(n, 01);
                            } else {
                                gridOut->setValueAtIndex(n, 02);
                            }
                        } else if (val1 < _threshold) {
                            if (std::isnan(val2)) {
                                gridOut->setValueAtIndex(n, 10);
                            } else if (val2 < _threshold) {
                                gridOut->setValueAtIndex(n, 11);
                            } else {
                                gridOut->setValueAtIndex(n, 12);
                            }
                        } else {
                            if (std::isnan(val2)) {
                                gridOut->setValueAtIndex(n, 20);
                            } else if (val2 < _threshold) {
                                gridOut->setValueAtIndex(n, 21);
                            } else {
                                gridOut->setValueAtIndex(n, 22);
                            }
                        }
                    }
                    gridOut->computeMinMax();
                }
            }
        }
    }


}
