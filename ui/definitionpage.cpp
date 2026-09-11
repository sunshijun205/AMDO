#include "definitionpage.h"

#include "chartwidgets.h"
#include "controller/definitionpresenter.h"
#include "model/aircraftcatalogs.h"
#include "model/aircraftstore.h"
#include "service/aircraftdocumentservice.h"
#include "service/aircraftimportexportservice.h"
#include "uihelpers.h"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

static void replaceHost(QWidget *host, QWidget *child)
{
    QVBoxLayout *lay = qobject_cast<QVBoxLayout *>(host->layout());
    if (!lay) {
        lay = new QVBoxLayout(host);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
    }
    while (lay->count() > 0) {
        QLayoutItem *item = lay->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    lay->addWidget(child);
}

static QWidget *makeKpiHost()
{
    auto *host = new QWidget;
    auto *lay = new QVBoxLayout(host);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    return host;
}

static int selectedRow(const QTableWidget *table)
{
    if (!table || table->selectionModel() == nullptr)
        return -1;
    const QModelIndexList rows = table->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return table->currentRow();
    return rows.first().row();
}

static QString selectedId(const QTableWidget *table)
{
    return tableRowId(table, selectedRow(table)).toString();
}

static double parseNumberText(QString text, bool *ok)
{
    text.remove(QLatin1Char(','));
    text = text.trimmed();
    return text.toDouble(ok);
}

static void selectCombo(QComboBox *box, const QString &value)
{
    if (!box)
        return;
    int idx = box->findText(value);
    if (idx < 0 && !value.isEmpty()) {
        box->insertItem(0, value);
        idx = 0;
    }
    box->setCurrentIndex(idx >= 0 ? idx : 0);
}

static QString orDash(const QString &value)
{
    return value.isEmpty() ? QStringLiteral("—") : value;
}

static QWidget *makeProgressPanel(const QVector<QPair<QString, int>> &rows)
{
    auto *w = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(2);
    for (int i = 0; i < rows.size(); ++i)
        lay->addWidget(makeProgressRow(rows[i].first, QString::number(rows[i].second) + QLatin1Char('%'),
                                       rows[i].second));
    return w;
}

static AircraftKpis computeKpis(const AircraftDocument &d)
{
    AircraftKpis k;
    k.systemCount = d.systems.size();
    k.objectCount = d.systems.size() + 1;
    k.parameterCount = d.parameters.size();
    k.categoryText = d.configuration.category.isEmpty()
                         ? QString::fromUtf8("未定义") : d.configuration.category;

    QStringList types;
    for (int i = 0; i < d.systems.size(); ++i) {
        if (!types.contains(d.systems[i].object))
            types.append(d.systems[i].object);
    }
    k.typeCount = types.size();

    for (int i = 0; i < d.parameters.size(); ++i) {
        if (d.parameters[i].driveType.contains(QString::fromUtf8("设计变量")))
            ++k.designVarCount;
        else if (d.parameters[i].driveType.contains(QString::fromUtf8("公式"))
                 || d.parameters[i].driveType.contains(QString::fromUtf8("自动")))
            ++k.drivenCount;
    }

    int total = 0;
    int ok = 0;
    auto chk = [&](bool cond) {
        ++total;
        if (cond)
            ++ok;
    };
    chk(!d.title.isEmpty());
    chk(!d.configuration.category.isEmpty());
    chk(!d.semantics.lengthUnit.isEmpty());
    chk(!d.semantics.massUnit.isEmpty());
    chk(!d.semantics.coordinateSystem.isEmpty());
    for (int i = 0; i < d.systems.size(); ++i)
        chk(!d.systems[i].object.isEmpty() && !d.systems[i].status.contains(QString::fromUtf8("待")));
    for (int i = 0; i < d.parameters.size(); ++i)
        chk(d.parameters[i].valueKnown);
    k.completeness = total > 0 ? ok * 100 / total : 0;
    return k;
}

DefinitionPage::DefinitionPage(QWidget *parent)
    : QWidget(parent)
{
    buildUi();

    m_store.reset(new AircraftStore);
    m_document.reset(new AircraftDocumentService(m_store.get()));
    m_io.reset(new AircraftImportExportService(m_store.get()));
    m_presenter = new DefinitionPresenter(this, m_document.get(), m_io.get(), this);
}

DefinitionPage::~DefinitionPage() = default;

void DefinitionPage::requestImport()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QString::fromUtf8("导入方案"), QString(),
        QString::fromUtf8("方案文件 (*.json);;JSON (*.json)"));
    if (path.isEmpty())
        return;
    emit importPathRequested(path);
}

void DefinitionPage::requestExport()
{
    emit exportRequested();
}

void DefinitionPage::requestSaveAll()
{
    emit saveAllRequested();
}

void DefinitionPage::requestIntegrityCheck()
{
    emit integrityCheckRequested();
}

void DefinitionPage::buildUi()
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);

    auto *head = new QWidget;
    auto *hl = new QHBoxLayout(head);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(18);
    auto *left = new QWidget;
    auto *vl = new QVBoxLayout(left);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(4);
    auto *title = new QLabel(QString::fromUtf8("飞机方案定义"));
    title->setObjectName(QStringLiteral("PageTitle"));
    auto *sub = new QLabel(QString::fromUtf8("以统一语义模型组织构型、系统、几何及其关联关系"));
    sub->setObjectName(QStringLiteral("PageSubtitle"));
    sub->setWordWrap(true);
    vl->addWidget(title);
    vl->addWidget(sub);
    m_pageStatus = new QLabel;
    m_pageStatus->setObjectName(QStringLiteral("MutedLabel"));
    m_pageStatus->setWordWrap(true);
    vl->addWidget(m_pageStatus);
    hl->addWidget(left, 1);

    auto *right = new QWidget;
    auto *rl = new QVBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setSpacing(6);
    m_baselineBox = new QComboBox;
    m_baselineBox->setMinimumWidth(220);
    rl->addWidget(makeLabeled(QString::fromUtf8("方案版本"), m_baselineBox));
    auto *headBtns = new QWidget;
    auto *hbl = new QHBoxLayout(headBtns);
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->setSpacing(6);
    m_newBtn = makeButton(QString::fromUtf8("新建方案"));
    m_copyDraftBtn = makeButton(QString::fromUtf8("另存为新草稿"));
    hbl->addWidget(m_newBtn);
    hbl->addWidget(m_copyDraftBtn);
    rl->addWidget(headBtns);
    hl->addWidget(right, 0, Qt::AlignTop);
    lay->addWidget(head);

    connect(m_baselineBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DefinitionPage::onBaselineChanged);
    connect(m_copyDraftBtn, &QPushButton::clicked, this, &DefinitionPage::copyBaselineRequested);
    connect(m_newBtn, &QPushButton::clicked, this, &DefinitionPage::onNewClicked);

    auto *toolbar = new QWidget;
    auto *tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(0, 0, 0, 0);
    tbl->setSpacing(8);
    m_importBtn = makeButton(QString::fromUtf8("导入方案"));
    m_exportBtn = makeButton(QString::fromUtf8("导出方案"));
    tbl->addWidget(m_importBtn);
    tbl->addWidget(m_exportBtn);
    tbl->addStretch();
    m_saveBtn = makeButton(QString::fromUtf8("保存方案定义"), true);
    m_publishBtn = makeButton(QString::fromUtf8("创建方案版本"));
    tbl->addWidget(m_saveBtn);
    tbl->addWidget(m_publishBtn);
    lay->addWidget(toolbar);
    connect(m_importBtn, &QPushButton::clicked, this, &DefinitionPage::requestImport);
    connect(m_exportBtn, &QPushButton::clicked, this, &DefinitionPage::exportRequested);
    connect(m_saveBtn, &QPushButton::clicked, this, &DefinitionPage::saveAllRequested);
    connect(m_publishBtn, &QPushButton::clicked, this, &DefinitionPage::publishRequested);

    auto *tabs = new SubTabBar({
        {QStringLiteral("semantic"), QString::fromUtf8("飞机语义数据模型")},
        {QStringLiteral("configuration"), QString::fromUtf8("总体方案配置")},
        {QStringLiteral("geometry"), QString::fromUtf8("参数化几何")},
        {QStringLiteral("visualization"), QString::fromUtf8("三维可视化")}
    }, QStringLiteral("semantic"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(buildSemanticPage());
    stack->addWidget(buildConfigurationPage());
    stack->addWidget(buildGeometryPage());
    stack->addWidget(buildVisualizationPage());
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("configuration"))
            stack->setCurrentIndex(1);
        else if (id == QLatin1String("geometry"))
            stack->setCurrentIndex(2);
        else if (id == QLatin1String("visualization"))
            stack->setCurrentIndex(3);
        else
            stack->setCurrentIndex(0);
    });

    outer->addWidget(wrapScroll(body));
}

QWidget *DefinitionPage::buildSemanticPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_semKpiHost = makeKpiHost();
    lay->addWidget(m_semKpiHost);

    auto *top = new QWidget;
    auto *hl = new QHBoxLayout(top);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *mid = new QWidget;
    auto *ml = new QHBoxLayout(mid);
    ml->setContentsMargins(0, 0, 0, 0);
    ml->setSpacing(12);

    auto *treeP = makePanel();
    treeP->layout()->addWidget(makePanelTitle(QString::fromUtf8("方案对象树"), QString::fromUtf8("由构型与系统派生")));
    m_objectTree = new QTreeWidget;
    m_objectTree->setHeaderHidden(true);
    m_objectTree->setColumnCount(2);
    m_objectTree->setRootIsDecorated(true);
    m_objectTree->setMinimumHeight(220);
    treeP->layout()->addWidget(m_objectTree);

    auto *attr = makePanel();
    attr->layout()->addWidget(makePanelTitle(QString::fromUtf8("对象类型、属性与标识")));
    m_title = makeInput(QString());
    m_schema = makeInput(QString());
    m_namespace = makeInput(QString());
    m_reference = makeInput(QString());
    m_exchange = makeInput(QString());
    m_lengthUnit = makeInput(QString());
    m_massUnit = makeInput(QString());
    m_coordSystem = makeInput(QString());
    m_origin = makeInput(QString());
    attr->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("方案名称"), m_title),
        makeLabeled(QStringLiteral("Schema"), m_schema),
        makeLabeled(QString::fromUtf8("命名空间"), m_namespace),
        makeLabeled(QString::fromUtf8("引用策略"), m_reference),
        makeLabeled(QString::fromUtf8("交换格式"), m_exchange),
        makeLabeled(QString::fromUtf8("长度单位"), m_lengthUnit),
        makeLabeled(QString::fromUtf8("质量单位"), m_massUnit),
        makeLabeled(QString::fromUtf8("坐标系"), m_coordSystem),
        makeLabeled(QString::fromUtf8("原点"), m_origin)
    }, 2));
    ml->addWidget(treeP, 1);
    ml->addWidget(attr, 1);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *spec = makePanel();
    spec->layout()->addWidget(makePanelTitle(QString::fromUtf8("模型规范")));
    m_modelSpecHost = makeKpiHost();
    spec->layout()->addWidget(m_modelSpecHost);
    al->addWidget(spec);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("语义检查")));
    m_semCheckHost = makeKpiHost();
    chk->layout()->addWidget(m_semCheckHost);
    al->addWidget(chk);
    m_saveSemBtn = makeButton(QString::fromUtf8("保存语义模型"), true);
    connect(m_saveSemBtn, &QPushButton::clicked, this, &DefinitionPage::saveSemanticsRequested);
    al->addWidget(m_saveSemBtn);
    al->addStretch();

    hl->addWidget(mid, 1);
    hl->addWidget(aside);
    lay->addWidget(top);

    auto *iface = makePanel();
    iface->layout()->addWidget(makePanelTitle(QString::fromUtf8("引用与接口"),
                                              QString::fromUtf8("由系统对象派生")));
    m_interfaceHost = makeKpiHost();
    iface->layout()->addWidget(m_interfaceHost);
    lay->addWidget(iface);
    return root;
}

QWidget *DefinitionPage::buildConfigurationPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_configKpiHost = makeKpiHost();
    lay->addWidget(m_configKpiHost);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    auto *cfg = makePanel();
    cfg->layout()->addWidget(makePanelTitle(QString::fromUtf8("构型与总体布局")));
    m_category = new QComboBox;
    m_category->addItems(AircraftCatalogs::categories());
    m_wingLayout = new QComboBox;
    m_wingLayout->addItems(AircraftCatalogs::wingLayouts());
    m_tailConfig = new QComboBox;
    m_tailConfig->addItems(AircraftCatalogs::tailConfigs());
    m_engineArrangement = new QComboBox;
    m_engineArrangement->addItems(AircraftCatalogs::engineArrangements());
    m_gearType = new QComboBox;
    m_gearType->addItems(AircraftCatalogs::gearTypes());
    m_cabinLayout = new QComboBox;
    m_cabinLayout->addItems(AircraftCatalogs::cabinLayouts());
    cfg->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("飞机类别"), m_category),
        makeLabeled(QString::fromUtf8("机翼布局"), m_wingLayout),
        makeLabeled(QString::fromUtf8("尾翼构型"), m_tailConfig),
        makeLabeled(QString::fromUtf8("发动机布置"), m_engineArrangement),
        makeLabeled(QString::fromUtf8("起落架形式"), m_gearType),
        makeLabeled(QString::fromUtf8("客舱布局"), m_cabinLayout)
    }, 3));
    vl->addWidget(cfg);

    auto *sys = makePanel();
    sys->layout()->addWidget(makePanelTitle(QString::fromUtf8("系统、设备与安装定位")));
    m_systemTable = makeEditableTable({
        QString::fromUtf8("编号"), QString::fromUtf8("对象"), QString::fromUtf8("方案"),
        QString::fromUtf8("安装/定位"), QString::fromUtf8("材料或属性"), QString::fromUtf8("状态")
    });
    sys->layout()->addWidget(m_systemTable);
    auto *sysBtns = new QWidget;
    auto *sbl = new QHBoxLayout(sysBtns);
    sbl->setContentsMargins(0, 0, 0, 0);
    auto *addS = makeButton(QString::fromUtf8("添加对象"));
    auto *delS = makeButton(QString::fromUtf8("删除对象"));
    sbl->addWidget(addS);
    sbl->addWidget(delS);
    sbl->addStretch();
    sys->layout()->addWidget(sysBtns);
    vl->addWidget(sys);
    connect(addS, &QPushButton::clicked, this, &DefinitionPage::addSystemRequested);
    connect(delS, &QPushButton::clicked, this, &DefinitionPage::removeSystemRequested);

    auto *mat = makePanel();
    mat->layout()->addWidget(makePanelTitle(QString::fromUtf8("材料、物理属性与连接")));
    m_primaryMaterial = new QComboBox;
    m_primaryMaterial->addItems(AircraftCatalogs::materials());
    m_connectionStrategy = makeInput(QString());
    m_symmetry = makeInput(QString());
    mat->layout()->addWidget(makeMiniFields({
        makeLabeled(QString::fromUtf8("主结构材料"), m_primaryMaterial),
        makeLabeled(QString::fromUtf8("连接策略"), m_connectionStrategy),
        makeLabeled(QString::fromUtf8("对称关系"), m_symmetry)
    }, 3));
    vl->addWidget(mat);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *prog = makePanel();
    prog->layout()->addWidget(makePanelTitle(QString::fromUtf8("完整性检查")));
    m_integrityHost = makeKpiHost();
    prog->layout()->addWidget(m_integrityHost);
    al->addWidget(prog);
    auto *todo = makePanel();
    todo->layout()->addWidget(makePanelTitle(QString::fromUtf8("待处理")));
    m_todoHost = makeKpiHost();
    todo->layout()->addWidget(m_todoHost);
    al->addWidget(todo);
    m_saveConfigBtn = makeButton(QString::fromUtf8("保存方案配置"), true);
    connect(m_saveConfigBtn, &QPushButton::clicked, this, &DefinitionPage::saveConfigurationRequested);
    al->addWidget(m_saveConfigBtn);
    auto *checkBtn = makeButton(QString::fromUtf8("执行完整性检查"));
    connect(checkBtn, &QPushButton::clicked, this, &DefinitionPage::integrityCheckRequested);
    al->addWidget(checkBtn);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QWidget *DefinitionPage::buildGeometryPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    m_geoKpiHost = makeKpiHost();
    lay->addWidget(m_geoKpiHost);

    auto *two = new QWidget;
    auto *hl = new QHBoxLayout(two);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *view = makePanel();
    view->layout()->addWidget(makePanelTitle(QString::fromUtf8("参数化几何预览"), QString::fromUtf8("俯视参数骨架（示意）")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("AircraftView"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->addWidget(new AircraftTopView);
    view->layout()->addWidget(canvas);
    hl->addWidget(view, 1);

    auto *dims = makePanel();
    dims->layout()->addWidget(makePanelTitle(QString::fromUtf8("尺寸与外形参数体系")));
    m_paramTable = makeEditableTable({
        QString::fromUtf8("编号"), QString::fromUtf8("参数"), QString::fromUtf8("符号"),
        QString::fromUtf8("数值"), QString::fromUtf8("单位"), QString::fromUtf8("驱动方式"),
        QString::fromUtf8("备注")
    });
    dims->layout()->addWidget(m_paramTable);
    auto *paramBtns = new QWidget;
    auto *pbl = new QHBoxLayout(paramBtns);
    pbl->setContentsMargins(0, 0, 0, 0);
    auto *addP = makeButton(QString::fromUtf8("添加参数"));
    auto *delP = makeButton(QString::fromUtf8("删除参数"));
    m_saveParamBtn = makeButton(QString::fromUtf8("保存几何参数"), true);
    pbl->addWidget(addP);
    pbl->addWidget(delP);
    pbl->addStretch();
    pbl->addWidget(m_saveParamBtn);
    dims->layout()->addWidget(paramBtns);
    hl->addWidget(dims, 1);
    lay->addWidget(two);
    connect(addP, &QPushButton::clicked, this, &DefinitionPage::addParameterRequested);
    connect(delP, &QPushButton::clicked, this, &DefinitionPage::removeParameterRequested);
    connect(m_saveParamBtn, &QPushButton::clicked, this, &DefinitionPage::saveParametersRequested);

    auto *bottom = new QWidget;
    auto *bl = new QHBoxLayout(bottom);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(12);
    auto *rel = makePanel();
    rel->layout()->addWidget(makePanelTitle(QString::fromUtf8("几何约束与驱动关系"),
                                            QString::fromUtf8("由参数驱动方式派生")));
    m_relationHost = makeKpiHost();
    rel->layout()->addWidget(m_relationHost);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *upd = makePanel();
    upd->layout()->addWidget(makePanelTitle(QString::fromUtf8("更新设置")));
    upd->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("更新模式"), QString::fromUtf8("参数变化后自动"), {QString::fromUtf8("手动更新")}),
        makeField(QString::fromUtf8("几何容差"), QStringLiteral("1e-4 m")),
        makeField(QString::fromUtf8("失败处理"), QString::fromUtf8("回退上一版本")),
        makeField(QString::fromUtf8("联动分析"), QString::fromUtf8("质量 + 气动"))
    }, 1));
    al->addWidget(upd);
    auto *updBtn = makeButton(QString::fromUtf8("更新几何并检查"), true);
    connect(updBtn, &QPushButton::clicked, this, &DefinitionPage::integrityCheckRequested);
    al->addWidget(updBtn);
    al->addStretch();
    bl->addWidget(rel, 1);
    bl->addWidget(aside);
    lay->addWidget(bottom);
    return root;
}

QWidget *DefinitionPage::buildVisualizationPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    auto *toolbar = new QWidget;
    auto *tl = new QHBoxLayout(toolbar);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(9);
    tl->addWidget(makeSelectField(QString::fromUtf8("显示构型"), QString::fromUtf8("巡航构型"), {QString::fromUtf8("起飞构型"), QString::fromUtf8("着陆构型")}));
    tl->addWidget(makeSelectField(QString::fromUtf8("视图"), QString::fromUtf8("等轴测"), {QString::fromUtf8("俯视"), QString::fromUtf8("侧视"), QString::fromUtf8("正视")}));
    tl->addWidget(makeSelectField(QString::fromUtf8("着色方式"), QString::fromUtf8("按系统"), {QString::fromUtf8("按材料"), QString::fromUtf8("按学科")}));
    tl->addStretch();
    auto *cut = makeButton(QString::fromUtf8("剖切"));
    auto *measure = makeButton(QString::fromUtf8("测量"));
    auto *save = makeButton(QString::fromUtf8("保存视图"), true);
    wireDummyAction(cut, this);
    wireDummyAction(measure, this);
    wireDummyAction(save, this);
    tl->addWidget(cut);
    tl->addWidget(measure);
    tl->addWidget(save);
    lay->addWidget(toolbar);

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);
    auto *view = makePanel();
    view->layout()->addWidget(makePanelTitle(QString::fromUtf8("三维方案视图"),
                                             QString::fromUtf8("示意占位 · 尚未接入几何内核/真三维")));
    auto *canvas = new QFrame;
    canvas->setObjectName(QStringLiteral("AircraftView"));
    auto *cl = new QVBoxLayout(canvas);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->addWidget(new AircraftIsoView);
    view->layout()->addWidget(canvas);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *disp = makePanel();
    disp->layout()->addWidget(makePanelTitle(QString::fromUtf8("显示与选择")));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("机体结构"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("推进系统"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("设备与系统"), true));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("参考几何"), false));
    disp->layout()->addWidget(makeCheck(QString::fromUtf8("接口与连接点"), false));
    al->addWidget(disp);
    auto *sel = makePanel();
    sel->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前选择：右发动机")));
    sel->layout()->addWidget(makeSummary({
        {QString::fromUtf8("对象"), QStringLiteral("Engine-02")},
        {QString::fromUtf8("位置"), QStringLiteral("X 15.2 / Y 6.4")},
        {QString::fromUtf8("方案参数"), QString::fromUtf8("7 个")},
        {QString::fromUtf8("关联分析"), QString::fromUtf8("推进 / 气动 / 重量")}
    }));
    al->addWidget(sel);
    auto *linkBtn = makeButton(QString::fromUtf8("查看对象与分析关联"), true);
    wireDummyAction(linkBtn, this);
    al->addWidget(linkBtn);
    al->addStretch();

    hl->addWidget(view, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

QString DefinitionPage::selectedSystemId() const
{
    return selectedId(m_systemTable);
}

QString DefinitionPage::selectedParameterId() const
{
    return selectedId(m_paramTable);
}

void DefinitionPage::onNewClicked()
{
    const QStringList templates = AircraftCatalogs::templateNames();
    bool ok = false;
    const QString choice = QInputDialog::getItem(
        this, QString::fromUtf8("新建方案"),
        QString::fromUtf8("选择起始模板："), templates, 0, false, &ok);
    if (!ok || choice.isEmpty())
        return;
    if (choice == QString::fromUtf8("空白方案"))
        emit newBlankRequested();
    else
        emit newFromTemplateRequested(choice);
}

void DefinitionPage::onBaselineChanged(int)
{
    if (m_updating)
        return;
    emit switchBaselineRequested(m_baselineBox->currentData().toString());
}

void DefinitionPage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    const QList<QWidget *> editors = findChildren<QWidget *>();
    for (int i = 0; i < editors.size(); ++i) {
        QWidget *w = editors[i];
        if (w->objectName() == QLatin1String("SubTabButton")
            || w->objectName() == QLatin1String("ModeButton")
            || w == m_baselineBox)
            continue;
        const bool isEditor = qobject_cast<QLineEdit *>(w) || qobject_cast<QComboBox *>(w)
            || qobject_cast<QTableWidget *>(w) || qobject_cast<QPushButton *>(w);
        if (!isEditor)
            continue;
        if (w == m_copyDraftBtn || w == m_exportBtn)
            continue;
        w->setEnabled(!readOnly);
    }
    if (m_copyDraftBtn)
        m_copyDraftBtn->setEnabled(readOnly);
    if (m_baselineBox)
        m_baselineBox->setEnabled(true);
}

void DefinitionPage::setBusy(bool busy)
{
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (int i = 0; i < buttons.size(); ++i) {
        if (buttons[i]->objectName() == QLatin1String("SubTabButton"))
            continue;
        const bool allowInReadOnly = (buttons[i] == m_copyDraftBtn || buttons[i] == m_exportBtn);
        buttons[i]->setEnabled(!busy && (!m_readOnly || allowInReadOnly));
    }
    if (m_baselineBox)
        m_baselineBox->setEnabled(!busy);
}

void DefinitionPage::setStatus(const QString &text, bool isError)
{
    m_pageStatus->setText(text);
    m_pageStatus->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void DefinitionPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("飞机方案定义"), message);
}

void DefinitionPage::reportIntegrity(const QStringList &issues)
{
    if (issues.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("完整性检查"),
                                 QString::fromUtf8("完整性检查通过，未发现待处理项。"));
        return;
    }
    QString text = QString::fromUtf8("发现以下待处理项：\n\n");
    for (int i = 0; i < issues.size(); ++i)
        text += QString::fromUtf8("• ") + issues[i] + QLatin1Char('\n');
    QMessageBox::warning(this, QString::fromUtf8("完整性检查"), text);
}

void DefinitionPage::setBaselines(const QVector<AircraftBaselineInfo> &baselines, const QString &currentId)
{
    m_updating = true;
    m_baselineBox->clear();
    m_baselineBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    int current = 0;
    for (int i = 0; i < baselines.size(); ++i) {
        QString label = baselines[i].title;
        if (label.isEmpty())
            label = baselines[i].id;
        if (baselines[i].version > 0)
            label += QString::fromUtf8("  v%1").arg(baselines[i].version);
        if (!baselines[i].publishedAt.isEmpty())
            label += QStringLiteral("  ") + baselines[i].publishedAt;
        m_baselineBox->addItem(label, baselines[i].id);
        if (baselines[i].id == currentId)
            current = i + 1;
    }
    if (currentId == QLatin1String("draft"))
        current = 0;
    m_baselineBox->setCurrentIndex(current);
    m_updating = false;
}

void DefinitionPage::setDocument(const AircraftDocument &doc, bool readOnly)
{
    m_shown = doc;
    m_updating = true;

    m_title->setText(doc.title);
    m_schema->setText(doc.semantics.schema);
    m_namespace->setText(doc.semantics.namespaceStr);
    m_reference->setText(doc.semantics.referenceStrategy);
    m_exchange->setText(doc.semantics.exchangeFormat);
    m_lengthUnit->setText(doc.semantics.lengthUnit);
    m_massUnit->setText(doc.semantics.massUnit);
    m_coordSystem->setText(doc.semantics.coordinateSystem);
    m_origin->setText(doc.semantics.origin);

    selectCombo(m_category, doc.configuration.category);
    selectCombo(m_wingLayout, doc.configuration.wingLayout);
    selectCombo(m_tailConfig, doc.configuration.tailConfig);
    selectCombo(m_engineArrangement, doc.configuration.engineArrangement);
    selectCombo(m_gearType, doc.configuration.gearType);
    selectCombo(m_cabinLayout, doc.configuration.cabinLayout);
    selectCombo(m_primaryMaterial, doc.configuration.primaryMaterial);
    m_connectionStrategy->setText(doc.configuration.connectionStrategy);
    m_symmetry->setText(doc.configuration.symmetry);

    QVector<QStringList> sysRows;
    QVector<QVariant> sysIds;
    for (int i = 0; i < doc.systems.size(); ++i) {
        sysRows.append({doc.systems[i].id, doc.systems[i].object, doc.systems[i].scheme,
                        doc.systems[i].mounting, doc.systems[i].material, doc.systems[i].status});
        sysIds.append(doc.systems[i].id);
    }
    setTableContents(m_systemTable, sysRows, sysIds);

    QVector<QStringList> paramRows;
    QVector<QVariant> paramIds;
    for (int i = 0; i < doc.parameters.size(); ++i) {
        paramRows.append({doc.parameters[i].id, doc.parameters[i].name, doc.parameters[i].symbol,
                          doc.parameters[i].valueKnown ? acFormatNumber(doc.parameters[i].value) : QString(),
                          doc.parameters[i].unit, doc.parameters[i].driveType, doc.parameters[i].note});
        paramIds.append(doc.parameters[i].id);
    }
    setTableContents(m_paramTable, paramRows, paramIds);

    m_objectTree->clear();
    auto *rootItem = new QTreeWidgetItem(
        m_objectTree,
        {QString::fromUtf8("Aircraft ") + (doc.title.isEmpty() ? QStringLiteral("—") : doc.title),
         QString::fromUtf8("根对象")});
    rootItem->setExpanded(true);
    for (int i = 0; i < doc.systems.size(); ++i)
        new QTreeWidgetItem(rootItem, {doc.systems[i].object, doc.systems[i].scheme});

    const AircraftKpis k = computeKpis(doc);
    replaceHost(m_semKpiHost, makeKpis({
        {QString::fromUtf8("对象实例"), QString::number(k.objectCount), QString::fromUtf8("个")},
        {QString::fromUtf8("对象类型"), QString::number(k.typeCount), QString::fromUtf8("类")},
        {QString::fromUtf8("几何参数"), QString::number(k.parameterCount), QString::fromUtf8("个")},
        {QString::fromUtf8("完整度"), QString::number(k.completeness), QStringLiteral("%")}
    }));
    replaceHost(m_configKpiHost, makeKpis({
        {QString::fromUtf8("构型类别"), k.categoryText, QString()},
        {QString::fromUtf8("系统对象"), QString::number(k.systemCount), QString::fromUtf8("个")},
        {QString::fromUtf8("对象类型"), QString::number(k.typeCount), QString::fromUtf8("类")},
        {QString::fromUtf8("完整度"), QString::number(k.completeness), QStringLiteral("%")}
    }));
    replaceHost(m_geoKpiHost, makeKpis({
        {QString::fromUtf8("几何参数"), QString::number(k.parameterCount), QString::fromUtf8("个")},
        {QString::fromUtf8("设计变量"), QString::number(k.designVarCount), QString::fromUtf8("个")},
        {QString::fromUtf8("驱动量"), QString::number(k.drivenCount), QString::fromUtf8("个")},
        {QString::fromUtf8("完整度"), QString::number(k.completeness), QStringLiteral("%")}
    }));

    // 语义页：模型规范 + 语义检查 + 引用与接口（均由方案数据派生）。
    replaceHost(m_modelSpecHost, makeSummary({
        {QStringLiteral("Schema"), orDash(doc.semantics.schema)},
        {QString::fromUtf8("命名空间"), orDash(doc.semantics.namespaceStr)},
        {QString::fromUtf8("引用策略"), orDash(doc.semantics.referenceStrategy)},
        {QString::fromUtf8("交换格式"), orDash(doc.semantics.exchangeFormat)}
    }));

    int missingObj = 0;
    int unknownParam = 0;
    for (int i = 0; i < doc.systems.size(); ++i) {
        if (doc.systems[i].object.isEmpty())
            ++missingObj;
    }
    for (int i = 0; i < doc.parameters.size(); ++i) {
        if (!doc.parameters[i].valueKnown)
            ++unknownParam;
    }
    QStringList semChecks;
    semChecks.append(missingObj == 0
                         ? QString::fromUtf8("对象类型全部有效")
                         : QString::fromUtf8("%1 个对象缺少名称").arg(missingObj));
    semChecks.append((!doc.semantics.lengthUnit.isEmpty() && !doc.semantics.massUnit.isEmpty())
                         ? QString::fromUtf8("长度/质量单位已定义")
                         : QString::fromUtf8("单位定义不完整"));
    semChecks.append(!doc.semantics.coordinateSystem.isEmpty()
                         ? QString::fromUtf8("坐标系引用一致")
                         : QString::fromUtf8("坐标系语义缺失"));
    semChecks.append(unknownParam == 0
                         ? QString::fromUtf8("几何参数数值齐全")
                         : QString::fromUtf8("%1 个参数缺少数值").arg(unknownParam));
    replaceHost(m_semCheckHost, makeBulletList(semChecks));

    QVector<QStringList> ifaceRows;
    QVector<int> ifaceWarn;
    for (int i = 0; i < doc.systems.size(); ++i) {
        ifaceRows.append({QString::fromUtf8("IF-%1").arg(i + 1, 3, 10, QLatin1Char('0')),
                          doc.systems[i].object, doc.systems[i].scheme,
                          doc.systems[i].mounting, doc.systems[i].status});
    }
    replaceHost(m_interfaceHost, makeTable(
        {QString::fromUtf8("接口编号"), QString::fromUtf8("对象关系"), QString::fromUtf8("类型"),
         QString::fromUtf8("交换内容"), QString::fromUtf8("状态")},
        ifaceRows, TableOptions{}));

    // 配置页：完整性检查 + 待处理（派生）。
    int objectFilled = 0;
    for (int i = 0; i < doc.systems.size(); ++i) {
        if (!doc.systems[i].object.isEmpty() && !doc.systems[i].status.contains(QString::fromUtf8("待")))
            ++objectFilled;
    }
    const int objectPercent = doc.systems.isEmpty() ? 0 : objectFilled * 100 / doc.systems.size();
    const int paramPercent = doc.parameters.isEmpty()
                                 ? 0
                                 : (doc.parameters.size() - unknownParam) * 100 / doc.parameters.size();
    replaceHost(m_integrityHost, makeProgressPanel({
        {QString::fromUtf8("对象完整"), objectPercent},
        {QString::fromUtf8("参数完整"), paramPercent},
        {QString::fromUtf8("总体完整度"), k.completeness}
    }));

    QStringList todos;
    for (int i = 0; i < doc.systems.size(); ++i) {
        if (doc.systems[i].status.contains(QString::fromUtf8("待")))
            todos.append(QString::fromUtf8("%1 状态待确认").arg(doc.systems[i].object));
    }
    for (int i = 0; i < doc.parameters.size(); ++i) {
        if (!doc.parameters[i].valueKnown)
            todos.append(QString::fromUtf8("%1 数值未填写").arg(doc.parameters[i].name));
    }
    if (todos.isEmpty())
        todos.append(QString::fromUtf8("暂无待处理项"));
    replaceHost(m_todoHost, makeBulletList(todos));

    // 几何页：几何约束与驱动关系（由公式/自动驱动的参数派生）。
    QVector<QStringList> relRows;
    for (int i = 0; i < doc.parameters.size(); ++i) {
        const AcParameter &p = doc.parameters[i];
        if (p.driveType.contains(QString::fromUtf8("公式")) || p.driveType.contains(QString::fromUtf8("自动"))) {
            const QString input = p.note.isEmpty() ? p.symbol : p.note;
            relRows.append({p.name, input, p.symbol, p.driveType, QString::fromUtf8("有效")});
        }
    }
    replaceHost(m_relationHost, makeTable(
        {QString::fromUtf8("关系"), QString::fromUtf8("输入"), QString::fromUtf8("输出"),
         QString::fromUtf8("方式"), QString::fromUtf8("状态")},
        relRows, TableOptions{}));

    m_updating = false;
    setReadOnly(readOnly);
    if (m_systemTable->rowCount() > 0)
        m_systemTable->selectRow(0);
    if (m_paramTable->rowCount() > 0)
        m_paramTable->selectRow(0);
}

void DefinitionPage::snapshotSemantics(AcSemantics *semantics, QString *title) const
{
    *title = m_title->text();
    semantics->schema = m_schema->text();
    semantics->namespaceStr = m_namespace->text();
    semantics->referenceStrategy = m_reference->text();
    semantics->exchangeFormat = m_exchange->text();
    semantics->lengthUnit = m_lengthUnit->text();
    semantics->massUnit = m_massUnit->text();
    semantics->coordinateSystem = m_coordSystem->text();
    semantics->origin = m_origin->text();
}

void DefinitionPage::snapshotConfiguration(AcConfiguration *configuration,
                                           QVector<AcSystemItem> *systems) const
{
    configuration->category = m_category->currentText();
    configuration->wingLayout = m_wingLayout->currentText();
    configuration->tailConfig = m_tailConfig->currentText();
    configuration->engineArrangement = m_engineArrangement->currentText();
    configuration->gearType = m_gearType->currentText();
    configuration->cabinLayout = m_cabinLayout->currentText();
    configuration->primaryMaterial = m_primaryMaterial->currentText();
    configuration->connectionStrategy = m_connectionStrategy->text();
    configuration->symmetry = m_symmetry->text();

    *systems = m_shown.systems;
    const QVector<QStringList> rows = tableAllRows(m_systemTable);
    QVector<AcSystemItem> result;
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_systemTable, r).toString();
        AcSystemItem item;
        for (int i = 0; i < systems->size(); ++i) {
            if ((*systems)[i].id == id) {
                item = (*systems)[i];
                break;
            }
        }
        item.id = rows[r].value(0).isEmpty() ? id : rows[r].value(0);
        item.object = rows[r].value(1);
        item.scheme = rows[r].value(2);
        item.mounting = rows[r].value(3);
        item.material = rows[r].value(4);
        item.status = rows[r].value(5);
        result.append(item);
    }
    *systems = result;
}

void DefinitionPage::snapshotParameters(QVector<AcParameter> *parameters) const
{
    const QVector<QStringList> rows = tableAllRows(m_paramTable);
    QVector<AcParameter> result;
    for (int r = 0; r < rows.size(); ++r) {
        const QString id = tableRowId(m_paramTable, r).toString();
        AcParameter p;
        for (int i = 0; i < m_shown.parameters.size(); ++i) {
            if (m_shown.parameters[i].id == id) {
                p = m_shown.parameters[i];
                break;
            }
        }
        p.id = rows[r].value(0).isEmpty() ? id : rows[r].value(0);
        p.name = rows[r].value(1);
        p.symbol = rows[r].value(2);
        bool ok = false;
        const double value = parseNumberText(rows[r].value(3), &ok);
        p.value = ok ? value : 0;
        p.valueKnown = ok;
        p.unit = rows[r].value(4);
        p.driveType = rows[r].value(5);
        p.note = rows[r].value(6);
        result.append(p);
    }
    *parameters = result;
}
