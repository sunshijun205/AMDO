#ifndef SRDCATALOGS_H
#define SRDCATALOGS_H

#include "model/srdtypes.h"

class SrdCatalogs
{
public:
    static QVector<SrdCatalogMetric> metrics();
    static bool findMetric(const QString &id, SrdCatalogMetric *out);
    static QVector<SrdCatalogClause> clauses();
    static bool findClause(const QString &id, SrdCatalogClause *out);
    static SrdDocument seedDocument();
};

#endif
