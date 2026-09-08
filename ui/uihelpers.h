#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPair>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

struct KpiItem {
    QString label;
    QString value;
    QString unit;
};

struct SummaryItem {
    QString key;
    QString value;
    bool warn = false;
    bool good = false;
};

struct TableOptions {
    bool firstColumnCheck = false;
    bool firstChecksChecked = true;
    QVector<int> warnRows;
    QVector<int> chipColumns;
};

QPushButton *makeButton(const QString &text, bool primary = false, QWidget *parent = nullptr);
QComboBox *makeSelect(const QString &current, const QStringList &others = {}, QWidget *parent = nullptr);
QLineEdit *makeInput(const QString &value, QWidget *parent = nullptr);
QWidget *makeLabeled(const QString &label, QWidget *control, QWidget *parent = nullptr);
QWidget *makeField(const QString &label, const QString &value, const QString &unit = {}, QWidget *parent = nullptr);
QWidget *makeSelectField(const QString &label, const QString &current, const QStringList &others = {}, QWidget *parent = nullptr);
QCheckBox *makeCheck(const QString &label, bool checked = true, QWidget *parent = nullptr);
QFrame *makePanel(QWidget *parent = nullptr);
QWidget *makePanelTitle(const QString &title, const QString &meta = {}, const QString &subtitle = {}, QWidget *parent = nullptr);
QWidget *makeKpis(const QVector<KpiItem> &items, QWidget *parent = nullptr);
QWidget *makeSummary(const QVector<SummaryItem> &rows, QWidget *parent = nullptr);
QWidget *makeBulletList(const QStringList &items, QWidget *parent = nullptr);
QWidget *makeHeading(const QString &title, const QString &subtitle,
                     const QString &templateLabel, const QStringList &templateItems,
                     QWidget *parent = nullptr);
QWidget *makeMiniFields(const QList<QWidget *> &fields, int columns = 2, QWidget *parent = nullptr);
QWidget *makeProgressRow(const QString &label, const QString &value, int percent, QWidget *parent = nullptr);
QTableWidget *makeTable(const QStringList &headers, const QVector<QStringList> &rows,
                        const TableOptions &options = {}, QWidget *parent = nullptr);
QScrollArea *wrapScroll(QWidget *content, QWidget *parent = nullptr);
QWidget *makeStatusText(const QString &text, bool warn = false, QWidget *parent = nullptr);
QLabel *makeChip(const QString &text, bool neutral = false, QWidget *parent = nullptr);
QFrame *makeFlowNode(const QString &title, const QString &subtitle, const QString &state = {}, QWidget *parent = nullptr);
QLabel *makeFlowArrow(const QString &text = QString::fromUtf8("\xE2\x86\x92"), QWidget *parent = nullptr);
void wireDummyAction(QAbstractButton *button, QWidget *dialogParent);
bool isWarnStatus(const QString &text);
bool isGoodStatus(const QString &text);

class SubTabBar : public QFrame
{
    Q_OBJECT
public:
    explicit SubTabBar(const QVector<QPair<QString, QString>> &items,
                       const QString &activeId,
                       QWidget *parent = nullptr);
    void setActive(const QString &id);
    QString currentId() const;

signals:
    void currentChanged(const QString &id);

private:
    QString m_currentId;
};

class CandidateRow : public QFrame
{
    Q_OBJECT
public:
    CandidateRow(const QString &rank, const QString &name, const QString &desc,
                 const QString &score, QWidget *parent = nullptr);
};

#endif
