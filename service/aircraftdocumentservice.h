#ifndef AIRCRAFTDOCUMENTSERVICE_H
#define AIRCRAFTDOCUMENTSERVICE_H

#include "model/aircraftstore.h"
#include "model/aircrafttypes.h"

class AircraftCpacsService;

class AircraftDocumentService
{
public:
    explicit AircraftDocumentService(AircraftStore *store, AircraftCpacsService *cpacs = nullptr);

    bool loadDraft(QString *errorMessage = nullptr);
    bool saveDraft(QString *errorMessage = nullptr);
    bool replaceDocument(const AircraftDocument &doc, QString *errorMessage = nullptr);
    bool newBlank(QString *errorMessage = nullptr);
    bool newFromTemplate(const QString &templateName, QString *errorMessage = nullptr);
    bool openBaseline(const QString &id, QString *errorMessage = nullptr);
    bool copyBaselineToDraft(const QString &id, QString *errorMessage = nullptr);
    bool publishBaseline(QString *errorMessage = nullptr, AircraftDocument *publishedOut = nullptr);
    bool createCaseSnapshot(QString *outPath = nullptr, QString *errorMessage = nullptr);

    bool setSemantics(const AcSemantics &semantics, const QString &title, QString *errorMessage = nullptr);
    bool setConfiguration(const AcConfiguration &configuration,
                          const QVector<AcSystemItem> &systems,
                          QString *errorMessage = nullptr);
    bool setParameters(const QVector<AcParameter> &parameters, QString *errorMessage = nullptr);

    bool addSystem(QString *errorMessage = nullptr);
    bool removeSystem(const QString &id, QString *errorMessage = nullptr);
    bool addParameter(QString *errorMessage = nullptr);
    bool removeParameter(const QString &id, QString *errorMessage = nullptr);

    const AircraftDocument &current() const { return m_current; }
    bool viewingReadOnly() const { return m_readOnly; }
    QString draftPath() const;
    QString cpacsPathForVersion(int version) const;
    bool listBaselines(QVector<AircraftBaselineInfo> *out, QString *errorMessage = nullptr) const;
    QString viewingId() const;

private:
    bool ensureEditable(QString *errorMessage) const;
    bool ensureDefaultBaseline(QString *errorMessage);
    bool ensureCpacsRevisions(QString *errorMessage);
    bool persist(QString *errorMessage);
    QString nextSystemId() const;
    QString nextParameterId() const;
    int maxBaselineVersion(QString *errorMessage) const;

    bool writeCpacsRevision(const AircraftDocument &doc, QString *errorMessage);

    AircraftStore *m_store;
    AircraftCpacsService *m_cpacs;
    AircraftDocument m_current;
    bool m_readOnly = false;
};

#endif
