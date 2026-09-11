#ifndef AIRCRAFTIMPORTEXPORTSERVICE_H
#define AIRCRAFTIMPORTEXPORTSERVICE_H

#include "model/aircrafttypes.h"

#include <QString>

class AircraftStore;

class AircraftImportExportService
{
public:
    explicit AircraftImportExportService(AircraftStore *store);

    bool importAircraftJson(const QString &path, AircraftDocument *out, QString *errorMessage = nullptr);
    bool exportAircraftJson(const QString &path, const AircraftDocument &doc, QString *errorMessage = nullptr);
    QString defaultExportPath(const AircraftDocument &doc) const;

private:
    AircraftStore *m_store;
};

#endif
