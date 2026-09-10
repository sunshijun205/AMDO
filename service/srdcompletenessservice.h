#ifndef SRDCOMPLETENESSSERVICE_H
#define SRDCOMPLETENESSSERVICE_H

#include "model/srdtypes.h"

class SrdCompletenessService
{
public:
    SrdCompletenessReport evaluate(const SrdDocument &doc) const;
};

#endif
