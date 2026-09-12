#ifndef AIRCRAFTSTORE_H
#define AIRCRAFTSTORE_H

#include "model/aircrafttypes.h"

#include <QString>
#include <QVector>

class AircraftStore
{
public:
    QString rootDir() const;
    QString draftPath() const;
    QString baselinesDir() const;
    QString cpacsDir() const;
    QString cpacsPathForVersion(int version) const;
    QString casesDir() const;
    QString caseInputPath(int caseNumber) const;
    int nextCaseNumber() const;

    bool loadDraft(AircraftDocument *out, QString *errorMessage = nullptr) const;
    bool saveDraft(const AircraftDocument &doc, QString *errorMessage = nullptr) const;

    bool saveBaseline(const AircraftDocument &doc, QString *errorMessage = nullptr) const;
    bool loadBaseline(const QString &id, AircraftDocument *out, QString *errorMessage = nullptr) const;
    bool listBaselines(QVector<AircraftBaselineInfo> *out, QString *errorMessage = nullptr) const;

    bool readDocumentFile(const QString &path, AircraftDocument *out, QString *errorMessage = nullptr) const;
    bool writeDocumentFile(const QString &path, const AircraftDocument &doc, QString *errorMessage = nullptr) const;
};

#endif
