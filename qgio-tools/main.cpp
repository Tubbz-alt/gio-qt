#include <QDebug>
#include <QCoreApplication>
#include <QExplicitlySharedDataPointer>

#include <dgiofile.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiovolumemanager.h>
#include <dgiofileinfo.h>

int main(int argc, char * argv[])
{
    DGioFile * f = DGioFile::createFromPath("/media/wzc/aaaaaaaaaaaaaaaa");
    if (f) {
        qDebug() << f->basename() << f->path() << f->uri();
        QExplicitlySharedDataPointer<DGioFileInfo> fi = f->createFileSystemInfo();
        if (fi) {
            qDebug() << fi->fsFreeBytes() << fi->fsUsedBytes() << fi->fsTotalBytes();
        }
        delete f;
    }

    qDebug() << "----------------------";

    DGioMount * m = DGioMount::createFromPath("/media/wzc/aaaaaaaaaaaaaaaa");
    if (m) {
        QExplicitlySharedDataPointer<DGioFile> f = m->getRootFile();
        QExplicitlySharedDataPointer<DGioFile> f2 = m->getDefaultLocationFile();
        qDebug() << m->name() << m->themedIconNames() << f->createFileSystemInfo()->fsTotalBytes() << f->uri() << f2->uri();
        qDebug() << m->name() << m->themedIconNames() << f->createFileSystemInfo()->fsTotalBytes() << f->uri() << f2->uri();
//        m->unmount();
        delete m;
    }

    qDebug() << "----------------------";

    const QList<QExplicitlySharedDataPointer<DGioMount> > mnts = DGioVolumeManager::getMounts();

    for (const QExplicitlySharedDataPointer<DGioMount> &p : mnts) {
        QExplicitlySharedDataPointer<DGioFile> f = p->getRootFile();
        QExplicitlySharedDataPointer<DGioFile> f2 = p->getDefaultLocationFile();
        qDebug() << f->uri() << f2->uri() << f->path() << f2->path();
        qDebug() << f->uri() << f2->uri();
        qDebug() << p->name() << p->uuid() << p->canUnmount() << p->themedIconNames() << p->themedIconNames();
    }

    qDebug() << "----------------------";

    const QList<QExplicitlySharedDataPointer<DGioVolume> > vols = DGioVolumeManager::getVolumes();

    for (const QExplicitlySharedDataPointer<DGioVolume> &p : vols) {
        qDebug() << p->name();
    }

    qDebug() << "----------------------";

    QCoreApplication app(argc, argv);

    DGioVolumeManager vm;

    QObject::connect(&vm, &DGioVolumeManager::mountAdded, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountAdded" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountPreRemoved, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountPreRemoved" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountRemoved, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountRemoved" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountChanged, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountChanged" << mnt->name();
    });

    app.exec();

    return 0;
}
