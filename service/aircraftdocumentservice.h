#ifndef AIRCRAFTDOCUMENTSERVICE_H
#define AIRCRAFTDOCUMENTSERVICE_H

#include "model/aircraftstore.h"
#include "model/aircrafttypes.h"

class AircraftDocumentService
{
public:
    explicit AircraftDocumentService(AircraftStore *store);

    bool loadDraft(QString *errorMessage = nullptr);
    bool saveDraft(QString *errorMessage = nullptr);
    bool replaceDocument(const AircraftDocument &doc, QString *errorMessage = nullptr);
    bool newBlank(QString *errorMessage = nullptr);
    bool newFromTemplate(const QString &templateName, QString *errorMessage = nullptr);
    bool openBaseline(const QString &id, QString *errorMessage = nullptr);
    bool copyBaselineToDraft(const QString &id, QString *errorMessage = nullptr);
    bool publishBaseline(QString *errorMessage = nullptr);

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
    bool listBaselines(QVector<AircraftBaselineInfo> *out, QString *errorMessage = nullptr) const;
    QString viewingId() const;

private:
    bool ensureEditable(QString *errorMessage) const;
    bool persist(QString *errorMessage);
    QString nextSystemId() const;
    QString nextParameterId() const;
    int maxBaselineVersion(QString *errorMessage) const;

    AircraftStore *m_store;
    AircraftDocument m_current;
    bool m_readOnly = false;
};

#endif
