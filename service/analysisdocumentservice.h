#ifndef ANALYSISDOCUMENTSERVICE_H
#define ANALYSISDOCUMENTSERVICE_H

#include "model/analysisstore.h"
#include "model/analysistypes.h"

#include <QStringList>
#include <QVector>

class AnalysisDocumentService
{
public:
    explicit AnalysisDocumentService(AnalysisStore *store);

    bool loadDraft(QString *errorMessage = nullptr);
    bool saveValues(const QHash<QString, QString> &values, QString *errorMessage = nullptr);
    bool setReferences(const QString &sourceRevision, const QString &sourceCase,
                       const QString &sourceSrd, const QString &sourceCondition,
                       QString *errorMessage = nullptr);

    bool publishBaseline(QString *errorMessage = nullptr, AnalysisDocument *publishedOut = nullptr);
    bool openBaseline(const QString &id, QString *errorMessage = nullptr);
    bool copyBaselineToDraft(const QString &id, QString *errorMessage = nullptr);
    bool listBaselines(QVector<AnalysisBaselineInfo> *out, QString *errorMessage = nullptr) const;

    const AnalysisDocument &current() const { return m_current; }
    bool viewingReadOnly() const { return m_readOnly; }
    QString viewingId() const;
    QString draftPath() const;

    // 配置校验（缺关联、空字段、数值非法等）；不写盘。
    QStringList validate() const;

private:
    bool ensureEditable(QString *errorMessage) const;
    bool persist(QString *errorMessage);
    int maxBaselineVersion(QString *errorMessage) const;

    AnalysisStore *m_store;
    AnalysisDocument m_current;
    bool m_readOnly = false;
};

#endif
