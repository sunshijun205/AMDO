#ifndef ANALYSISPAGE_H
#define ANALYSISPAGE_H

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
class AnalysisPresenter;

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

public slots:
    void setDocument(const AnalysisDocument &doc);
    void setBaselines(const QVector<AnalysisBaselineInfo> &baselines, const QString &currentId);
    void setReadOnly(bool readOnly);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);
    void reportValidation(const QStringList &issues);

private slots:
    void onBaselineChanged(int index);

private:
    void buildUi();
    QWidget *buildDomainPage(const AnalysisDomain &domain);
    QWidget *makeFieldWidget(const AnalysisField &field, const QString &key);

    QHash<QString, QWidget *> m_editors; // 字段键 → 内部编辑控件
    QLabel *m_status = nullptr;
    QComboBox *m_baselineBox = nullptr;
    QPushButton *m_publishBtn = nullptr;
    QPushButton *m_copyDraftBtn = nullptr;
    bool m_readOnly = false;
    bool m_updating = false;

    std::unique_ptr<AnalysisStore> m_store;
    std::unique_ptr<AnalysisDocumentService> m_document;
    AnalysisPresenter *m_presenter = nullptr;
};

#endif
