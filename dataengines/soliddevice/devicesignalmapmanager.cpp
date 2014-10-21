/*
 *   Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "devicesignalmapmanager.h"

DeviceSignalMapManager::DeviceSignalMapManager(QObject *parent) : QObject(parent)
{
    user = parent;
}

DeviceSignalMapManager::~DeviceSignalMapManager()
{
}

void DeviceSignalMapManager::mapDevice(Solid::Battery *battery, const QString &udi)
{
    BatterySignalMapper *map=0;
    if (!signalmap.contains(Solid::DeviceInterface::Battery)) {
        map = new BatterySignalMapper(this);
        signalmap[Solid::DeviceInterface::Battery] = map;
        connect(map, SIGNAL(deviceChanged(QString,QString,QVariant)), user, SLOT(deviceChanged(QString,QString,QVariant)));
    } else {
        map = (BatterySignalMapper*)signalmap[Solid::DeviceInterface::Battery];
    }

    connect(battery, SIGNAL(chargePercentChanged(int,QString)), map, SLOT(chargePercentChanged(int)));
    connect(battery, SIGNAL(chargeStateChanged(int,QString)), map, SLOT(chargeStateChanged(int)));
    connect(battery, SIGNAL(plugStateChanged(bool,QString)), map, SLOT(plugStateChanged(bool)));
    map->setMapping(battery, udi);
}

void DeviceSignalMapManager::mapDevice(Solid::StorageAccess *storageaccess, const QString &udi)
{
    StorageAccessSignalMapper *map=0;
    if (!signalmap.contains(Solid::DeviceInterface::StorageAccess)) {
        map = new StorageAccessSignalMapper(this);
        signalmap[Solid::DeviceInterface::StorageAccess] = map;
        connect(map, SIGNAL(deviceChanged(QString,QString,QVariant)), user, SLOT(deviceChanged(QString,QString,QVariant)));
    } else {
        map = (StorageAccessSignalMapper*)signalmap[Solid::DeviceInterface::StorageAccess];
    }

    connect(storageaccess, SIGNAL(accessibilityChanged(bool,QString)), map, SLOT(accessibilityChanged(bool)));
    map->setMapping(storageaccess, udi);
}

void DeviceSignalMapManager::unmapDevice(Solid::Battery *battery)
{
    BatterySignalMapper *map = (BatterySignalMapper*)signalmap.value(Solid::DeviceInterface::Battery);
    if (!map) {
        return;
    }

    disconnect(battery, SIGNAL(chargePercentChanged(int,QString)), map, SLOT(chargePercentChanged(int)));
    disconnect(battery, SIGNAL(chargeStateChanged(int,QString)), map, SLOT(chargeStateChanged(int)));
    disconnect(battery, SIGNAL(plugStateChanged(bool,QString)), map, SLOT(plugStateChanged(bool)));
}

void DeviceSignalMapManager::unmapDevice(Solid::StorageAccess *storageaccess)
{
    StorageAccessSignalMapper *map = (StorageAccessSignalMapper*)signalmap.value(Solid::DeviceInterface::StorageAccess);
    if (!map) {
        return;
    }

    disconnect(storageaccess, SIGNAL(accessibilityChanged(bool,QString)), map, SLOT(accessibilityChanged(bool)));
}


