#ifndef ANALYSISSTORE_H
#define ANALYSISSTORE_H

#include "model/analysistypes.h"

#include <QString>
#include <QVector>

class AnalysisStore
{
public:
    QString rootDir() const;
    QString draftPath() const;
    QString baselinesDir() const;

    bool loadDraft(AnalysisDocument *out, QString *errorMessage = nullptr) const;
    bool saveDraft(const AnalysisDocument &doc, QString *errorMessage = nullptr) const;

    bool saveBaseline(const AnalysisDocument &doc, QString *errorMessage = nullptr) const;
    bool loadBaseline(const QString &id, AnalysisDocument *out, QString *errorMessage = nullptr) const;
    bool listBaselines(QVector<AnalysisBaselineInfo> *out, QString *errorMessage = nullptr) const;

    bool readDocumentFile(const QString &path, AnalysisDocument *out, QString *errorMessage = nullptr) const;
    bool writeDocumentFile(const QString &path, const AnalysisDocument &doc, QString *errorMessage = nullptr) const;
};

#endif
