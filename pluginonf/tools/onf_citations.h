#ifndef ONF_CITATIONS_H
#define ONF_CITATIONS_H

#include <QString>

class ONF_citations
{
public:
    static ONF_citations& citation();

    QString _citationOthmaniEtAl2001;

private:
    static ONF_citations   _citationInstance;

    ONF_citations();
    ~ONF_citations();
    ONF_citations& operator= (const ONF_citations&){}
};

#endif // ONF_CITATIONS_H
