#ifndef PROJECTNOTE_H
#define PROJECTNOTE_H

#include <QString>

// 本地数据层 POD：方案备注（MVP 示例）
struct ProjectNote {
    QString text;
    bool textKnown = false;
};

#endif
