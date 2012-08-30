/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2011 Radek Polak <psonek2@seznam.cz>
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

#ifndef VENDOR_NEO_P_H
#define VENDOR_NEO_P_H

#include <qmodemservice.h>
#include <qmodemcall.h>
#include <qmodemcallprovider.h>
#include <qmodemsimtoolkit.h>
#include <qmodemphonebook.h>
#include <qmodempinmanager.h>
#include <qmodempreferrednetworkoperators.h>
#include <qbandselection.h>
#include <qvibrateaccessory.h>
#include <qmodemnetworkregistration.h>
#include <qmodemsiminfo.h>
#include <qmodemcallvolume.h>
#include <qmodemsimfiles.h>
#include <qmodemservicenumbers.h>

#include <alsa/asoundlib.h>
#include <QTimer>

class NeoModemService;

class NeoCallProvider : public QModemCallProvider
{
    Q_OBJECT
public:
    NeoCallProvider(NeoModemService * service);
    ~NeoCallProvider();

public slots:
    void doClcc();
    void clcc(bool, const QAtResult &);
    virtual void ringing(const QString & number, const QString & callType,
                          uint modemIdentifier = 0);
protected:
    QTimer clccTimer;
    NeoModemService *modemService;
    void abortDial(uint id, QPhoneCall::Scope scope);
    bool hasRepeatingRings() const;
};

class NeoModemService : public QModemService
{
    Q_OBJECT
public:
    NeoModemService
        (const QString & service, QSerialIODeviceMultiplexer * mux,
         QObject * parent = 0);
    ~NeoModemService();

    void initialize();
    bool supportsAtCced();

private:
    NeoCallProvider *neoCallProvider;
    
private slots:
    void sigq(const QString & msg);
    void reset();
    void suspend();
    void wake();
};

class NeoVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
    NeoVibrateAccessory(QModemService * service);
    ~NeoVibrateAccessory();

public slots:
    void setVibrateNow(const bool value, int timeoutMs = 0, int strength = 0xffff);
    void setVibrateOnRing(const bool value);

private:
    int rumbleFd;
    qint16 effectId;
};

#endif
