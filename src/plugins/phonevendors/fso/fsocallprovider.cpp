/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2011 Radek Polak.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "fsocallprovider.h"

FsoCallProvider::FsoCallProvider( FsoModemService * service )
    : QPhoneCallProvider( service->service(), service )
    , service(service)
{
    connect(&service->gsmCall,
            SIGNAL(CallStatus(int, const QString &, const QVariantMap &)),
            this,
            SLOT(callStatusChange(int, const QString &, const QVariantMap &)));

    setCallTypes(QStringList() << "Voice");
}

FsoCallProvider::~FsoCallProvider()
{
}

QPhoneCallImpl *FsoCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    qDebug() << "FsoCallProvider::create callType=" << callType;
    return new FsoPhoneCall( this, service, identifier, callType, -1 );
}

void FsoCallProvider::callStatusChange(int id, const QString &status, const QVariantMap &properties)
{
    QString str = QString("id=%1, status=%2").arg(id).arg(status);
    for(int i = 0; i < properties.count(); i++)
    {
        QString key = properties.keys().at(i);
        str += ", " + key + "=" + properties.value(key).toString();
    }
    qDebug() << "CallStatusChange" << str;

    if(status == "INCOMING")
    {
        // Incoming calls have to be created here
        FsoPhoneCall *call = new FsoPhoneCall( this, service, NULL, "Voice", id );
        call->setNumber(properties.value("peer").toString());
        call->setFsoStatus(status);
        return;
    }
    
    // Check if it is an existing call
    QList<QPhoneCallImpl *> list = calls();
    for(int i = 0; i < list.count(); i++)
    {
        FsoPhoneCall *call = static_cast<FsoPhoneCall*>(list.at(i));
        qDebug() << "call identifier=" << call->identifier() << ", call id=" << call->id;
        if(call->id == id)
        {
            call->setFsoStatus(status);
            return;
        }
    }

    if(status == "RELEASE")
    {
        return;     // Released call can have id -1, it was not found in the loop but that's ok
    }

    qWarning() << "callStatusChange: unhandled status for new call" << status;
}