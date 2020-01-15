#ifndef DFONTPREVIEWLISTDATATHREAD_H
#define DFONTPREVIEWLISTDATATHREAD_H

#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QThread>

class DFontPreviewListDataThread : public QObject
{
    Q_OBJECT
public:
    static DFontPreviewListDataThread *instance();

    DFontPreviewListDataThread();
    virtual ~DFontPreviewListDataThread();

    //从fontconfig配置文件同步字体启用/禁用状态数据
    void syncFontEnableDisableStatusData(QStringList disableFontPathList);

    void refreshFontListData(bool isStartup = false);
    void removeFontData(DFontPreviewItemData removeItemData);

    void insertFontItemData(QString filePath,
                            int index,
                            QStringList chineseFontPathList,
                            QStringList monoSpaceFontPathList,
                            bool isStartup = false);

    QList<DFontPreviewItemData> getFontModelList() const;
    QList<DFontPreviewItemData> getDiffFontModelList() const;

signals:
    void resultReady();

protected slots:
    void doWork();

protected:
    QThread *mThread {nullptr};
    DFontPreviewListDataThread *mFunc {nullptr};

    DFMDBManager *m_dbManager {nullptr};
    QList<DFontPreviewItemData> m_fontModelList;
    QList<DFontPreviewItemData> m_diffFontModelList;
};

#endif // DFONTPREVIEWLISTDATATHREAD_H