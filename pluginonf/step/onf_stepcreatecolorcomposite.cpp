#include "onf_stepcreatecolorcomposite.h"

#ifdef USE_OPENCV

#include "ct_itemdrawable/ct_image2d.h"
#include "ct_itemdrawable/ct_colorcomposite.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include <QDebug>

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_red "red"
#define DEFin_green "green"
#define DEFin_blue "blue"

#define DEFin_resZ "resZ"
#define DEFin_grpZ "grpZ"
#define DEFin_zvalue "zvalue"


// Constructor : initialization of parameters
ONF_StepCreateColorComposite::ONF_StepCreateColorComposite(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
}

// Step description (tooltip of contextual menu)
QString ONF_StepCreateColorComposite::getStepDescription() const
{
    return tr("Create color composite");
}

// Step detailled description
QString ONF_StepCreateColorComposite::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepCreateColorComposite::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepCreateColorComposite::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepCreateColorComposite(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepCreateColorComposite::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("2D Images"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resIn_res->addItemModel(DEFin_grp, DEFin_blue, CT_AbstractImage2D::staticGetType(), tr("Blue band"));
    resIn_res->addItemModel(DEFin_grp, DEFin_green, CT_AbstractImage2D::staticGetType(), tr("Green band"));
    resIn_res->addItemModel(DEFin_grp, DEFin_red, CT_AbstractImage2D::staticGetType(), tr("Red band"));

    CT_InResultModelGroup *resIn_zval = createNewInResultModel(DEFin_resZ, tr("DSM"));
    resIn_zval->setZeroOrMoreRootGroup();
    resIn_zval->addGroupModel("", DEFin_grpZ, CT_AbstractItemGroup::staticGetType(), tr("Group"));
    resIn_zval->addItemModel(DEFin_grpZ, DEFin_zvalue, CT_Image2D<float>::staticGetType(), tr("Z raster"));
    resIn_zval->setMinimumNumberOfPossibilityThatMustBeSelectedForOneTurn(0);
}

// Creation and affiliation of OUT models
void ONF_StepCreateColorComposite::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_res = createNewOutResultModelToCopy(DEFin_res);
    if (resCpy_res != NULL)
    {
        resCpy_res->addItemModel(DEFin_grp, _colorcomposite_ModelName, new CT_ColorComposite(), tr("Color Composite"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepCreateColorComposite::createPostConfigurationDialog()
{
//    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
}

void ONF_StepCreateColorComposite::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    CT_Image2D<float>* zvalue = NULL;

    if (getInputResults().size() > 1)
    {
        CT_ResultGroup* res_zVal = getInputResults().at(1);

        CT_ResultGroupIterator itCpy_grpZ(res_zVal, this, DEFin_grpZ);
        if (itCpy_grpZ.hasNext() && !isStopped())
        {
            CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grpZ.next();

            zvalue = (CT_Image2D<float>*)grp->firstItemByINModelName(this, DEFin_zvalue);
        }
    }

    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        
        CT_AbstractImage2D* red   = (CT_AbstractImage2D*)grp->firstItemByINModelName(this, DEFin_red);
        CT_AbstractImage2D* green = (CT_AbstractImage2D*)grp->firstItemByINModelName(this, DEFin_green);
        CT_AbstractImage2D* blue  = (CT_AbstractImage2D*)grp->firstItemByINModelName(this, DEFin_blue);

        if (red != NULL && green != NULL && blue != NULL)
        {
            CT_ColorComposite* colorComposite = new CT_ColorComposite(_colorcomposite_ModelName.completeName(), res, red, green, blue, zvalue);
            grp->addItemDrawable(colorComposite);
        }
    }
}

#endif
