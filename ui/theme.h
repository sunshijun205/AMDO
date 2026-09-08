#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>

namespace Theme {

inline QColor bg() { return QColor("#eef3f6"); }
inline QColor shell() { return QColor("#f8fafb"); }
inline QColor panel() { return QColor("#ffffff"); }
inline QColor panel2() { return QColor("#f3f7f8"); }
inline QColor line() { return QColor("#d5dfe4"); }
inline QColor text() { return QColor("#17242f"); }
inline QColor muted() { return QColor("#687985"); }
inline QColor accent() { return QColor("#0c9b88"); }
inline QColor accentSoft() { return QColor("#dff4ef"); }
inline QColor blue() { return QColor("#2b6fae"); }
inline QColor warn() { return QColor("#b46b22"); }
inline QColor navText() { return QColor("#314b59"); }
inline QColor logBg() { return QColor("#17242f"); }
inline QColor logFg() { return QColor("#dbe7ed"); }

inline QString styleSheet()
{
    return QStringLiteral(R"(
QMainWindow, QWidget#RootShell {
    background: #eef3f6;
    color: #17242f;
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
    font-size: 13px;
}
QLabel { color: #17242f; }
QLabel#BrandLabel {
    font-size: 18px;
    font-weight: 500;
}
QLabel#ProjectLabel {
    color: #687985;
    font-size: 13px;
}
QLabel#MutedLabel, QLabel#FieldLabel, QLabel#UnitLabel, QLabel#MetaLabel, QLabel#NoteLabel {
    color: #687985;
}
QLabel#FieldLabel { font-size: 12px; }
QLabel#UnitLabel {
    font-size: 12px;
    background: #f3f7f8;
    border: 1px solid #d5dfe4;
    border-radius: 5px;
    padding: 0 8px;
}
QLabel#KpiCaption { color: #687985; font-size: 11px; }
QLabel#KpiValue { font-size: 18px; font-weight: 500; }
QLabel#KpiUnit { color: #687985; font-size: 11px; }
QLabel#PanelTitle { font-size: 14px; font-weight: 500; }
QLabel#SectionTitle { font-size: 14px; font-weight: 500; }
QLabel#PageTitle { font-size: 21px; font-weight: 500; }
QLabel#PageSubtitle { color: #687985; font-size: 13px; }
QLabel#Chip {
    background: #dff4ef;
    color: #17242f;
    border-radius: 10px;
    padding: 2px 7px;
    font-size: 11px;
}
QLabel#ChipNeutral {
    background: #f3f7f8;
    color: #687985;
    border-radius: 10px;
    padding: 2px 7px;
    font-size: 11px;
}
QLabel#StatusGood { color: #0c9b88; }
QLabel#StatusWarn { color: #b46b22; }
QLabel#RankBadge {
    background: #dff4ef;
    color: #17242f;
    border-radius: 13px;
    font-size: 11px;
}
QLabel#LogView {
    background: #17242f;
    color: #dbe7ed;
    border-radius: 6px;
    font-family: Consolas, "Courier New", monospace;
    font-size: 11px;
    padding: 11px 13px;
}
QLabel#SideTitle {
    color: #687985;
    font-size: 12px;
    letter-spacing: 1px;
}
QLabel#SideNote {
    color: #687985;
    font-size: 12px;
}
QPushButton {
    border: 1px solid #d5dfe4;
    background: #ffffff;
    color: #17242f;
    padding: 8px 13px;
    border-radius: 6px;
}
QPushButton:hover { background: #f3f7f8; }
QPushButton#PrimaryButton {
    background: #0c9b88;
    color: white;
    border: 1px solid #0c9b88;
}
QPushButton#PrimaryButton:hover { background: #0b8c7b; }
QPushButton#ModeButton {
    min-height: 46px;
    border: 1px solid #d5dfe4;
    background: #ffffff;
    color: #314b59;
    border-radius: 7px;
    font-size: 16px;
    font-weight: 500;
}
QPushButton#ModeButton:hover { background: #dff4ef; color: #17242f; }
QPushButton#ModeButton:checked {
    background: #0c9b88;
    color: white;
    border: 1px solid #0c9b88;
    font-weight: 500;
}
QPushButton#SubTabButton {
    border: 0;
    background: transparent;
    color: #314b59;
    padding: 8px 16px;
    border-radius: 5px;
    font-size: 14px;
    font-weight: 500;
}
QPushButton#SubTabButton:hover { background: #dff4ef; color: #17242f; }
QPushButton#SubTabButton:checked {
    background: #0c9b88;
    color: white;
}
QPushButton#DomainTab {
    border: 0;
    background: transparent;
    color: #17242f;
    text-align: left;
    padding: 8px 10px;
    border-radius: 7px;
    min-height: 48px;
}
QPushButton#DomainTab:hover { background: #f3f7f8; }
QPushButton#DomainTab:checked { background: #dff4ef; }
QLabel#DomainNum {
    background: #f3f7f8;
    color: #687985;
    border-radius: 13px;
    font-size: 12px;
}
QLabel#DomainNumActive {
    background: #0c9b88;
    color: white;
    border-radius: 13px;
    font-size: 12px;
}
QFrame#TopBar, QFrame#FunctionBar, QFrame#SideBar {
    background: #f8fafb;
}
QFrame#TopBar { border-bottom: 1px solid #d5dfe4; }
QFrame#FunctionBar { border-bottom: 1px solid #d5dfe4; }
QFrame#SideBar { border-right: 1px solid #d5dfe4; }
QFrame#Panel, QFrame#Section, QFrame#KpiCard, QFrame#RunBar, QFrame#SubTabBar {
    background: #ffffff;
    border: 1px solid #d5dfe4;
    border-radius: 8px;
}
QFrame#KpiCard { border-radius: 7px; }
QFrame#SubTabBar {
    background: #f3f7f8;
    border-radius: 7px;
}
QFrame#FlowCanvas, QFrame#AircraftView {
    background: #f3f7f8;
    border-radius: 6px;
}
QFrame#FlowNode {
    background: #ffffff;
    border: 1px solid #d5dfe4;
    border-radius: 7px;
}
QFrame#FlowNodeActive {
    background: #ffffff;
    border: 2px solid #0c9b88;
    border-radius: 7px;
}
QFrame#FlowNodeControl {
    background: #dff4ef;
    border: 1px solid #d5dfe4;
    border-radius: 7px;
}
QFrame#FlowNodeDone {
    background: #ffffff;
    border: 1px solid #0c9b88;
    border-radius: 7px;
}
QFrame#FlowNodeRunning {
    background: #ffffff;
    border: 1px solid #2b6fae;
    border-radius: 7px;
}
QFrame#FlowNodePending {
    background: #ffffff;
    border: 1px solid #d5dfe4;
    border-radius: 7px;
}
QFrame#TreeRow {
    background: transparent;
    border-radius: 5px;
}
QFrame#TreeRowActive { background: #dff4ef; border-radius: 5px; }
QLineEdit, QComboBox {
    height: 33px;
    border: 1px solid #d5dfe4;
    border-radius: 5px;
    background: #ffffff;
    color: #17242f;
    padding: 0 9px;
    font-size: 13px;
}
QComboBox::drop-down { border: 0; width: 20px; }
QCheckBox { color: #17242f; font-size: 12px; spacing: 7px; }
QCheckBox::indicator {
    width: 14px;
    height: 14px;
}
QTableWidget {
    background: #ffffff;
    border: 0;
    gridline-color: #d5dfe4;
    font-size: 12px;
    selection-background-color: #f3f7f8;
    selection-color: #17242f;
}
QTableWidget::item { padding: 4px 6px; }
QHeaderView::section {
    background: #ffffff;
    color: #687985;
    border: 0;
    border-bottom: 1px solid #d5dfe4;
    font-weight: 400;
    font-size: 12px;
    padding: 7px 8px;
    text-align: left;
}
QProgressBar {
    border: 0;
    background: #f3f7f8;
    border-radius: 5px;
    height: 7px;
    text-align: center;
    color: transparent;
    max-height: 7px;
}
QProgressBar::chunk {
    background: #0c9b88;
    border-radius: 5px;
}
QScrollArea { border: 0; background: #eef3f6; }
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: #d5dfe4;
    border-radius: 5px;
    min-height: 24px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QTreeWidget {
    background: transparent;
    border: 0;
    font-size: 12px;
}
QTreeWidget::item { height: 28px; padding: 2px; }
QTreeWidget::item:selected, QTreeWidget::item:hover { background: #dff4ef; color: #17242f; }
)");
}

} // namespace Theme

#endif
