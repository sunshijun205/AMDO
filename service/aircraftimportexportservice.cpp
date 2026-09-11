#include "service/aircraftimportexportservice.h"

#include "model/aircraftstore.h"

AircraftImportExportService::AircraftImportExportService(AircraftStore *store)
    : m_store(store)
{
}

bool AircraftImportExportService::importAircraftJson(const QString &path, AircraftDocument *out,
                                                     QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入失败：存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->readDocumentFile(path, out, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导入方案失败：%1").arg(detail);
        return false;
    }
    return true;
}

bool AircraftImportExportService::exportAircraftJson(const QString &path, const AircraftDocument &doc,
                                                     QString *errorMessage)
{
    if (!m_store) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导出失败：存储未初始化");
        return false;
    }
    QString detail;
    if (!m_store->writeDocumentFile(path, doc, &detail)) {
        if (errorMessage)
            *errorMessage = QString::fromUtf8("导出方案失败：%1").arg(detail);
        return false;
    }
    return true;
}

QString AircraftImportExportService::defaultExportPath(const AircraftDocument &doc) const
{
    QString name = doc.id.isEmpty() ? QStringLiteral("aircraft-draft") : doc.id;
    name.replace(QLatin1Char('/'), QLatin1Char('_'));
    if (!m_store)
        return name + QLatin1String("-export.json");
    return m_store->rootDir() + QLatin1Char('/') + name + QLatin1String("-export.json");
}
