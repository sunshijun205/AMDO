#ifndef SRDDERIVATIONSERVICE_H
#define SRDDERIVATIONSERVICE_H

#include "model/srdtypes.h"

class SrdDerivationService
{
public:
    QVector<SrdFlightCondition> suggestConditions(const SrdDocument &doc) const;
    QVector<SrdEnvelopePoint> suggestEnvelopePoints(const SrdDocument &doc) const;
    QVector<SrdRequirement> suggestRequirements(const SrdDocument &doc) const;
};

#endif
