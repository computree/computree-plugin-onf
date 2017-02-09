#include "onf_stepcomputepointheightattribute.h"

#ifdef USE_OPENCV

// Utilise le depot
#include "ct_global/ct_context.h"

// Utilise les attributs optionnels
#include "ct_itemdrawable/ct_pointsattributesscalartemplated.h"

#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/ct_outresultmodelgroupcopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"

// Inclusion of standard result class
#include "ct_result/ct_resultgroup.h"

// Inclusion of used ItemDrawable classes
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_iterator/ct_pointiterator.h"
#include "ct_iterator/ct_groupiterator.h"

// Alias for indexing in models
#define DEFin_resSc "resSc"
#define DEFin_scGrp "scGrp"
#define DEFin_sc "sc"

#define DEFin_resDTM "resDTM"
#define DEFin_DTMGrp "DTMGrp"
#define DEFin_DTM "DTM"


// Constructor : initialization of parameters
ONF_StepComputePointHeightAttribute::ONF_StepComputePointHeightAttribute(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepComputePointHeightAttribute::getStepDescription() const
{
    return tr("Calculer la hauteur des points");
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepComputePointHeightAttribute::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepComputePointHeightAttribute(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepComputePointHeightAttribute::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy * resultSc = createNewInResultModelForCopy(DEFin_resSc, tr("Scene(s)"));
    resultSc->setZeroOrMoreRootGroup();
    resultSc->addGroupModel("", DEFin_scGrp, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resultSc->addItemModel(DEFin_scGrp, DEFin_sc, CT_AbstractItemDrawableWithPointCloud::staticGetType(), tr("Scene(s)"));

    CT_InResultModelGroup *resultDTM = createNewInResultModel(DEFin_resDTM, tr("MNT"));
    resultDTM->setZeroOrMoreRootGroup();
    resultDTM->addGroupModel("", DEFin_DTMGrp, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resultDTM->addItemModel(DEFin_DTMGrp, DEFin_DTM, CT_Image2D<float>::staticGetType(), tr("MNT"));
}

// Creation and affiliation of OUT models
void ONF_StepComputePointHeightAttribute::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *res = createNewOutResultModelToCopy(DEFin_resSc);

    if(res != NULL)
    {
        res->addItemModel(DEFin_scGrp, _outHeightAttributeModelName, new CT_PointsAttributesScalarTemplated<float>(), tr("Height"));
    }

}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepComputePointHeightAttribute::createPostConfigurationDialog()
{
}

void ONF_StepComputePointHeightAttribute::compute()
{
    CT_ResultGroup* resin_DTM = getInputResults().at(1);

    CT_Image2D<float>* dtm = NULL;
    CT_ResultItemIterator it(resin_DTM, this, DEFin_DTM);
    if (it.hasNext())
    {
        dtm = (CT_Image2D<float>*) it.next();
    }

    if (dtm != 0)
    {
        CT_ResultGroup* resOut = getOutResultList().first();

        CT_ResultGroupIterator itSc(resOut, this, DEFin_scGrp);
        while (itSc.hasNext())
        {
            CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itSc.next();

            if (grp != NULL)
            {
                CT_AbstractItemDrawableWithPointCloud* scene = (CT_AbstractItemDrawableWithPointCloud*) grp->firstItemByINModelName(this, DEFin_sc);

                if (scene != NULL && !isStopped())
                {
                    const CT_AbstractPointCloudIndex *pointCloudIndex = scene->getPointCloudIndex();
                    size_t n_points = pointCloudIndex->size();

                    CT_PointIterator itP(pointCloudIndex);

                    // On declare un tableau d'attributs double que l'on va remplir avec les coordonnées correspondant a l'axe demandé
                    CT_StandardCloudStdVectorT<float> *attribute = new CT_StandardCloudStdVectorT<float>();

                    float minAttribute = std::numeric_limits<float>::max();
                    float maxAttribute = -std::numeric_limits<float>::max();

                    size_t i = 0;
                    // On applique la translation a tous les points du nuage
                    while (itP.hasNext()&& !isStopped())
                    {
                        const CT_Point& point = itP.next().currentPoint();
                        float h = (float)point(2) - dtm->valueAtCoords(point(0), point(1));

                        attribute->addT(h);

                        if (h < minAttribute) {minAttribute = h;}
                        if (h > maxAttribute) {maxAttribute = h;}

                        setProgress( 100.0*i++ / n_points);
                    }

                    if (i > 0)
                    {
                        CT_PointsAttributesScalarTemplated<float>*  itemOut_attribute  = new CT_PointsAttributesScalarTemplated<float>(_outHeightAttributeModelName.completeName(),
                                                                                                                                       resOut,
                                                                                                                                       scene->getPointCloudIndexRegistered(),
                                                                                                                                       attribute,
                                                                                                                                       minAttribute,
                                                                                                                                       maxAttribute);
                        grp->addItemDrawable(itemOut_attribute);
                    }
                }
            }
        }
    }

}

#endif
