﻿/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dgiofileiterator.h"
#include "dgiofileinfo.h"

#include <glibmm.h>
#include <glibmm/refptr.h>

#include <giomm/init.h>
#include <giomm/file.h>

#include <QDebug>

using namespace Gio;

class DGioFileIteratorPrivate
{
public:
    DGioFileIteratorPrivate(DGioFileIterator *qq, FileEnumerator *gmmFileEnumeratorPtr);

    Glib::RefPtr<FileEnumerator> getGmmFileEnumeratorInstance() const;

private:
    Glib::RefPtr<FileEnumerator> m_gmmFileEnumeratorPtr;

    DGioFileIterator *q_ptr;

    void slot_nextFileAsyncResult(const Glib::RefPtr<Gio::AsyncResult>& result);

    Q_DECLARE_PUBLIC(DGioFileIterator)
};

DGioFileIteratorPrivate::DGioFileIteratorPrivate(DGioFileIterator *qq, FileEnumerator *gmmFileEnumeratorPtr)
    : m_gmmFileEnumeratorPtr(gmmFileEnumeratorPtr)
    , q_ptr(qq)
{

}

Glib::RefPtr<FileEnumerator> DGioFileIteratorPrivate::getGmmFileEnumeratorInstance() const
{
    return m_gmmFileEnumeratorPtr;
}

void DGioFileIteratorPrivate::slot_nextFileAsyncResult(const Glib::RefPtr<AsyncResult> &result)
{
    Q_Q(DGioFileIterator);

    try {
        Glib::ListHandle< Glib::RefPtr<FileInfo> > files = m_gmmFileEnumeratorPtr->next_files_finish(result);
        QList<QExplicitlySharedDataPointer<DGioFileInfo> > fileInfoList;

        for (auto gmmFileInfoPtr : files) {
            QExplicitlySharedDataPointer<DGioFileInfo> info(new DGioFileInfo(gmmFileInfoPtr.release()));
            fileInfoList.append(info);
        }

        Q_EMIT q->nextFilesReady(fileInfoList);
    } catch (const Gio::Error& error) {
        if (error.code() != Gio::Error::CANCELLED) {
            Q_EMIT q->nextFilesCancelled();
        } else {
            // should we add an error signal?
            qDebug() << QString::fromStdString(error.what().raw());
        }
    } catch (const Glib::Error& error) {
        // should we add an error signal?
        qDebug() << QString::fromStdString(error.what().raw());
    }
}

// -------------------------------------------------------------

DGioFileIterator::DGioFileIterator(FileEnumerator* gmmFileEnumeratorPtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioFileIteratorPrivate(this, gmmFileEnumeratorPtr))
{
    // gmmFileEnumeratorPtr must be valid;
    Q_CHECK_PTR(gmmFileEnumeratorPtr);
}

DGioFileIterator::~DGioFileIterator()
{

}

QExplicitlySharedDataPointer<DGioFileInfo> DGioFileIterator::nextFile()
{
    Q_D(DGioFileIterator);

    try {
        // can return nullptr if there is no file left.
        Glib::RefPtr<FileInfo> gmmFileInfoPtr = d->getGmmFileEnumeratorInstance()->next_file();
        if (gmmFileInfoPtr) {
            QExplicitlySharedDataPointer<DGioFileInfo> fileInfo(new DGioFileInfo(gmmFileInfoPtr.release()));
            return fileInfo;
        }
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }

    return QExplicitlySharedDataPointer<DGioFileInfo>(nullptr);
}

void DGioFileIterator::nextFilesAsync(int numberOfFiles, DGioIOPriority io_priority)
{
    Q_D(DGioFileIterator);


    d->getGmmFileEnumeratorInstance()->next_files_async(sigc::mem_fun(d, &DGioFileIteratorPrivate::slot_nextFileAsyncResult),
                                                        numberOfFiles, io_priority);
}
