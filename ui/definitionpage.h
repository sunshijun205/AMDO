#ifndef DEFINITIONPAGE_H
#define DEFINITIONPAGE_H

#include "model/aircrafttypes.h"

#include <QStringList>
#include <QWidget>
#include <memory>

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTreeWidget;
class AircraftStore;
class AircraftDocumentService;
class AircraftImportExportService;
class DefinitionPresenter;

class DefinitionPage : public QWidget
{
    Q_OBJECT
public:
    explicit DefinitionPage(QWidget *parent = nullptr);
    ~DefinitionPage() override;

    // 供顶栏动作按钮调用。
    void requestImport();
    void requestExport();
    void requestSaveAll();
    void requestIntegrityCheck();

    void snapshotSemantics(AcSemantics *semantics, QString *title) const;
    void snapshotConfiguration(AcConfiguration *configuration,
                               QVector<AcSystemItem> *systems) const;
    void snapshotParameters(QVector<AcParameter> *parameters) const;

    QString selectedSystemId() const;
    QString selectedParameterId() const;

signals:
    void loadRequested();
    void switchBaselineRequested(const QString &id);
    void copyBaselineRequested();
    void newBlankRequested();
    void newFromTemplateRequested(const QString &templateName);
    void saveAllRequested();
    void saveSemanticsRequested();
    void saveConfigurationRequested();
    void saveParametersRequested();
    void addSystemRequested();
    void removeSystemRequested();
    void addParameterRequested();
    void removeParameterRequested();
    void publishRequested();
    void importPathRequested(const QString &path);
    void exportRequested();
    void integrityCheckRequested();

public slots:
    void setDocument(const AircraftDocument &doc, bool readOnly);
    void setBaselines(const QVector<AircraftBaselineInfo> &baselines, const QString &currentId);
    void setBusy(bool busy);
    void setStatus(const QString &text, bool isError = false);
    void showError(const QString &message);
    void reportIntegrity(const QStringList &issues);

private slots:
    void onBaselineChanged(int index);
    void onNewClicked();

private:
    void buildUi();
    QWidget *buildSemanticPage();
    QWidget *buildConfigurationPage();
    QWidget *buildGeometryPage();
    QWidget *buildVisualizationPage();
    void setReadOnly(bool readOnly);

    AircraftDocument m_shown;
    bool m_readOnly = false;
    bool m_updating = false;

    std::unique_ptr<AircraftStore> m_store;
    std::unique_ptr<AircraftDocumentService> m_document;
    std::unique_ptr<AircraftImportExportService> m_io;
    DefinitionPresenter *m_presenter = nullptr;

    QComboBox *m_baselineBox = nullptr;
    QLabel *m_pageStatus = nullptr;
    QPushButton *m_copyDraftBtn = nullptr;
    QPushButton *m_newBtn = nullptr;

    // 语义数据模型
    QWidget *m_semKpiHost = nullptr;
    QLineEdit *m_title = nullptr;
    QLineEdit *m_schema = nullptr;
    QLineEdit *m_namespace = nullptr;
    QLineEdit *m_reference = nullptr;
    QLineEdit *m_exchange = nullptr;
    QLineEdit *m_lengthUnit = nullptr;
    QLineEdit *m_massUnit = nullptr;
    QLineEdit *m_coordSystem = nullptr;
    QLineEdit *m_origin = nullptr;
    QTreeWidget *m_objectTree = nullptr;
    QWidget *m_modelSpecHost = nullptr;
    QWidget *m_semCheckHost = nullptr;
    QWidget *m_interfaceHost = nullptr;

    // 总体方案配置
    QWidget *m_configKpiHost = nullptr;
    QComboBox *m_category = nullptr;
    QComboBox *m_wingLayout = nullptr;
    QComboBox *m_tailConfig = nullptr;
    QComboBox *m_engineArrangement = nullptr;
    QComboBox *m_gearType = nullptr;
    QComboBox *m_cabinLayout = nullptr;
    QComboBox *m_primaryMaterial = nullptr;
    QLineEdit *m_connectionStrategy = nullptr;
    QLineEdit *m_symmetry = nullptr;
    QTableWidget *m_systemTable = nullptr;
    QWidget *m_integrityHost = nullptr;
    QWidget *m_todoHost = nullptr;

    // 参数化几何
    QWidget *m_geoKpiHost = nullptr;
    QTableWidget *m_paramTable = nullptr;
    QWidget *m_relationHost = nullptr;

    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_publishBtn = nullptr;
    QPushButton *m_importBtn = nullptr;
    QPushButton *m_exportBtn = nullptr;
    QPushButton *m_saveSemBtn = nullptr;
    QPushButton *m_saveConfigBtn = nullptr;
    QPushButton *m_saveParamBtn = nullptr;
};

#endif
