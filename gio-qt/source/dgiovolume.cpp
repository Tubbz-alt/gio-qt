/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dgiomount.h"
#include "dgiovolume.h"

#include <glibmm/refptr.h>
#include <giomm/volume.h>

#include <QDebug>

using namespace Gio;

class DGioVolumePrivate
{
public:
    DGioVolumePrivate(DGioVolume *qq, Volume *gmmVolumePtr);

    Glib::RefPtr<Volume> getGmmVolumeInstance() const;

    QString name() const;

private:
    Glib::RefPtr<Volume> m_gmmVolumePtr;

    DGioVolume *q_ptr;

    Q_DECLARE_PUBLIC(DGioVolume)
};

DGioVolumePrivate::DGioVolumePrivate(DGioVolume *qq, Volume *gmmVolumePtr)
    : m_gmmVolumePtr(gmmVolumePtr)
    , q_ptr(qq)
{
    //    m_gvolumePtr = Glib::wrap(gvolumePtr);
}

Glib::RefPtr<Volume> DGioVolumePrivate::getGmmVolumeInstance() const
{
    return m_gmmVolumePtr;
}

QString DGioVolumePrivate::name() const
{
    return QString::fromStdString(m_gmmVolumePtr->get_name());
}

// -------------------------------------------------------------

DGioVolume::DGioVolume(Volume* gmmVolumePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioVolumePrivate(this, gmmVolumePtr))
{
    // gvolumePtr must be valid;
    Q_CHECK_PTR(gmmVolumePtr);
}

DGioVolume::~DGioVolume()
{

}

QString DGioVolume::name() const
{
    Q_D(const DGioVolume);

    return d->name();
}

QString DGioVolume::volumeMonitorName() const
{
    Q_D(const DGioVolume);

    if (QString(G_OBJECT_CLASS_NAME(G_OBJECT_GET_CLASS(d->m_gmmVolumePtr->gobj()))) == "GProxyVolume") {
        return (const char*)g_object_get_data(G_OBJECT(d->m_gmmVolumePtr->gobj()), "g-proxy-volume-volume-monitor-name");
    }

    return "";
}

bool DGioVolume::canMount() const
{
    Q_D(const DGioVolume);

    return d->getGmmVolumeInstance()->can_mount();
}

bool DGioVolume::canEject() const
{
    Q_D(const DGioVolume);

    return d->getGmmVolumeInstance()->can_eject();
}

bool DGioVolume::shouldAutoMount() const
{
    Q_D(const DGioVolume);

    return d->getGmmVolumeInstance()->should_automount();
}

void DGioVolume::mount() const
{
    Q_D(const DGioVolume);

    d->getGmmVolumeInstance()->mount();
}

// Return value can be nullptr
QExplicitlySharedDataPointer<DGioMount> DGioVolume::getMount()
{
    Q_D(DGioVolume);

    Glib::RefPtr<Mount> mnt = d->getGmmVolumeInstance()->get_mount();
    QExplicitlySharedDataPointer<DGioMount> mntPtr(new DGioMount(mnt.release()));

    return mntPtr;
}

QString DGioVolume::identifier(DGioVolumeIdentifierType id) const
{
    Q_D(const DGioVolume);

    const static QHash<DGioVolumeIdentifierType, std::string> idtmap = {
        {DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_LABEL, G_VOLUME_IDENTIFIER_KIND_LABEL},
        {DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_NFS_MOUNT, G_VOLUME_IDENTIFIER_KIND_NFS_MOUNT},
        {DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_UNIX_DEVICE, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE},
        {DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_UUID, G_VOLUME_IDENTIFIER_KIND_UUID},
        {DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_CLASS, G_VOLUME_IDENTIFIER_KIND_CLASS}
    };

    return QString::fromStdString(d->getGmmVolumeInstance()->get_identifier(idtmap[id]));
}
