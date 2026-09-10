#ifndef SRDIMPORTEXPORTSERVICE_H
#define SRDIMPORTEXPORTSERVICE_H

#include "model/srdtypes.h"

#include <QString>

class SrdStore;

class SrdImportExportService
{
public:
    explicit SrdImportExportService(SrdStore *store);

    bool importSrdJson(const QString &path, SrdDocument *out, QString *errorMessage = nullptr);
    bool importNeedsCsv(const QString &path, QVector<SrdPerformanceNeed> *out, QString *errorMessage = nullptr);
    bool exportEvaluationSpec(const QString &path, const SrdDocument &doc, QString *errorMessage = nullptr);
    QString defaultExportPath(const SrdDocument &doc) const;

private:
    SrdStore *m_store;
};

#endif
