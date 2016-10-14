#include "onf_stepcreatemaximacloud.h"

#ifdef USE_OPENCV

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_itemdrawable/tools/iterator/ct_itemiterator.h"
#include "ct_iterator/ct_mutablepointiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/ct_referencepoint.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/abstract/ct_abstractareashape2d.h"



// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_image "image"
#define DEFin_maxima "maxima"
#define DEFin_DTM "DTM"
#define DEF_SearchInArea   "emprise"

#define EPSILON 0.000001


// Constructor : initialization of parameters
ONF_StepCreateMaximaCloud::ONF_StepCreateMaximaCloud(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _createRefPoints = false;
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreateMaximaCloud::getStepDescription() const
{
    return tr("Créer un nuage de points de maxima");
}

// Step detailled description
QString ONF_StepCreateMaximaCloud::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreateMaximaCloud::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreateMaximaCloud::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreateMaximaCloud(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreateMaximaCloud::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Maxima"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_image, CT_Image2D<float>::staticGetType(), tr("Image (hauteurs)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_maxima, CT_Image2D<qint32>::staticGetType(), tr("Maxima"));
    resIn_res->addItemModel(DEFin_grp, DEFin_DTM, CT_Image2D<float>::staticGetType(), tr("MNT"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
    resIn_res->addItemModel(DEFin_grp, DEF_SearchInArea, CT_AbstractAreaShape2D::staticGetType(),
                              tr("Emprise"), "", CT_InAbstractModel::C_ChooseOneIfMultiple, CT_InAbstractModel::F_IsOptional);
}

// Creation and affiliation of OUT models
void ONF_StepCreateMaximaCloud::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_res = createNewOutResultModelToCopy(DEFin_res);
    if (resCpy_res != NULL)
    {
        if (_createRefPoints == 0)
        {
            resCpy_res->addItemModel(DEFin_grp, _maximaScene_ModelName, new CT_Scene(), tr("Maxima (points)"));
        } else {
            resCpy_res->addGroupModel(DEFin_grp, _grpRefPtsMaxima_ModelName);
            resCpy_res->addItemModel(_grpRefPtsMaxima_ModelName, _refPtsMaxima_ModelName, new CT_ReferencePoint(), tr("Maxima (points)"));
        }
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreateMaximaCloud::createPreConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPreConfigurationDialog();

    CT_ButtonGroup &bg_mode = configDialog->addButtonGroup(_createRefPoints);
    configDialog->addExcludeValue("", "", tr("Créer un nuage de points"), bg_mode, 0);
    configDialog->addExcludeValue("", "", tr("Créer des points de référence"), bg_mode, 1);
}

void ONF_StepCreateMaximaCloud::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        CT_Image2D<qint32>* maximaIn = (CT_Image2D<qint32>*)grp->firstItemByINModelName(this, DEFin_maxima);
        CT_Image2D<float>* imageIn = (CT_Image2D<float>*)grp->firstItemByINModelName(this, DEFin_image);
        CT_Image2D<float>* dtm = (CT_Image2D<float>*)grp->firstItemByINModelName(this, DEFin_DTM);
        const CT_AbstractAreaShape2D *emprise = (const CT_AbstractAreaShape2D*)grp->firstItemByINModelName(this, DEF_SearchInArea);

        if (dtm != NULL)
        {
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Un MNT a été founit, les valeurs Z des maxima seront corrigées")));
        } else {
            PS_LOG->addMessage(LogInterface::info, LogInterface::step, QString(tr("Aucun MNT n'a été founit, les valeurs Z des maxima NE seront PAS corrigées")));
        }

        if (maximaIn != NULL)
        {
            // Get maxima coordinates list
            QMultiMap<qint32, Eigen::Vector3d*> maximaCoords;

            for (size_t xx = 0 ; xx < maximaIn->colDim() ; xx++)
            {
                for (size_t yy = 0 ; yy < maximaIn->linDim() ; yy++)
                {
                    qint32 maximaID = maximaIn->value(xx, yy);

                    if (maximaID > 0 && maximaID != maximaIn->NA())
                    {
                        Eigen::Vector3d* coords = new Eigen::Vector3d();
                        if (maximaIn->getCellCenterCoordinates(xx, yy, *coords))
                        {
                            if (emprise == NULL || emprise->contains((*coords)(0), (*coords)(1)))
                            {
                                (*coords)(2) = imageIn->value(xx, yy);
                                maximaCoords.insert(maximaID, coords);
                            }
                        }
                    }
                }
            }

            setProgress(25);



            QList<qint32> maximaList = maximaCoords.uniqueKeys();
            int mxSize = maximaList.size();

            CT_NMPCIR pcir;
            CT_MutablePointIterator *it = NULL;
            CT_Point pReaded;

            if (_createRefPoints == 0)
            {
                pcir = PS_REPOSITORY->createNewPointCloud(mxSize);
                it = new CT_MutablePointIterator(pcir);
            }

            // Create maxima coords vector
            for (int i = 0 ; i < mxSize ; i++)
            {
                qint32 id = maximaList.at(i);

                QList<Eigen::Vector3d*> coordinates = maximaCoords.values(id);

                double x = 0;
                double y = 0;
                double z = -std::numeric_limits<double>::max();

                // Compute position of the current maxima if more than one pixel
                int size = coordinates.size();
                if (size > 0)
                {
                    for (int j = 0 ; j < size ; j++)
                    {
                        Eigen::Vector3d* pos = coordinates.at(j);
                        x += (*pos)(0);
                        y += (*pos)(1);
                        if ((*pos)(2) > z) {z = (*pos)(2);}
                    }

                    x /= size;
                    y /= size;

                    if (dtm != NULL)
                    {
                        float dtmVal = dtm->valueAtCoords(x, y);
                        if (dtmVal != dtm->NA())
                        {
                            z -= dtmVal;
                        }else
                        {
                            PS_LOG->addMessage(LogInterface::error, LogInterface::step, QString(tr("Valeur manquante dans le MNT pour un l'apex : x=%1 ; y=%2").arg(x).arg(y)));
                        }
                    }

                }

                if (_createRefPoints == 0)
                {
                    pReaded(CT_Point::X) = x;
                    pReaded(CT_Point::Y) = y;
                    pReaded(CT_Point::Z) = z;

                    it->next();
                    it->replaceCurrentPoint(pReaded);
                } else {
                    CT_ReferencePoint* refPoint = new CT_ReferencePoint(_refPtsMaxima_ModelName.completeName(), res, x, y, z, 0);
                    CT_StandardItemGroup* grpPt = new CT_StandardItemGroup(_grpRefPtsMaxima_ModelName.completeName(), res);
                    grpPt->addItemDrawable(refPoint);
                    grp->addGroup(grpPt);
                }
            }
            setProgress(60);

            if (_createRefPoints == 0)
            {
                CT_Scene* sceneMaxima = new CT_Scene(_maximaScene_ModelName.completeName(), res, pcir);
                sceneMaxima->updateBoundingBox();
                grp->addItemDrawable(sceneMaxima);
            }


            setProgress(80);

        }
    }
    setProgress(100);
}



#endif
