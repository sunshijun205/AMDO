#include "designpage.h"

#include "chartwidgets.h"
#include "controller/designpresenter.h"
#include "model/aircraftstore.h"
#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "model/srdtypes.h"
#include "service/aircraftcpacsservice.h"
#include "service/aircraftdocumentservice.h"
#include "service/analysiscomputeservice.h"
#include "service/evaluationservice.h"
#include "service/studyservice.h"
#include "uihelpers.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

static QWidget *makeHostWidget()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    return w;
}

static void setHostContent(QWidget *host, QWidget *content)
{
    if (!host || !host->layout())
        return;
    QLayout *l = host->layout();
    QLayoutItem *item = nullptr;
    while ((item = l->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    l->addWidget(content);
}

static QString numText(double v)
{
    return QString::number(v, 'g', 6);
}

// ---- 设计变量与约束（暂为原型静态展示）------------------------------------
// TODO：与「设计空间探索」的真实变量表打通（当前探索页自带精简变量编辑）。
static QWidget *variablesPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("设计变量"), QStringLiteral("18"), QString::fromUtf8("个")},
        {QString::fromUtf8("约束"), QStringLiteral("12"), QString::fromUtf8("项")},
        {QString::fromUtf8("分析工况"), QStringLiteral("7"), QString::fromUtf8("组")},
        {QString::fromUtf8("预计组合规模"), QStringLiteral("3.2×10⁸"), QString()}
    }));

    auto *grid = new QWidget;
    auto *hl = new QHBoxLayout(grid);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(12);

    auto *stack = new QWidget;
    auto *vl = new QVBoxLayout(stack);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    TableOptions varOpt;
    varOpt.firstColumnCheck = true;
    varOpt.chipColumns = {2};
    auto *vars = makePanel();
    vars->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计变量与范围"), QString::fromUtf8("原型示意"),
                                             QString::fromUtf8("真实可运行的精简变量在「设计空间探索」页")));
    vars->layout()->addWidget(makeTable(
        {QString::fromUtf8("启用"), QString::fromUtf8("变量"), QString::fromUtf8("学科"), QString::fromUtf8("基准值"),
         QString::fromUtf8("下界"), QString::fromUtf8("上界"), QString::fromUtf8("单位"), QString::fromUtf8("类型")},
        {
            {QString(), QString::fromUtf8("翼面积 S"), QString::fromUtf8("气动"), QStringLiteral("124.0"), QStringLiteral("110"), QStringLiteral("145"), QString::fromUtf8("m²"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("展弦比 AR"), QString::fromUtf8("气动"), QStringLiteral("9.4"), QStringLiteral("8.0"), QStringLiteral("11.5"), QString::fromUtf8("—"), QString::fromUtf8("连续")},
            {QString(), QString::fromUtf8("后掠角 Λ25"), QString::fromUtf8("气动"), QStringLiteral("25.0"), QStringLiteral("18"), QStringLiteral("32"), QStringLiteral("deg"), QString::fromUtf8("连续")}
        }, varOpt));
    vl->addWidget(vars);

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(270);
    auto *type = makePanel();
    type->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计类型")));
    type->layout()->addWidget(makeMiniFields({
        makeSelectField(QString::fromUtf8("模式"), QString::fromUtf8("探索与优化"), {QString::fromUtf8("仅探索"), QString::fromUtf8("仅优化")}),
        makeSelectField(QString::fromUtf8("变量集"), QString::fromUtf8("MDO 基准变量集"), {QString::fromUtf8("气动专项")})
    }));
    al->addWidget(type);
    auto *note = new QLabel(QString::fromUtf8("原型页。真实设计空间探索请见「设计空间探索」标签。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    al->addWidget(note);
    al->addStretch();

    hl->addWidget(stack, 1);
    hl->addWidget(aside);
    lay->addWidget(grid);
    return root;
}

// ---- 优化设计（专业优化算法暂为原型；真实闭环用「设计空间探索」的网格取最优）----
// TODO：NSGA-II/差分进化/贝叶斯等按 IStudyOptimizer 接口接入（需外部专业库）。
static QWidget *optimizationPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QString::fromUtf8("优化目标"), QStringLiteral("3"), QString::fromUtf8("项")},
        {QString::fromUtf8("算法"), QStringLiteral("NSGA-II"), QString()},
        {QString::fromUtf8("种群规模"), QStringLiteral("80"), QString()},
        {QString::fromUtf8("最大评估"), QStringLiteral("4,800"), QString::fromUtf8("次")}
    }));

    auto *obj = makePanel();
    obj->layout()->addWidget(makePanelTitle(QString::fromUtf8("优化目标设置"), QString::fromUtf8("原型示意 · 高级优化算法待接入")));
    TableOptions opt;
    opt.firstColumnCheck = true;
    obj->layout()->addWidget(makeTable(
        {QString::fromUtf8("启用"), QString::fromUtf8("目标响应"), QString::fromUtf8("方向"), QString::fromUtf8("单位"), QString::fromUtf8("优先权重")},
        {
            {QString(), QString::fromUtf8("任务燃油"), QString::fromUtf8("最小化"), QStringLiteral("kg"), QStringLiteral("1.00")},
            {QString(), QString::fromUtf8("巡航升阻比"), QString::fromUtf8("最大化"), QString::fromUtf8("—"), QStringLiteral("0.55")}
        }, opt));
    lay->addWidget(obj);
    auto *note = new QLabel(QString::fromUtf8(
        "高级优化算法(NSGA-II/梯度/MDO)需外部专业库，暂按 IStudyOptimizer 接口预留（MOCK）。"
        "当前真实闭环：在「设计空间探索」网格采样并取最优。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    lay->addWidget(note);
    lay->addStretch();
    return root;
}

// ---- MDO 求解引擎（需 OpenMDAO 等外部数值后端，暂为原型/MOCK）--------------
static QWidget *mdoPage(QWidget *parent)
{
    auto *root = new QWidget(parent);
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);
    lay->addWidget(makeKpis({
        {QStringLiteral("MDO 架构"), QStringLiteral("MDF"), QString()},
        {QString::fromUtf8("耦合学科"), QStringLiteral("6"), QString::fromUtf8("个")},
        {QString::fromUtf8("耦合变量"), QStringLiteral("9"), QString::fromUtf8("个")},
        {QString::fromUtf8("总导数方式"), QString::fromUtf8("混合"), QString()}
    }));
    auto *prob = makePanel();
    prob->layout()->addWidget(makePanelTitle(QString::fromUtf8("优化问题自动构建"), QString::fromUtf8("原型示意")));
    prob->layout()->addWidget(makeMiniFields({
        makeSelectField(QStringLiteral("MDO 架构"), QStringLiteral("MDF"), {QStringLiteral("IDF"), QString::fromUtf8("协同优化 CO")}),
        makeSelectField(QString::fromUtf8("优化驱动器"), QStringLiteral("SLSQP"), {QStringLiteral("IPOPT"), QStringLiteral("NSGA-II")})
    }, 2));
    lay->addWidget(prob);
    auto *note = new QLabel(QString::fromUtf8(
        "MDO 数值后端(OpenMDAO/伴随导数/耦合求解)属外部专业库，超当前纯前端原型范围，暂为占位。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    lay->addWidget(note);
    lay->addStretch();
    return root;
}

QWidget *DesignPage::buildExplorationPage()
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(12);

    // 优化基准：草稿 / 已发布分析集版本（其 sourceRevision 决定被优化的飞机）。
    auto *baseBar = new QWidget;
    auto *bl = new QHBoxLayout(baseBar);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);
    auto *baseLabel = new QLabel(QString::fromUtf8("优化基准"));
    baseLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_baseBox = new QComboBox;
    m_baseBox->setMinimumWidth(240);
    bl->addWidget(baseLabel);
    bl->addWidget(m_baseBox);
    bl->addStretch();
    lay->addWidget(baseBar);
    connect(m_baseBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int) { if (!m_updating) reloadConstraints(); });

    // 设计变量编辑（精简：S_ref、b —— 经 S/b→AR→L/D 真实影响气动指标）。
    auto *varPanel = makePanel();
    varPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计变量与范围"),
                                                 QString::fromUtf8("网格采样 · 变量真实影响 AR/L-D")));
    auto *vg = new QWidget;
    auto *grid = new QGridLayout(vg);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    auto *h0 = new QLabel(QString::fromUtf8("变量"));
    auto *h1 = new QLabel(QString::fromUtf8("下界"));
    auto *h2 = new QLabel(QString::fromUtf8("上界"));
    auto *h3 = new QLabel(QString::fromUtf8("步数"));
    h0->setObjectName(QStringLiteral("FieldLabel"));
    h1->setObjectName(QStringLiteral("FieldLabel"));
    h2->setObjectName(QStringLiteral("FieldLabel"));
    h3->setObjectName(QStringLiteral("FieldLabel"));
    grid->addWidget(h0, 0, 0);
    grid->addWidget(h1, 0, 1);
    grid->addWidget(h2, 0, 2);
    grid->addWidget(h3, 0, 3);

    m_sMin = makeInput(QStringLiteral("110"));
    m_sMax = makeInput(QStringLiteral("140"));
    m_sSteps = makeInput(QStringLiteral("4"));
    grid->addWidget(new QLabel(QString::fromUtf8("机翼面积 S (m²)")), 1, 0);
    grid->addWidget(m_sMin, 1, 1);
    grid->addWidget(m_sMax, 1, 2);
    grid->addWidget(m_sSteps, 1, 3);

    m_bMin = makeInput(QStringLiteral("30"));
    m_bMax = makeInput(QStringLiteral("37"));
    m_bSteps = makeInput(QStringLiteral("4"));
    grid->addWidget(new QLabel(QString::fromUtf8("机翼展长 b (m)")), 2, 0);
    grid->addWidget(m_bMin, 2, 1);
    grid->addWidget(m_bMax, 2, 2);
    grid->addWidget(m_bSteps, 2, 3);
    varPanel->layout()->addWidget(vg);

    auto *runBar = new QWidget;
    auto *rl = new QHBoxLayout(runBar);
    rl->setContentsMargins(0, 0, 0, 0);
    auto *hint = new QLabel(QString::fromUtf8("总采样点 = 各变量步数之积（上限 500）。基准方案取当前分析集草稿。"));
    hint->setObjectName(QStringLiteral("NoteLabel"));
    hint->setWordWrap(true);
    auto *runBtn = makeButton(QString::fromUtf8("运行探索"), true);
    connect(runBtn, &QPushButton::clicked, this, &DesignPage::exploreRequested);
    rl->addWidget(hint, 1);
    rl->addWidget(runBtn);
    varPanel->layout()->addWidget(runBar);
    lay->addWidget(varPanel);

    // 设计约束：来自关联 SRD 的需求（探索按这些约束判可行性）。
    auto *conPanel = makePanel();
    conPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计约束"),
                                                 QString::fromUtf8("来自关联设计需求(SRD) · 探索按此判可行性")));
    m_constraintHost = makeHostWidget();
    conPanel->layout()->addWidget(m_constraintHost);
    lay->addWidget(conPanel);

    m_studyKpiHost = makeHostWidget();
    lay->addWidget(m_studyKpiHost);

    auto *resPanel = makePanel();
    resPanel->layout()->addWidget(makePanelTitle(QString::fromUtf8("设计点结果"),
                                                 QString::fromUtf8("逐点：覆盖 S/b → 分析 → 评价")));
    m_studyTableHost = makeHostWidget();
    resPanel->layout()->addWidget(m_studyTableHost);
    m_bestLabel = new QLabel(QString::fromUtf8("最优方案：—"));
    m_bestLabel->setWordWrap(true);
    resPanel->layout()->addWidget(m_bestLabel);
    auto *promoteBtn = makeButton(QString::fromUtf8("提升最优为飞机方案版本"));
    connect(promoteBtn, &QPushButton::clicked, this, &DesignPage::promoteRequested);
    resPanel->layout()->addWidget(promoteBtn);
    lay->addWidget(resPanel);

    m_status = new QLabel(QString::fromUtf8("就绪：设置变量范围后点「运行探索」。"));
    m_status->setObjectName(QStringLiteral("PageStatus"));
    lay->addWidget(m_status);

    // 初始占位。
    setHostContent(m_studyKpiHost, makeKpis({
        {QString::fromUtf8("采样点"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("可行点"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("最优评分"), QString::fromUtf8("—"), QString()},
        {QString::fromUtf8("最优点"), QString::fromUtf8("—"), QString()}
    }));
    setHostContent(m_studyTableHost, makeTable(
        {QString::fromUtf8("点#"), QString::fromUtf8("机翼面积 S"), QString::fromUtf8("机翼展长 b"),
         QStringLiteral("AR"), QStringLiteral("L/D"), QString::fromUtf8("评分"), QString::fromUtf8("可行性")},
        {}, TableOptions{}));
    return root;
}

QString DesignPage::baseObjectId() const
{
    return m_baseBox ? m_baseBox->currentData().toString() : QStringLiteral("draft");
}

void DesignPage::reloadObjectOptions()
{
    if (!m_baseBox || !m_analysisStore)
        return;
    m_updating = true;
    m_baseBox->clear();
    m_baseBox->addItem(QString::fromUtf8("草稿（可编辑）"), QStringLiteral("draft"));
    QVector<AnalysisBaselineInfo> baselines;
    QString detail;
    if (m_analysisStore->listBaselines(&baselines, &detail)) {
        for (int i = 0; i < baselines.size(); ++i) {
            QString label = baselines[i].id;
            if (baselines[i].version > 0)
                label += QString::fromUtf8("  v%1").arg(baselines[i].version);
            m_baseBox->addItem(label, baselines[i].id);
        }
    }
    m_updating = false;
}

void DesignPage::reloadConstraints()
{
    if (!m_constraintHost)
        return;

    QString srdId;
    if (m_analysisStore) {
        AnalysisDocument doc;
        QString detail;
        const QString objectId = baseObjectId();
        const bool ok = (objectId.isEmpty() || objectId == QLatin1String("draft"))
                            ? m_analysisStore->loadDraft(&doc, &detail)
                            : m_analysisStore->loadBaseline(objectId, &doc, &detail);
        if (ok)
            srdId = doc.sourceSrd;
    }

    QVector<QStringList> rows;
    if (!srdId.isEmpty() && m_srdStore) {
        SrdDocument srd;
        QString d2;
        if (m_srdStore->loadBaseline(srdId, &srd, &d2)) {
            for (int i = 0; i < srd.requirements.size(); ++i) {
                const SrdRequirement &rq = srd.requirements[i];
                QString bound = rq.relation;
                if (rq.boundKnown)
                    bound += QLatin1Char(' ') + numText(rq.boundValue);
                if (!rq.unit.isEmpty())
                    bound += QLatin1Char(' ') + rq.unit;
                rows.append({
                    rq.metricName.isEmpty() ? rq.metricId : rq.metricName,
                    bound,
                    rq.grade,
                    rq.domain
                });
            }
        }
    }

    if (rows.isEmpty()) {
        auto *lbl = new QLabel(srdId.isEmpty()
            ? QString::fromUtf8("未关联设计需求（在「学科分析」页关联设计需求后，这里显示约束）。")
            : QString::fromUtf8("关联的设计需求 %1 暂无约束条目。").arg(srdId));
        lbl->setObjectName(QStringLiteral("NoteLabel"));
        lbl->setWordWrap(true);
        setHostContent(m_constraintHost, lbl);
        return;
    }
    setHostContent(m_constraintHost, makeTable(
        {QString::fromUtf8("约束指标"), QString::fromUtf8("关系·限值"),
         QString::fromUtf8("等级"), QString::fromUtf8("学科")},
        rows, TableOptions{}));
}

StudyDefinition DesignPage::studyDefinition() const
{
    StudyDefinition def;
    def.sampling = QStringLiteral("grid");
    def.tolerancePercent = 0.5;

    auto readD = [](QLineEdit *e, double dflt) {
        bool ok = false;
        const double v = e ? e->text().toDouble(&ok) : dflt;
        return ok ? v : dflt;
    };
    auto readI = [](QLineEdit *e, int dflt) {
        bool ok = false;
        const int v = e ? e->text().toInt(&ok) : dflt;
        return (ok && v >= 1) ? v : dflt;
    };

    StudyVariable s;
    s.symbol = QStringLiteral("S_ref");
    s.name = QString::fromUtf8("机翼面积 S");
    s.unit = QString::fromUtf8("m²");
    s.minValue = readD(m_sMin, 110.0);
    s.maxValue = readD(m_sMax, 140.0);
    s.steps = readI(m_sSteps, 4);
    s.enabled = true;

    StudyVariable b;
    b.symbol = QStringLiteral("b");
    b.name = QString::fromUtf8("机翼展长 b");
    b.unit = QStringLiteral("m");
    b.minValue = readD(m_bMin, 30.0);
    b.maxValue = readD(m_bMax, 37.0);
    b.steps = readI(m_bSteps, 4);
    b.enabled = true;

    def.variables << s << b;
    return def;
}

void DesignPage::showStudyResult(const StudyResult &result)
{
    int feasible = 0;
    for (int i = 0; i < result.points.size(); ++i)
        if (result.points[i].feasibility == QString::fromUtf8("通过"))
            ++feasible;

    QString bestScore = QString::fromUtf8("—");
    QString bestIdxText = QString::fromUtf8("—");
    QString bestDesc = QString::fromUtf8("—");
    if (result.bestIndex >= 0 && result.bestIndex < result.points.size()) {
        const StudyPointResult &bp = result.points[result.bestIndex];
        if (bp.scoreKnown)
            bestScore = QString::number(bp.score, 'f', 1);
        bestIdxText = QString::fromUtf8("#%1").arg(bp.index);
        QStringList varParts;
        for (int i = 0; i < result.variables.size(); ++i) {
            const QString sym = result.variables[i].symbol;
            varParts << QString::fromUtf8("%1=%2").arg(sym).arg(numText(bp.variables.value(sym)));
        }
        bestDesc = QString::fromUtf8("点%1  %2  AR=%3  L/D=%4  评分=%5%  %6")
                       .arg(bestIdxText, varParts.join(QLatin1Char(' ')),
                            bp.arKnown ? numText(bp.ar) : QString::fromUtf8("—"),
                            bp.ldKnown ? numText(bp.ld) : QString::fromUtf8("—"),
                            bp.scoreKnown ? QString::number(bp.score, 'f', 1) : QString::fromUtf8("—"),
                            bp.feasibility);
    }

    setHostContent(m_studyKpiHost, makeKpis({
        {QString::fromUtf8("采样点"), QString::number(result.points.size()), QString::fromUtf8("个")},
        {QString::fromUtf8("可行点"), QString::number(feasible), QString::fromUtf8("个")},
        {QString::fromUtf8("最优评分"), bestScore, bestScore == QString::fromUtf8("—") ? QString() : QStringLiteral("%")},
        {QString::fromUtf8("最优点"), bestIdxText, QString()}
    }));

    // 结果表：点# + 各变量 + AR + L/D + 评分 + 可行性。
    QStringList headers;
    headers << QString::fromUtf8("点#");
    for (int i = 0; i < result.variables.size(); ++i)
        headers << result.variables[i].name;
    headers << QStringLiteral("AR") << QStringLiteral("L/D")
            << QString::fromUtf8("评分") << QString::fromUtf8("可行性");

    QVector<QStringList> rows;
    TableOptions opt;
    for (int i = 0; i < result.points.size(); ++i) {
        const StudyPointResult &p = result.points[i];
        QStringList row;
        row << QString::number(p.index);
        for (int v = 0; v < result.variables.size(); ++v)
            row << numText(p.variables.value(result.variables[v].symbol));
        row << (p.arKnown ? numText(p.ar) : QString::fromUtf8("—"));
        row << (p.ldKnown ? numText(p.ld) : QString::fromUtf8("—"));
        row << (p.scoreKnown ? QString::number(p.score, 'f', 1) + QStringLiteral("%") : QString::fromUtf8("—"));
        row << p.feasibility;
        rows.append(row);
        if (p.feasibility == QString::fromUtf8("违反"))
            opt.warnRows.append(i);
    }
    setHostContent(m_studyTableHost, makeTable(headers, rows, opt));

    if (m_bestLabel)
        m_bestLabel->setText(QString::fromUtf8("最优方案：") + bestDesc);

    // 刷新约束表，反映当前分析集关联的 SRD。
    reloadConstraints();
}

void DesignPage::setStatus(const QString &text, bool isError)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void DesignPage::showError(const QString &message)
{
    setStatus(message, true);
}

DesignPage::DesignPage(QWidget *parent)
    : QWidget(parent)
{
    m_aircraftStore.reset(new AircraftStore);
    m_aircraftCpacs.reset(new AircraftCpacsService(m_aircraftStore.get()));
    m_aircraftDoc.reset(new AircraftDocumentService(m_aircraftStore.get(), m_aircraftCpacs.get()));
    m_srdStore.reset(new SrdStore);
    m_analysisStore.reset(new AnalysisStore);
    m_compute.reset(new AnalysisComputeService(m_aircraftStore.get(), m_analysisStore.get(),
                                               m_srdStore.get()));
    m_evaluation.reset(new EvaluationService(m_compute.get(), m_srdStore.get(), m_analysisStore.get()));
    m_study.reset(new StudyService(m_compute.get(), m_evaluation.get(), m_analysisStore.get()));

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *body = new QWidget;
    auto *lay = new QVBoxLayout(body);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(
        QString::fromUtf8("方案优化"),
        QString::fromUtf8("设计空间探索：采样设计变量 → 逐点分析+评价 → 汇总最优；高级优化(NSGA/MDO)按接口预留"),
        QString(), {}));

    auto *tabs = new SubTabBar({
        {QStringLiteral("variables"), QString::fromUtf8("设计变量与约束")},
        {QStringLiteral("exploration"), QString::fromUtf8("设计空间探索")},
        {QStringLiteral("optimization"), QString::fromUtf8("优化设计")},
        {QStringLiteral("mdo"), QString::fromUtf8("MDO 求解引擎")}
    }, QStringLiteral("exploration"));
    lay->addWidget(tabs, 0, Qt::AlignLeft);

    auto *stack = new QStackedWidget;
    stack->addWidget(variablesPage(this));
    stack->addWidget(buildExplorationPage());
    stack->addWidget(optimizationPage(this));
    stack->addWidget(mdoPage(this));
    stack->setCurrentIndex(1); // 默认展示真实的“设计空间探索”
    lay->addWidget(stack, 1);
    connect(tabs, &SubTabBar::currentChanged, this, [stack](const QString &id) {
        if (id == QLatin1String("exploration")) stack->setCurrentIndex(1);
        else if (id == QLatin1String("optimization")) stack->setCurrentIndex(2);
        else if (id == QLatin1String("mdo")) stack->setCurrentIndex(3);
        else stack->setCurrentIndex(0);
    });
    outer->addWidget(wrapScroll(body));

    reloadObjectOptions();
    reloadConstraints();
    m_presenter = new DesignPresenter(this, m_analysisStore.get(), m_study.get(),
                                      m_aircraftStore.get(), m_aircraftDoc.get(), this);
}

DesignPage::~DesignPage() = default;
