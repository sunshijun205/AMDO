#ifndef ANALYSISPRESENTER_H
#define ANALYSISPRESENTER_H

#include <QObject>

class AnalysisPage;
class AnalysisDocumentService;
class AnalysisComputeService;

class AnalysisPresenter : public QObject
{
    Q_OBJECT
public:
    AnalysisPresenter(AnalysisPage *view, AnalysisDocumentService *document,
                      AnalysisComputeService *compute, QObject *parent = nullptr);

public slots:
    void onLoadRequested();
    void onSaveRequested();
    void onValidateRequested();
    void onPublishRequested();
    void onSwitchBaselineRequested(const QString &id);
    void onCopyToDraftRequested();
    void onReferencesChanged(const QString &sourceRevision, const QString &sourceCase,
                             const QString &sourceSrd, const QString &sourceCondition);
    void onRunRequested();

private:
    void refreshView();

    AnalysisPage *m_view;
    AnalysisDocumentService *m_document;
    AnalysisComputeService *m_compute;
};

#endif
