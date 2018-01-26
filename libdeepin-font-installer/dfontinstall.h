/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef DFONTINSTALL_H
#define DFONTINSTALL_H

#include <QThread>

class DFontInstall : public QThread
{
    Q_OBJECT

public:
    DFontInstall(QObject *parent = nullptr);
    ~DFontInstall();

    void startInstall(const QStringList &list);
    void run();

signals:
    void installStarted();
    void installFinished();

private:
    QStringList m_fileList;
};

#endif