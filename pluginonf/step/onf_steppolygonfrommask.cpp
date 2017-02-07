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

#include "onf_steppolygonfrommask.h"

#ifdef USE_OPENCV


#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

#include "ct_itemdrawable/ct_image2d.h"
#include "ct_itemdrawable/ct_polygon2d.h"

#include "ct_result/ct_resultgroup.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/core/core.hpp"


#define DEF_SearchInResult      "r"
#define DEF_SearchInGroup       "g"
#define DEF_SearchInMask      "m"
#define DEF_SearchOutResult     "r"

ONF_StepPolygonFromMask::ONF_StepPolygonFromMask(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

QString ONF_StepPolygonFromMask::getStepDescription() const
{
    return tr("Création de polygones à partir de masques");
}

QString ONF_StepPolygonFromMask::getStepDetailledDescription() const
{
    return tr("");
}

CT_VirtualAbstractStep* ONF_StepPolygonFromMask::createNewInstance(CT_StepInitializeData &dataInit)
{
    // cree une copie de cette etape
    return new ONF_StepPolygonFromMask(dataInit);
}

//////////////////// PROTECTED //////////////////

void ONF_StepPolygonFromMask::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultModel = createNewInResultModelForCopy(DEF_SearchInResult, tr("Dalles"));
    resultModel->setZeroOrMoreRootGroup();
    resultModel->addGroupModel("", DEF_SearchInGroup, CT_AbstractItemGroup::staticGetType(), tr("Grp"));
    resultModel->addItemModel(DEF_SearchInGroup, DEF_SearchInMask, CT_Image2D<uchar>::staticGetType(), tr("Masque"));
}

void ONF_StepPolygonFromMask::createPostConfigurationDialog()
{
    //CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepPolygonFromMask::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEF_SearchInResult);

    if(res != NULL) {
        res->addGroupModel(DEF_SearchInGroup, _outPolygonGrpModelName);
        res->addItemModel(_outPolygonGrpModelName, _outPolygonModelName, new CT_Polygon2D(), tr("Polygone"));
    }
}

void ONF_StepPolygonFromMask::compute()
{
    CT_ResultGroup *outRes = getOutResultList().first();

    CT_ResultGroupIterator it(outRes, this, DEF_SearchInGroup);
    while (it.hasNext() && (!isStopped()))
    {
        CT_StandardItemGroup *group = (CT_StandardItemGroup*) it.next();
        CT_Image2D<uchar> *mask = (CT_Image2D<uchar>*)group->firstItemByINModelName(this, DEF_SearchInMask);

        if(mask != NULL)
        {
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(mask->getMat(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

            for (int i = 0 ; i < contours.size() ; i++)
            {
                const std::vector<cv::Point> &contour = contours.at(i);

                QVector<Eigen::Vector2d *> vertices;

                double demiRes = mask->resolution() / 2.0;

                int xxLast = 0;
                int yyLast = 0;

                if (contour.size() > 0)
                {
                    const cv::Point &vert = contour.at(contour.size() - 1);
                    xxLast = vert.x;
                    yyLast = vert.y;
                }

                for (int j = 0 ; j < contour.size() ; j++)
                {
                    const cv::Point &vert = contour.at(j);
                    int xx = vert.x;
                    int yy = vert.y;

                    double x = mask->getCellCenterColCoord(xx);
                    double y = mask->getCellCenterLinCoord(yy);

                    bool sides[4];
                    sides[0] = (mask->value(xx+1, yy) == false);
                    sides[1] = (mask->value(xx, yy-1) == false);
                    sides[2] = (mask->value(xx-1, yy) == false);
                    sides[3] = (mask->value(xx, yy+1) == false);

                    if (sides[0] && sides[2] && !sides[1] && !sides[3])
                    {
                        if (yy < yyLast)
                        {
                            vertices.append(new Eigen::Vector2d(x + demiRes, y + demiRes));
                        } else {
                            vertices.append(new Eigen::Vector2d(x - demiRes, y - demiRes));
                        }
                    } else if (sides[1] && sides[3] && !sides[0] && !sides[2])
                    {
                        if (xx < xxLast)
                        {
                            vertices.append(new Eigen::Vector2d(x - demiRes, y + demiRes));
                        } else {
                            vertices.append(new Eigen::Vector2d(x + demiRes, y - demiRes));
                        }
                    } else if (sides[0] && sides[1] && sides[2] && sides[3])
                    {
                        vertices.append(new Eigen::Vector2d(x + demiRes, y + demiRes));
                        vertices.append(new Eigen::Vector2d(x - demiRes, y + demiRes));
                        vertices.append(new Eigen::Vector2d(x - demiRes, y - demiRes));
                        vertices.append(new Eigen::Vector2d(x + demiRes, y - demiRes));
                    } else if (sides[0] && !sides[3])
                    {
                        vertices.append(new Eigen::Vector2d(x + demiRes, y + demiRes));
                        if (sides[1])
                        {
                            vertices.append(new Eigen::Vector2d(x - demiRes, y + demiRes));
                            if (sides[2])
                            {
                                vertices.append(new Eigen::Vector2d(x - demiRes, y - demiRes));
                            }
                        }
                    } else if (sides[1] && !sides[0])
                    {
                        vertices.append(new Eigen::Vector2d(x - demiRes, y + demiRes));
                        if (sides[2])
                        {
                            vertices.append(new Eigen::Vector2d(x - demiRes, y - demiRes));
                            if (sides[3])
                            {
                                vertices.append(new Eigen::Vector2d(x + demiRes, y - demiRes));
                            }
                        }
                    } else if (sides[2] && !sides[1])
                    {
                        vertices.append(new Eigen::Vector2d(x - demiRes, y - demiRes));
                        if (sides[3])
                        {
                            vertices.append(new Eigen::Vector2d(x + demiRes, y - demiRes));
                            if (sides[0])
                            {
                                vertices.append(new Eigen::Vector2d(x + demiRes, y + demiRes));
                            }
                        }
                    } else if (sides[3] && !sides[2])
                    {
                        vertices.append(new Eigen::Vector2d(x + demiRes, y - demiRes));
                        if (sides[0])
                        {
                            vertices.append(new Eigen::Vector2d(x + demiRes, y + demiRes));
                            if (sides[1])
                            {
                                vertices.append(new Eigen::Vector2d(x - demiRes, y + demiRes));
                            }
                        }
                    }
                    xxLast = xx;
                    yyLast = yy;
                }

                if (vertices.size() > 0)
                {
                    CT_Polygon2DData* dataPoly = new CT_Polygon2DData(vertices, false);
                    CT_Polygon2D* poly = new CT_Polygon2D(_outPolygonModelName.completeName(), outRes, dataPoly);
                    CT_StandardItemGroup* outGrpPoly = new CT_StandardItemGroup(_outPolygonGrpModelName.completeName(), outRes);
                    outGrpPoly->addItemDrawable(poly);
                    group->addGroup(outGrpPoly);
                }
            }
        }
    }
}


#endif
