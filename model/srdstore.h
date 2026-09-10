#ifndef SRDSTORE_H
#define SRDSTORE_H

#include "model/srdtypes.h"

#include <QString>
#include <QVector>

class SrdStore
{
public:
    QString rootDir() const;
    QString draftPath() const;
    QString baselinesDir() const;

    bool loadDraft(SrdDocument *out, QString *errorMessage = nullptr) const;
    bool saveDraft(const SrdDocument &doc, QString *errorMessage = nullptr) const;

    bool saveBaseline(const SrdDocument &doc, QString *errorMessage = nullptr) const;
    bool loadBaseline(const QString &id, SrdDocument *out, QString *errorMessage = nullptr) const;
    bool listBaselines(QVector<SrdBaselineInfo> *out, QString *errorMessage = nullptr) const;

    bool readDocumentFile(const QString &path, SrdDocument *out, QString *errorMessage = nullptr) const;
    bool writeDocumentFile(const QString &path, const SrdDocument &doc, QString *errorMessage = nullptr) const;
};

#endif
