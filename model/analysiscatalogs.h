#ifndef ANALYSISCATALOGS_H
#define ANALYSISCATALOGS_H

#include "model/analysistypes.h"

class AnalysisCatalogs
{
public:
    // 六学科分析配置的静态结构与默认值。
    static QVector<AnalysisDomain> domains();

    // 由目录默认值构建的种子分析集（首次运行写入）。
    static AnalysisDocument seedDocument();
};

#endif
