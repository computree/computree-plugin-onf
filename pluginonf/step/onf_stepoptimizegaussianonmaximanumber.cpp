#include "onf_stepoptimizegaussianonmaximanumber.h"

#ifdef USE_OPENCV

#include "ct_itemdrawable/ct_image2d.h"
#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/types_c.h"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_image "image"



// Constructor : initialization of parameters
ONF_StepOptimizeGaussianOnMaximaNumber::ONF_StepOptimizeGaussianOnMaximaNumber(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _sigmaStep = 0.10;
    _sigmaMax = 2.0;
    _minHeight = 2.0;

}

// Step description (tooltip of contextual menu)
QString ONF_StepOptimizeGaussianOnMaximaNumber::getStepDescription() const
{
    return tr("Filtre Gaussien optimisé par le nombre de maxima");
}

// Step detailled description
QString ONF_StepOptimizeGaussianOnMaximaNumber::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepOptimizeGaussianOnMaximaNumber::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepOptimizeGaussianOnMaximaNumber::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepOptimizeGaussianOnMaximaNumber(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepOptimizeGaussianOnMaximaNumber::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Image 2D"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_image, CT_Image2D<float>::staticGetType(), tr("Image"));

}

// Creation and affiliation of OUT models
void ONF_StepOptimizeGaussianOnMaximaNumber::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_res = createNewOutResultModelToCopy(DEFin_res);
    if (resCpy_res != NULL)
    {
        resCpy_res->addItemModel(DEFin_grp, _filteredImage_ModelName, new CT_Image2D<float>(), tr("Image filtrée"));
        resCpy_res->addItemModel(DEFin_grp, _maximaImage_ModelName,  new CT_Image2D<qint32>(), tr("Maxima"));
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepOptimizeGaussianOnMaximaNumber::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();

    configDialog->addDouble(tr("Sigma max"), tr("en mètres"), 0, 1000, 2, _sigmaMax, 1);
    configDialog->addDouble(tr("Incrément de Sigma par étape"), tr("en mètres"), 0, 1000, 2, _sigmaStep, 1);
    configDialog->addDouble(tr("Ne pas détécter de maxima en dessous de"), tr("m"), 0, 99999, 2, _minHeight);
    configDialog->addEmpty();
    configDialog->addTitle(tr("N.B. : Portée du filtre = 7.7 x Sigma (en mètres)"));
}

void ONF_StepOptimizeGaussianOnMaximaNumber::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        
        CT_Image2D<float>* imageIn = (CT_Image2D<float>*)grp->firstItemByINModelName(this, DEFin_image);
        if (imageIn != NULL)
        {
            Eigen::Vector2d min;
            imageIn->getMinCoordinates(min);

            CT_Image2D<float>* filteredImage = new CT_Image2D<float>(_filteredImage_ModelName.completeName(), res, min(0), min(1), imageIn->colDim(), imageIn->linDim(), imageIn->resolution(), imageIn->level(), imageIn->NA(), imageIn->NA());
            grp->addItemDrawable(filteredImage);

            CT_Image2D<qint32>* maximaImage = new CT_Image2D<qint32>(_maximaImage_ModelName.completeName(), res, min(0), min(1), imageIn->colDim(), imageIn->linDim(), imageIn->resolution(), imageIn->level(), imageIn->NA(), 0);
            grp->addItemDrawable(maximaImage);

            cv::Mat_<float>& filteredMat = filteredImage->getMat();


            for (double sigmaInMeters = 0 ; sigmaInMeters < _sigmaMax ; sigmaInMeters += _sigmaStep)
            {
                qDebug() << "01";
                double sigma = sigmaInMeters / imageIn->resolution();
                cv::GaussianBlur(imageIn->getMat(), filteredMat, cv::Size2d(0, 0), sigma);
                qDebug() << "02";

                cv::Mat_<float> dilatedMat(filteredMat.rows, filteredMat.cols);
                qDebug() << "03";
                cv::Mat maximaMat = cv::Mat::zeros(filteredMat.rows, filteredMat.cols, CV_32F); // Nécessaire car compare ne prend pas la version template Mat_<Tp> en output !!!
                qDebug() << "04";

                // Détéction des maxima
                cv::dilate(filteredMat, dilatedMat, cv::getStructuringElement(cv::MORPH_RECT, cv::Size2d(3,3)));
                qDebug() << "05";
                cv::compare(filteredMat, dilatedMat, maximaMat, cv::CMP_EQ);
                qDebug() << "06";


                // numérotation des maxima
                cv::Mat labels = cv::Mat::zeros(filteredMat.rows, filteredMat.cols, CV_32S); // Nécessaire car compare ne prend pas la version template Mat_<Tp> en output !!!
                cv::connectedComponents(maximaMat, labels);
                qDebug() << "07";

                QList<qint32> maximaIds;
                cv::Mat_<qint32> labs = labels;
                for (int col = 0 ; col < maximaMat.cols ; col++)
                {
                    for (int lin = 0 ; lin < maximaMat.rows ; lin++)
                    {
                        if (filteredMat(lin, col) < _minHeight)
                        {
                            labs(lin, col) = 0;
                        }

                        qint32 maxId = labs(lin, col);
                        if (maxId != 0 && !maximaIds.contains(maxId)) {maximaIds.append(maxId);}
                    }
                }
                qDebug() << "08";

                maximaImage->getMat() = labels;
                qDebug() << "09";

                int maxNumber = maximaIds.size();
                qDebug() << "Sigma=" << sigmaInMeters << "   maxNum=" << maxNumber;
            }

            filteredImage->computeMinMax();
            maximaImage->computeMinMax();
        }
    }
}

#endif
