#ifndef ANALYSISPAGE_H
#define ANALYSISPAGE_H

#include "model/analysisresult.h"
#include "model/analysistypes.h"

#include <QHash>
#include <QVector>
#include <QWidget>
#include <memory>

class QComboBox;
class QLabel;
class QPushButton;
class AnalysisStore;
class AnalysisDocumentService;
class AnalysisComputeService;
class AnalysisPresenter;
class AircraftStore;
class SrdStore;

class AnalysisPage : public QWidget
{
    Q_OBJECT
public:
    explicit AnalysisPage(QWidget *parent = nullptr);
    ~AnalysisPage() override;

    // 供顶栏动作按钮调用。
    void requestSaveAll();
    void requestValidate();

    // 从表单读取全部字段取值。
    void snapshot(QHash<QString, QString> *values) const;

signals:
    void saveRequested();
    void validateRequested();
    void publishRequested();
    void switchBaselineRequested(const QString &id);
    void copyToDraftRequested();
    void referencesChanged(const QString &sourceRevision, const QString &sourceCase,
                           const QString &sourceSrd, const QString &sourceCondition);
    void runRequested();

public slots:
    void setDocument(const AnalysisDocument &doc);
    void setBaselines(const QVector<AnalysisBaselineInfo> &baselines, const QString &currentId);
    void setReadOnly(bool readOnly);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);
    void reportValidation(const QStringList &issues);
    void showRunResult(const AnalysisRunResult &result);

private slots:
    void onBaselineChanged(int index);
    void onReferenceChanged(int index);
    void onSrdChanged(int index);

private:
    void buildUi();
    void reloadReferenceOptions();
    void reloadConditionOptions(const QString &srdId);
    QWidget *buildDomainPage(const AnalysisDomain &domain);
    QWidget *makeFieldWidget(const AnalysisField &field, const QString &key);

    QHash<QString, QWidget *> m_editors; // 字段键 → 内部编辑控件
    QLabel *m_status = nullptr;
    QComboBox *m_baselineBox = nullptr;
    QComboBox *m_revisionBox = nullptr;
    QComboBox *m_srdBox = nullptr;
    QComboBox *m_conditionBox = nullptr;
    QComboBox *m_caseBox = nullptr;
    QPushButton *m_publishBtn = nullptr;
    QPushButton *m_copyDraftBtn = nullptr;
    QPushButton *m_runBtn = nullptr;
    bool m_readOnly = false;
    bool m_updating = false;

    std::unique_ptr<AircraftStore> m_aircraftStore;
    std::unique_ptr<SrdStore> m_srdStore;
    std::unique_ptr<AnalysisStore> m_store;
    std::unique_ptr<AnalysisDocumentService> m_document;
    std::unique_ptr<AnalysisComputeService> m_compute;
    AnalysisPresenter *m_presenter = nullptr;
};

#endif
