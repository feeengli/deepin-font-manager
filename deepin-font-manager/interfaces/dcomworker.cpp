/*
 *
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     lilinling <lilinling@uniontech.com>
*
* Maintainer: lilinling <lilinling@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dcomworker.h"
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"
#include "dcopyfilesmanager.h"

#include <QDir>

#include <unistd.h>

DComWorker::DComWorker(QObject *parent)
    : QObject(parent)
{
    if (!autoDelete())
        setAutoDelete(true);
}

void DComWorker::run()
{
}

GetFontListWorker::GetFontListWorker(GetFontListWorker::FontType type, bool isStartup, QObject *parent)
    : DComWorker(parent)
    , m_type(type)
    , m_isStartup(isStartup)
{
}

GetFontListWorker::GetFontListWorker()
{
    DFontInfoManager *inst = DFontInfoManager::instance();
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();

    thread->m_allFontPathList.clear();
    thread->m_allFontPathList = inst->getAllFontPath(true);

    removeUserAddFonts();

    DFMDBManager::instance()->getAllRecords();
    QList<DFontPreviewItemData> list = DFMDBManager::instance()->getFontInfo(50, &thread->m_delFontInfoList);
    thread->m_startModelList = list;
    thread->m_fontModelList.append(list);
}


void GetFontListWorker::run()
{
    qDebug() << __FUNCTION__ << m_type << "begin";
    DFontInfoManager *inst = DFontInfoManager::instance();
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    if (m_type == ALL || m_type == AllInSquence) {
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(m_isStartup);
        if (m_isStartup) {
            removeUserAddFonts();
            qDebug() << __FUNCTION__ << m_isStartup;
            inst->refreshList(thread->m_allFontPathList);
            thread->m_fontModelList = DFMDBManager::instance()->getAllFontInfo(&thread->m_delFontInfoList);
        }
    }

    if (m_type == CHINESE || m_type == AllInSquence) {
        thread->m_chineseFontPathList.clear();
        thread->m_chineseFontPathList = inst->getAllChineseFontPath();
    }

    if (m_type == MONOSPACE || m_type == AllInSquence) {
        thread->m_monoSpaceFontPathList.clear();
        thread->m_monoSpaceFontPathList = inst->getAllMonoSpaceFontPath();
    }
    qDebug() << __FUNCTION__ << m_type << "end";
}

void GetFontListWorker::removeUserAddFonts()
{
    if (geteuid() == 0) {
        return;
    }
    QStringList installFont = DFMDBManager::instance()->getInstalledFontsPath();

    QList<QString>::iterator iter = DFontPreviewListDataThread::instance()->m_allFontPathList.begin();
    while (iter != DFontPreviewListDataThread::instance()->m_allFontPathList.end()) {
        QString filePath = *iter;
        if (!DFMDBManager::instance()->isSystemFont(filePath) && !installFont.contains(filePath)) {
            QFileInfo openFile(filePath);
            QFile::remove(filePath);

            QDir fileDir(openFile.path());
            if (fileDir.isEmpty()) {
                fileDir.removeRecursively();
            }

            iter = DFontPreviewListDataThread::instance()->m_allFontPathList.erase(iter);
        } else {
            ++iter;
        }
    }
}

void FontManager::getFontList(bool isStartup)
{
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();

    GetFontListWorker *getAll = new GetFontListWorker(GetFontListWorker::ALL, isStartup);
    threadPool->start(getAll);
    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE, isStartup);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE, isStartup);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
}

void FontManager::getFontListInSequence(bool isStartup)
{
    GetFontListWorker getFontList(GetFontListWorker::AllInSquence, isStartup);
    getFontList.run();
}


/*************************************************************************
 <Function>      getStartFontList
 <Description>   获取启动时的需要用到的字体列表
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManager::getStartFontList()
{
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();

    GetFontListWorker *getAll = new GetFontListWorker();
    threadPool->start(getAll);
    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE, true);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE, true);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
}
