#include "onf_citations.h"

ONF_citations ONF_citations::_citationInstance = ONF_citations();

ONF_citations::ONF_citations()
{
    _citationOthmaniEtAl2001 = "TY  - CONF\n"
                               "T1  - Towards automated and operational forest inventories with T-Lidar\n"
                               "A1  - Othmani, Ahlem\n"
                               "A1  - Piboule, Alexandre\n"
                               "A1  - Krebs, Michael\n"
                               "A1  - Stolz, Christophe\n"
                               "A1  - Voon, LFC Lew Yan\n"
                               "JO  - 11th International Conference on LiDAR Applications for Assessing Forest Ecosystems (SilviLaser 2011)\n"
                               "Y1  - 2011\n"
                               "ER  - \n";
}

ONF_citations::~ONF_citations()
{

}

ONF_citations& ONF_citations::citation()
{
    return ONF_citations::_citationInstance;
}
