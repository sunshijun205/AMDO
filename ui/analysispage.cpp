#include "analysispage.h"

#include "controller/analysispresenter.h"
#include "model/aircraftstore.h"
#include "model/analysiscatalogs.h"
#include "model/analysisstore.h"
#include "model/srdstore.h"
#include "model/srdtypes.h"
#include "service/aircraftcpacsservice.h"
#include "service/analysiscomputeservice.h"
#include "service/analysisdocumentservice.h"
#include "uihelpers.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>

AnalysisPage::AnalysisPage(QWidget *parent)
    : QWidget(parent)
{
    m_aircraftStore.reset(new AircraftStore);
    m_srdStore.reset(new SrdStore);
    buildUi();
    reloadReferenceOptions();

    m_store.reset(new AnalysisStore);
    m_document.reset(new AnalysisDocumentService(m_store.get()));
    m_compute.reset(new AnalysisComputeService(m_aircraftStore.get(), m_store.get(), m_srdStore.get()));
    m_presenter = new AnalysisPresenter(this, m_document.get(), m_compute.get(), this);
}

AnalysisPage::~AnalysisPage() = default;

QWidget *AnalysisPage::makeFieldWidget(const AnalysisField &field, const QString &key)
{
    if (field.type == QLatin1String("check")) {
        QCheckBox *box = makeCheck(field.label, field.defaultValue == QString::fromUtf8("启用"));
        m_editors.insert(key, box);
        return box;
    }
    if (field.type == QLatin1String("select")) {
        QWidget *w = makeSelectField(field.label, field.defaultValue, field.options);
        if (QComboBox *combo = w->findChild<QComboBox *>())
            m_editors.insert(key, combo);
        return w;
    }
    QWidget *w = makeField(field.label, field.defaultValue, field.unit);
    if (QLineEdit *edit = w->findChild<QLineEdit *>())
        m_editors.insert(key, edit);
    return w;
}

QWidget *AnalysisPage::buildDomainPage(const AnalysisDomain &domain)
{
    auto *root = new QWidget;
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(20, 18, 20, 22);
    lay->setSpacing(14);
    lay->addWidget(makeHeading(domain.title, domain.subtitle,
                               QString::fromUtf8("配置模板"), domain.templates));

    auto *body = new QWidget;
    auto *hl = new QHBoxLayout(body);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(14);

    auto *sections = new QWidget;
    auto *grid = new QGridLayout(sections);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(12);
    for (int i = 0; i < domain.sections.size(); ++i) {
        const AnalysisSection &s = domain.sections[i];
        auto *sec = new QFrame;
        sec->setObjectName(QStringLiteral("Section"));
        auto *vl = new QVBoxLayout(sec);
        vl->setContentsMargins(12, 12, 12, 12);
        vl->setSpacing(10);
        auto *titleRow = new QWidget;
        auto *th = new QHBoxLayout(titleRow);
        th->setContentsMargins(0, 0, 0, 0);
        th->setSpacing(7);
        auto *bar = new QFrame;
        bar->setFixedSize(3, 15);
        bar->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 2px;"));
        auto *h = new QLabel(s.title);
        h->setObjectName(QStringLiteral("SectionTitle"));
        th->addWidget(bar);
        th->addWidget(h);
        th->addStretch();
        vl->addWidget(titleRow);

        auto *fields = new QWidget;
        auto *fg = new QGridLayout(fields);
        fg->setContentsMargins(0, 0, 0, 0);
        fg->setHorizontalSpacing(10);
        fg->setVerticalSpacing(8);
        for (int fi = 0; fi < s.fields.size(); ++fi) {
            const QString key = analysisFieldKey(domain.id, s.title, s.fields[fi].label);
            fg->addWidget(makeFieldWidget(s.fields[fi], key), fi / 2, fi % 2);
        }
        vl->addWidget(fields);
        grid->addWidget(sec, i / 2, i % 2);
    }

    auto *aside = new QWidget;
    auto *al = new QVBoxLayout(aside);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(12);
    aside->setFixedWidth(250);
    auto *sum = makePanel();
    sum->layout()->addWidget(makePanelTitle(QString::fromUtf8("当前配置摘要")));
    QVector<SummaryItem> summary;
    for (int i = 0; i < domain.summary.size(); ++i) {
        SummaryItem item;
        item.key = domain.summary[i].key;
        item.value = domain.summary[i].value;
        summary.append(item);
    }
    sum->layout()->addWidget(makeSummary(summary));
    al->addWidget(sum);
    auto *chk = makePanel();
    chk->layout()->addWidget(makePanelTitle(QString::fromUtf8("运行前检查")));
    chk->layout()->addWidget(makeBulletList(domain.checks));
    al->addWidget(chk);
    al->addStretch();

    hl->addWidget(sections, 1);
    hl->addWidget(aside);
    lay->addWidget(body);

    auto *run = new QFrame;
    run->setObjectName(QStringLiteral("RunBar"));
    auto *rl = new QHBoxLayout(run);
    rl->setContentsMargins(12, 10, 12, 10);
    auto *dot = new QFrame;
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 4px;"));
    auto *note = new QLabel(QString::fromUtf8("跨学科公共变量从项目基线继承；保存后写入本机分析集草稿。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    auto *saveBtn = makeButton(QString::fromUtf8("保存分析集"), true);
    connect(saveBtn, &QPushButton::clicked, this, &AnalysisPage::saveRequested);
    auto *checkBtn = makeButton(QString::fromUtf8("校验配置"));
    connect(checkBtn, &QPushButton::clicked, this, &AnalysisPage::validateRequested);
    rl->addWidget(dot);
    rl->addWidget(note);
    rl->addStretch();
    rl->addWidget(checkBtn);
    rl->addWidget(saveBtn);
    lay->addWidget(run);
    return root;
}

static QPushButton *makeDomainTab(int index, const QString &name, bool active)
{
    auto *btn = new QPushButton;
    btn->setObjectName(QStringLiteral("DomainTab"));
    btn->setCheckable(true);
    btn->setChecked(active);
    btn->setCursor(Qt::PointingHandCursor);
    auto *hl = new QHBoxLayout(btn);
    hl->setContentsMargins(4, 4, 8, 4);
    hl->setSpacing(7);
    auto *num = new QLabel(QString::number(index));
    num->setObjectName(active ? QStringLiteral("DomainNumActive") : QStringLiteral("DomainNum"));
    num->setProperty("role", QStringLiteral("num"));
    num->setFixedSize(25, 25);
    num->setAlignment(Qt::AlignCenter);
    num->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *lb = new QLabel(name);
    lb->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto *dot = new QFrame;
    dot->setFixedSize(8, 8);
    dot->setStyleSheet(QStringLiteral("background: #0c9b88; border-radius: 4px;"));
    dot->setAttribute(Qt::WA_TransparentForMouseEvents);
    hl->addWidget(num);
    hl->addWidget(lb, 1);
    hl->addWidget(dot);
    return btn;
}

void AnalysisPage::buildUi()
{
    const QVector<AnalysisDomain> domains = AnalysisCatalogs::domains();

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *header = new QFrame;
    header->setObjectName(QStringLiteral("RunBar"));
    auto *hb = new QHBoxLayout(header);
    hb->setContentsMargins(20, 10, 20, 10);
    hb->setSpacing(8);
    auto *verLabel = new QLabel(QString::fromUtf8("分析集版本"));
    verLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_baselineBox = new QComboBox;
    m_baselineBox->setMinimumWidth(220);
    auto *revLabel = new QLabel(QString::fromUtf8("关联飞机修订"));
    revLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_revisionBox = new QComboBox;
    m_revisionBox->setMinimumWidth(180);
    auto *srdLabel = new QLabel(QString::fromUtf8("关联设计需求"));
    srdLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_srdBox = new QComboBox;
    m_srdBox->setMinimumWidth(150);
    auto *condLabel = new QLabel(QString::fromUtf8("设计工况"));
    condLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_conditionBox = new QComboBox;
    m_conditionBox->setMinimumWidth(170);
    auto *caseLabel = new QLabel(QString::fromUtf8("关联用例"));
    caseLabel->setObjectName(QStringLiteral("FieldLabel"));
    m_caseBox = new QComboBox;
    m_caseBox->setMinimumWidth(120);
    hb->addWidget(verLabel);
    hb->addWidget(m_baselineBox);
    hb->addSpacing(12);
    hb->addWidget(revLabel);
    hb->addWidget(m_revisionBox);
    hb->addWidget(srdLabel);
    hb->addWidget(m_srdBox);
    hb->addWidget(condLabel);
    hb->addWidget(m_conditionBox);
    hb->addWidget(caseLabel);
    hb->addWidget(m_caseBox);
    hb->addStretch();
    m_runBtn = makeButton(QString::fromUtf8("运行学科计算"));
    m_copyDraftBtn = makeButton(QString::fromUtf8("另存为新草稿"));
    m_publishBtn = makeButton(QString::fromUtf8("发布分析集版本"), true);
    hb->addWidget(m_runBtn);
    hb->addWidget(m_copyDraftBtn);
    hb->addWidget(m_publishBtn);
    outer->addWidget(header);
    connect(m_runBtn, &QPushButton::clicked, this, &AnalysisPage::runRequested);
    connect(m_baselineBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onBaselineChanged);
    connect(m_revisionBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onReferenceChanged);
    connect(m_srdBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onSrdChanged);
    connect(m_conditionBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onReferenceChanged);
    connect(m_caseBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &AnalysisPage::onReferenceChanged);
    connect(m_copyDraftBtn, &QPushButton::clicked, this, &AnalysisPage::copyToDraftRequested);
    connect(m_publishBtn, &QPushButton::clicked, this, &AnalysisPage::publishRequested);

    auto *content = new QWidget;
    auto *rootLay = new QHBoxLayout(content);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    auto *side = new QFrame;
    side->setObjectName(QStringLiteral("SideBar"));
    side->setFixedWidth(238);
    auto *sl = new QVBoxLayout(side);
    sl->setContentsMargins(14, 18, 14, 18);
    sl->setSpacing(4);
    auto *sideTitle = new QLabel(QStringLiteral("ANALYSIS DOMAINS"));
    sideTitle->setObjectName(QStringLiteral("SideTitle"));
    sl->addWidget(sideTitle);

    auto *stack = new QStackedWidget;
    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < domains.size(); ++i) {
        auto *tab = makeDomainTab(i + 1, domains[i].name, i == 0);
        group->addButton(tab, i);
        sl->addWidget(tab);
        stack->addWidget(wrapScroll(buildDomainPage(domains[i])));
    }
    connect(group, &QButtonGroup::idClicked, this, [stack, group](int id) {
        stack->setCurrentIndex(id);
        const auto buttons = group->buttons();
        for (QAbstractButton *button : buttons) {
            auto *num = button->findChild<QLabel *>();
            if (!num || num->property("role").toString() != QLatin1String("num"))
                continue;
            const bool on = (group->id(button) == id);
            num->setObjectName(on ? QStringLiteral("DomainNumActive") : QStringLiteral("DomainNum"));
            num->style()->unpolish(num);
            num->style()->polish(num);
        }
    });
    sl->addStretch();
    auto *note = new QLabel(QString::fromUtf8("每个学科统一采用：分析方法 → 输入与工况 → 离散/模型 → 求解控制。跨学科公共变量从项目基线继承。"));
    note->setObjectName(QStringLiteral("SideNote"));
    note->setWordWrap(true);
    sl->addWidget(note);

    rootLay->addWidget(side);
    rootLay->addWidget(stack, 1);
    outer->addWidget(content, 1);

    m_status = new QLabel(QString::fromUtf8("就绪"));
    m_status->setObjectName(QStringLiteral("PageStatus"));
    m_status->setContentsMargins(20, 6, 20, 6);
    outer->addWidget(m_status);
}

void AnalysisPage::setDocument(const AnalysisDocument &doc)
{
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it) {
        if (!doc.values.contains(it.key()))
            continue;
        const QString value = doc.values.value(it.key());
        QWidget *w = it.value();
        if (auto *box = qobject_cast<QCheckBox *>(w)) {
            box->setChecked(value == QString::fromUtf8("启用"));
        } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
            int idx = combo->findText(value);
            if (idx < 0) {
                combo->insertItem(0, value);
                idx = 0;
            }
            combo->setCurrentIndex(idx);
        } else if (auto *edit = qobject_cast<QLineEdit *>(w)) {
            edit->setText(value);
        }
    }

    // 关联下拉：按文档的引用选中（编程更新，避免触发 referencesChanged）。
    m_updating = true;
    if (m_revisionBox) {
        int idx = m_revisionBox->findData(doc.sourceRevision);
        m_revisionBox->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    if (m_srdBox) {
        int idx = m_srdBox->findData(doc.sourceSrd);
        m_srdBox->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    if (m_caseBox) {
        int idx = m_caseBox->findData(doc.sourceCase);
        m_caseBox->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    m_updating = false;

    // 设计工况选项随文档的 SRD 关联重建，再按文档选中。
    reloadConditionOptions(doc.sourceSrd);
    m_updating = true;
    if (m_conditionBox) {
        int idx = m_conditionBox->findData(doc.sourceCondition);
        m_conditionBox->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    m_updating = false;
}

void AnalysisPage::setBaselines(const QVector<AnalysisBaselineInfo> &baselines, const QString &currentId)
{
    if (!m_baselineBox)
        return;
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

void AnalysisPage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it)
        it.value()->setEnabled(!readOnly);
    if (m_copyDraftBtn)
        m_copyDraftBtn->setEnabled(readOnly);
    if (m_publishBtn)
        m_publishBtn->setEnabled(!readOnly);
    if (m_revisionBox)
        m_revisionBox->setEnabled(!readOnly);
    if (m_srdBox)
        m_srdBox->setEnabled(!readOnly);
    if (m_conditionBox)
        m_conditionBox->setEnabled(!readOnly);
    if (m_caseBox)
        m_caseBox->setEnabled(!readOnly);
}

void AnalysisPage::onBaselineChanged(int)
{
    if (m_updating || !m_baselineBox)
        return;
    emit switchBaselineRequested(m_baselineBox->currentData().toString());
}

void AnalysisPage::reloadReferenceOptions()
{
    if (!m_revisionBox || !m_caseBox)
        return;
    m_updating = true;

    m_revisionBox->clear();
    m_revisionBox->addItem(QString::fromUtf8("（未关联）"), QString());
    if (m_aircraftStore) {
        QVector<AircraftBaselineInfo> baselines;
        QString detail;
        if (m_aircraftStore->listBaselines(&baselines, &detail)) {
            for (int i = 0; i < baselines.size(); ++i) {
                const QString revId = AircraftCpacsService::revisionId(baselines[i].version);
                QString label = revId;
                if (!baselines[i].id.isEmpty())
                    label += QStringLiteral(" · ") + baselines[i].id;
                if (!baselines[i].title.isEmpty())
                    label += QStringLiteral(" · ") + baselines[i].title;
                m_revisionBox->addItem(label, revId);
            }
        }
    }

    m_srdBox->clear();
    m_srdBox->addItem(QString::fromUtf8("（未关联）"), QString());
    if (m_srdStore) {
        QVector<SrdBaselineInfo> srdBaselines;
        QString srdDetail;
        if (m_srdStore->listBaselines(&srdBaselines, &srdDetail)) {
            for (int i = 0; i < srdBaselines.size(); ++i) {
                QString label = srdBaselines[i].id;
                if (!srdBaselines[i].title.isEmpty())
                    label += QStringLiteral(" · ") + srdBaselines[i].title;
                m_srdBox->addItem(label, srdBaselines[i].id);
            }
        }
    }

    m_caseBox->clear();
    m_caseBox->addItem(QString::fromUtf8("（不关联用例）"), QString());
    if (m_aircraftStore) {
        const QStringList caseIds = m_aircraftStore->listCaseIds();
        for (int i = 0; i < caseIds.size(); ++i)
            m_caseBox->addItem(caseIds[i], caseIds[i]);
    }

    m_updating = false;

    // 设计工况选项依赖当前 SRD 选择；先按当前 SRD 填一次。
    reloadConditionOptions(m_srdBox->currentData().toString());
}

// 依据关联的 SRD 基线，填充其飞行包线点作为可选设计工况。
void AnalysisPage::reloadConditionOptions(const QString &srdId)
{
    if (!m_conditionBox)
        return;
    m_updating = true;
    m_conditionBox->clear();
    m_conditionBox->addItem(QString::fromUtf8("（用分析集工况）"), QString());
    if (!srdId.isEmpty() && m_srdStore) {
        SrdDocument srd;
        QString detail;
        if (m_srdStore->loadBaseline(srdId, &srd, &detail)) {
            for (int i = 0; i < srd.envelopePoints.size(); ++i) {
                const SrdEnvelopePoint &ep = srd.envelopePoints[i];
                const QString label = QString::fromUtf8("包线点%1  Ma %2 / %3 km")
                                          .arg(i + 1)
                                          .arg(ep.mach, 0, 'g', 3)
                                          .arg(ep.altitudeKm, 0, 'g', 4);
                m_conditionBox->addItem(label, QString::number(i));
            }
        }
    }
    m_updating = false;
}

void AnalysisPage::onSrdChanged(int)
{
    if (m_updating || !m_srdBox)
        return;
    // SRD 变了：重建设计工况选项（默认回到“用分析集工况”），再统一发关联变更。
    reloadConditionOptions(m_srdBox->currentData().toString());
    onReferenceChanged(0);
}

void AnalysisPage::onReferenceChanged(int)
{
    if (m_updating || !m_revisionBox || !m_caseBox || !m_srdBox || !m_conditionBox)
        return;
    emit referencesChanged(m_revisionBox->currentData().toString(),
                           m_caseBox->currentData().toString(),
                           m_srdBox->currentData().toString(),
                           m_conditionBox->currentData().toString());
}

void AnalysisPage::snapshot(QHash<QString, QString> *values) const
{
    values->clear();
    for (auto it = m_editors.constBegin(); it != m_editors.constEnd(); ++it) {
        QWidget *w = it.value();
        if (auto *box = qobject_cast<QCheckBox *>(w))
            values->insert(it.key(), box->isChecked() ? QString::fromUtf8("启用") : QString::fromUtf8("关闭"));
        else if (auto *combo = qobject_cast<QComboBox *>(w))
            values->insert(it.key(), combo->currentText());
        else if (auto *edit = qobject_cast<QLineEdit *>(w))
            values->insert(it.key(), edit->text());
    }
}

void AnalysisPage::requestSaveAll()
{
    emit saveRequested();
}

void AnalysisPage::requestValidate()
{
    emit validateRequested();
}

void AnalysisPage::setStatus(const QString &text, bool isError)
{
    if (!m_status)
        return;
    m_status->setText(text);
    m_status->setStyleSheet(isError ? QStringLiteral("color: #b46b22;") : QString());
}

void AnalysisPage::showError(const QString &message)
{
    QMessageBox::warning(this, QString::fromUtf8("学科分析"), message);
}

void AnalysisPage::reportValidation(const QStringList &issues)
{
    if (issues.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("配置校验"),
                                 QString::fromUtf8("配置校验通过，未发现待处理项。"));
        return;
    }
    QString text = QString::fromUtf8("发现以下待处理项：\n\n");
    for (int i = 0; i < issues.size(); ++i)
        text += QString::fromUtf8("• ") + issues[i] + QLatin1Char('\n');
    QMessageBox::warning(this, QString::fromUtf8("配置校验"), text);
}

void AnalysisPage::showRunResult(const AnalysisRunResult &result)
{
    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("学科运行计算结果"));
    dialog.resize(760, 560);
    auto *lay = new QVBoxLayout(&dialog);

    QString head = QString::fromUtf8("运行时间：%1").arg(result.runAt);
    head += QString::fromUtf8("　飞机修订：%1")
                .arg(result.sourceRevision.isEmpty() ? QString::fromUtf8("未关联")
                                                      : result.sourceRevision);
    if (result.sourceRevision.isEmpty() || !result.aircraftResolved)
        head += QString::fromUtf8("（几何相关项将缺失/占位）");
    auto *headLabel = new QLabel(head);
    headLabel->setWordWrap(true);
    lay->addWidget(headLabel);

    auto *note = new QLabel(QString::fromUtf8(
        "说明：保真度为 MOCK 的为占位假值，仅打通链路，后期细化各学科计算时替换。"));
    note->setObjectName(QStringLiteral("NoteLabel"));
    note->setWordWrap(true);
    lay->addWidget(note);

    auto *tree = new QTreeWidget;
    tree->setColumnCount(4);
    tree->setHeaderLabels({QString::fromUtf8("项目"), QString::fromUtf8("数值"),
                           QString::fromUtf8("保真度"), QString::fromUtf8("说明")});
    tree->header()->setStretchLastSection(true);
    tree->setColumnWidth(0, 220);
    tree->setColumnWidth(1, 140);
    tree->setColumnWidth(2, 90);
    for (int i = 0; i < result.disciplines.size(); ++i) {
        const AnalysisDisciplineResult &d = result.disciplines[i];
        auto *top = new QTreeWidgetItem(tree, {d.domainName,
                                               QString(), d.status, QString()});
        top->setExpanded(true);
        for (int j = 0; j < d.items.size(); ++j) {
            const AnalysisResultItem &it = d.items[j];
            const QString valueText = it.valueKnown
                ? (QString::number(it.value, 'g', 6)
                   + (it.unit.isEmpty() ? QString() : QLatin1Char(' ') + it.unit))
                : QString::fromUtf8("—");
            new QTreeWidgetItem(top, {it.label, valueText, it.fidelity, it.note});
        }
    }
    lay->addWidget(tree, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    lay->addWidget(buttons);

    dialog.exec();
}
