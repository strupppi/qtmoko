/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
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

#include "tst_qsimtoolkit.h"

#ifndef SYSTEMTEST

// Test encoding and decoding of EVENT DOWNLOAD envelopes based on the
// Test encoding and decoding of SEND USSD commands based on the
// GCF test strings in GSM 51.010, section 27.22.4.12 - SEND USSD.
void tst_QSimToolkit::testEncodeSendUSSD_data()
{
    QSimToolkitData::populateDataSendUSSD();
}
void tst_QSimToolkit::testEncodeSendUSSD()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, number );
    QFETCH( QString, respText );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Check that the command PDU can be parsed correctly.
    QSimCommand decoded = QSimCommand::fromPdu(data);
    QVERIFY( decoded.type() == QSimCommand::SendUSSD );
    QVERIFY( decoded.destinationDevice() == QSimCommand::Network );
    QCOMPARE( decoded.text(), text );
    if ( text.isEmpty() ) {
        if ( ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
            QVERIFY( decoded.suppressUserFeedback() );
        else
            QVERIFY( !decoded.suppressUserFeedback() );
    } else {
        QVERIFY( !decoded.suppressUserFeedback() );
    }
    QCOMPARE( decoded.number(), number );
    QCOMPARE( (int)decoded.iconId(), iconId );
    QCOMPARE( decoded.iconSelfExplanatory(), iconSelfExplanatory );
    QCOMPARE( decoded.textAttribute(), textAttribute );
    if ( !textAttribute.isEmpty() )
        QCOMPARE( decoded.textHtml(), html );

    // Check that the original command PDU can be reconstructed correctly.
    QByteArray encoded = decoded.toPdu( (QSimCommand::ToPduOptions)options );
    QCOMPARE( encoded, data );

    // Check that the terminal response PDU can be parsed correctly.
    QSimTerminalResponse decodedResp = QSimTerminalResponse::fromPdu(resp);
    QVERIFY( data.contains( decodedResp.commandPdu() ) );
    if ( resptype < 0x0100 ) {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)resptype );
        QVERIFY( decodedResp.causeData().isEmpty() );
        QVERIFY( decodedResp.cause() == QSimTerminalResponse::NoSpecificCause );
    } else {
        QVERIFY( decodedResp.result() == (QSimTerminalResponse::Result)(resptype >> 8) );
        QVERIFY( decodedResp.causeData().size() == 1 );
        QVERIFY( decodedResp.cause() == (QSimTerminalResponse::Cause)(resptype & 0xFF) );
    }

    // Check that the original terminal response PDU can be reconstructed correctly.
    QCOMPARE( decodedResp.toPdu(), resp );
}

// Test that SEND USSD commands can be properly delivered to a client
// application and that the client application can respond appropriately.
void tst_QSimToolkit::testDeliverSendUSSD_data()
{
    QSimToolkitData::populateDataSendUSSD();
}
void tst_QSimToolkit::testDeliverSendUSSD()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, number );
    QFETCH( QString, respText );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(resp);
    Q_UNUSED(respText);
    Q_UNUSED(resptype);
    Q_UNUSED(html);

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Clear the client/server state.
    server->clear();
    deliveredCommand = QSimCommand();

    // Compose and send the command.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SendUSSD );
    cmd.setDestinationDevice( QSimCommand::Network );
    cmd.setText( text );
    if ( text.isEmpty() && ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
        cmd.setSuppressUserFeedback( true );
    cmd.setNumber( number );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );
    server->emitCommand( cmd );

    // Wait for the command to arrive in the client.
    QVERIFY( QFutureSignal::wait( this, SIGNAL(commandSeen()), 100 ) );

    // Verify that the command was delivered exactly as we asked.
    QVERIFY( deliveredCommand.type() == cmd.type() );
    QVERIFY( deliveredCommand.text() == cmd.text() );
    QVERIFY( deliveredCommand.suppressUserFeedback() == cmd.suppressUserFeedback() );
    QVERIFY( deliveredCommand.number() == cmd.number() );
    QVERIFY( deliveredCommand.iconId() == cmd.iconId() );
    QVERIFY( deliveredCommand.iconSelfExplanatory() == cmd.iconSelfExplanatory() );
    QVERIFY( deliveredCommand.textAttribute() == cmd.textAttribute() );
    QCOMPARE( deliveredCommand.toPdu( (QSimCommand::ToPduOptions)options ), data );

    // The terminal response should have been sent immediately to ack reception of the command.
    // We cannot check the response text explicitly because that will be handled in the modem
    // and will typically be invisible to Qtopia.  We therefore compare against what the
    // response would be without the return data.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QSimTerminalResponse resp2;
    resp2.setCommand( deliveredCommand );
    resp2.setResult( QSimTerminalResponse::Success );
    QCOMPARE( server->lastResponse(), resp2.toPdu() );
}

// Test the user interface in "simapp" for SEND USSD.
void tst_QSimToolkit::testUISendUSSD_data()
{
    QSimToolkitData::populateDataSendUSSD();
}
void tst_QSimToolkit::testUISendUSSD()
{
    QFETCH( QByteArray, data );
    QFETCH( QByteArray, resp );
    QFETCH( int, resptype );
    QFETCH( QString, text );
    QFETCH( QString, number );
    QFETCH( QString, respText );
    QFETCH( int, iconId );
    QFETCH( bool, iconSelfExplanatory );
    QFETCH( QByteArray, textAttribute );
    QFETCH( QString, html );
    QFETCH( int, options );

    Q_UNUSED(html);
    Q_UNUSED(options);

    // Skip tests that we cannot test using the "simapp" UI.
    if ( resptype == 0x0004 ||      // Icon not displayed
         resptype == 0x0032 ) {     // Command data not understood
        QSKIP( "", SkipSingle );
    }

    // Output a dummy line to give some indication of which test we are currently running.
    qDebug() << "";

    // Create the command to be tested.
    QSimCommand cmd;
    cmd.setType( QSimCommand::SendUSSD );
    cmd.setDestinationDevice( QSimCommand::Network );
    cmd.setText( text );
    if ( text.isEmpty() && ( options & QSimCommand::EncodeEmptyStrings ) != 0 )
        cmd.setSuppressUserFeedback( true );
    cmd.setNumber( number );
    cmd.setIconId( (uint)iconId );
    cmd.setIconSelfExplanatory( iconSelfExplanatory );
    cmd.setTextAttribute( textAttribute );

    // Set up the server with the command, ready to be selected
    // from the "Run Test" menu item on the test menu.
    server->startUsingTestMenu( cmd );
    QVERIFY( waitForView( SimMenu::staticMetaObject ) );

    // Clear the server state just before we request the actual command under test.
    server->clear();

    // Select the first menu item.
    select();

    // Wait for the text to display.  If user feedback is suppressed, then
    // the command is supposed to be performed silently.
    if ( cmd.suppressUserFeedback() )
        QVERIFY( !waitForView( SimText::staticMetaObject ) );
    else
        QVERIFY( waitForView( SimText::staticMetaObject ) );

    // The terminal response should have been sent immediately to ack reception of the command.
    // We cannot check the response text explicitly because that will be handled in the modem
    // and will typically be invisible to Qtopia.  We therefore compare against what the
    // response would be without the return data.
    QCOMPARE( server->responseCount(), 1 );
    QCOMPARE( server->envelopeCount(), 0 );
    QSimTerminalResponse resp2;
    resp2.setCommand( deliveredCommand );
    resp2.setResult( QSimTerminalResponse::Success );
    QCOMPARE( server->lastResponse(), resp2.toPdu() );
}

#endif // !SYSTEMTEST

// Populate data-driven tests for SEND USSD from the GCF test cases
// in GSM 51.010, section 27.22.4.12.
void QSimToolkitData::populateDataSendUSSD()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("resp");
    QTest::addColumn<int>("resptype");
    QTest::addColumn<QString>("text");
    QTest::addColumn<QString>("number");
    QTest::addColumn<QString>("respText");
    QTest::addColumn<int>("iconId");
    QTest::addColumn<bool>("iconSelfExplanatory");
    QTest::addColumn<QByteArray>("textAttribute");
    QTest::addColumn<QString>("html");
    QTest::addColumn<int>("options");

    // Note: we only test the SEND USSD commands and TERMINAL RESPONSE's.
    // We don't test the data that is sent on the network, as we assume
    // that the modem or the modem vendor plugin will handle that automatically.

    static unsigned char const data_1_1_1[] =
        {0xD0, 0x50, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x37, 0x2D, 0x62, 0x69, 0x74, 0x20, 0x55, 0x53, 0x53, 0x44, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_1_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 1.1.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_1_1, sizeof(data_1_1_1) )
        << QByteArray( (char *)resp_1_1_1, sizeof(resp_1_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "7-bit USSD" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_2_1[] =
        {0xD0, 0x58, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x38, 0x2D, 0x62, 0x69, 0x74, 0x20, 0x55, 0x53, 0x53, 0x44, 0x8A,
         0x41, 0x44, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
         0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
         0x57, 0x58, 0x59, 0x5A, 0x2D, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
         0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73,
         0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x2D, 0x31, 0x32, 0x33, 0x34,
         0x35, 0x36, 0x37, 0x38, 0x39, 0x30};
    static unsigned char const resp_1_2_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1D, 0x04, 0x55, 0x53, 0x53, 0x44, 0x20, 0x73, 0x74, 0x72, 0x69,
         0x6E, 0x67, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x64, 0x20,
         0x66, 0x72, 0x6F, 0x6D, 0x20, 0x53, 0x53};
    QTest::newRow( "SEND USSD 1.2.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_2_1, sizeof(data_1_2_1) )
        << QByteArray( (char *)resp_1_2_1, sizeof(resp_1_2_1) )
        << 0x0000       // Command performed successfully
        << QString( "8-bit USSD" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::NoPduOptions );

    static unsigned char const data_1_3_1[] =
        {0xD0, 0x2F, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x09, 0x55, 0x43, 0x53, 0x32, 0x20, 0x55, 0x53, 0x53, 0x44, 0x8A, 0x19,
         0x48, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12, 0x04,
         0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22, 0x04,
         0x15};
    static unsigned char const resp_1_3_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x39, 0x08, 0x00, 0x55, 0x00, 0x53, 0x00, 0x53, 0x00, 0x44, 0x00,
         0x20, 0x00, 0x73, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00, 0x6E, 0x00,
         0x67, 0x00, 0x20, 0x00, 0x72, 0x00, 0x65, 0x00, 0x63, 0x00, 0x65, 0x00,
         0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x64, 0x00, 0x20, 0x00, 0x66, 0x00,
         0x72, 0x00, 0x6F, 0x00, 0x6D, 0x00, 0x20, 0x00, 0x53, 0x00, 0x53};
    static ushort const str_1_3_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "SEND USSD 1.3.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_3_1, sizeof(data_1_3_1) )
        << QByteArray( (char *)resp_1_3_1, sizeof(resp_1_3_1) )
        << 0x0000       // Command performed successfully
        << QString( "UCS2 USSD" )
        << QString::fromUtf16( str_1_3_1, sizeof(str_1_3_1) / sizeof(ushort) )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::UCS2Strings );

    static unsigned char const data_1_4_1[] =
        {0xD0, 0x50, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x37, 0x2D, 0x62, 0x69, 0x74, 0x20, 0x55, 0x53, 0x53, 0x44, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_4_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x37,
         0x47};
    QTest::newRow( "SEND USSD 1.4.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_4_1, sizeof(data_1_4_1) )
        << QByteArray( (char *)resp_1_4_1, sizeof(resp_1_4_1) )
        << 0x3747       // Unknown alphabet
        << QString( "7-bit USSD" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_5_1[] =
        {0xD0, 0x50, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x37, 0x2D, 0x62, 0x69, 0x74, 0x20, 0x55, 0x53, 0x53, 0x44, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_5_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x02, 0x37,
         0x00};
    QTest::newRow( "SEND USSD 1.5.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_5_1, sizeof(data_1_5_1) )
        << QByteArray( (char *)resp_1_5_1, sizeof(resp_1_5_1) )
        << 0x3700       // USSD error - no specific cause
        << QString( "7-bit USSD" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_6_1[] =
        {0xD0, 0x81, 0xFD, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83,
         0x85, 0x81, 0xB6, 0x6F, 0x6E, 0x63, 0x65, 0x20, 0x61, 0x20, 0x52, 0x45,
         0x4C, 0x45, 0x41, 0x53, 0x45, 0x20, 0x43, 0x4F, 0x4D, 0x50, 0x4C, 0x45,
         0x54, 0x45, 0x20, 0x6D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x63,
         0x6F, 0x6E, 0x74, 0x61, 0x69, 0x6E, 0x69, 0x6E, 0x67, 0x20, 0x74, 0x68,
         0x65, 0x20, 0x55, 0x53, 0x53, 0x44, 0x20, 0x52, 0x65, 0x74, 0x75, 0x72,
         0x6E, 0x20, 0x52, 0x65, 0x73, 0x75, 0x6C, 0x74, 0x20, 0x6D, 0x65, 0x73,
         0x73, 0x61, 0x67, 0x65, 0x20, 0x6E, 0x6F, 0x74, 0x20, 0x63, 0x6F, 0x6E,
         0x74, 0x61, 0x69, 0x6E, 0x69, 0x6E, 0x67, 0x20, 0x61, 0x6E, 0x20, 0x65,
         0x72, 0x72, 0x6F, 0x72, 0x20, 0x68, 0x61, 0x73, 0x20, 0x62, 0x65, 0x65,
         0x6E, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x64, 0x20, 0x66,
         0x72, 0x6F, 0x6D, 0x20, 0x74, 0x68, 0x65, 0x20, 0x6E, 0x65, 0x74, 0x77,
         0x6F, 0x72, 0x6B, 0x2C, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x45, 0x20,
         0x73, 0x68, 0x61, 0x6C, 0x6C, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72, 0x6D,
         0x20, 0x74, 0x68, 0x65, 0x20, 0x53, 0x49, 0x4D, 0x20, 0x74, 0x68, 0x61,
         0x74, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6F, 0x6D, 0x6D, 0x61, 0x6E,
         0x64, 0x20, 0x68, 0x61, 0x73, 0x8A, 0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58,
         0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92, 0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9,
         0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59, 0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB,
         0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3, 0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F,
         0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF, 0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD,
         0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_6_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 1.6.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_6_1, sizeof(data_1_6_1) )
        << QByteArray( (char *)resp_1_6_1, sizeof(resp_1_6_1) )
        << 0x0000       // Command performed successfully
        << QString( "once a RELEASE COMPLETE message containing the USSD Return Result "
                    "message not containing an error has been received from the network, "
                    "the ME shall inform the SIM that the command has" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_7_1[] =
        {0xD0, 0x44, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_7_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 1.7.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_7_1, sizeof(data_1_7_1) )
        << QByteArray( (char *)resp_1_7_1, sizeof(resp_1_7_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_1_8_1[] =
        {0xD0, 0x46, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x00, 0x8A, 0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49,
         0xE5, 0x92, 0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E,
         0xB1, 0x59, 0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD,
         0x5E, 0xB3, 0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B,
         0x3E, 0xAF, 0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60};
    static unsigned char const resp_1_8_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 1.8.1 - GCF 27.22.4.12.1" )
        << QByteArray( (char *)data_1_8_1, sizeof(data_1_8_1) )
        << QByteArray( (char *)resp_1_8_1, sizeof(resp_1_8_1) )
        << 0x0000       // Command performed successfully
        << QString( "" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_1_1a[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x00, 0x01};
    static unsigned char const resp_2_1_1a[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.1.1A - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_1_1a, sizeof(data_2_1_1a) )
        << QByteArray( (char *)resp_2_1_1a, sizeof(resp_2_1_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_1_1b[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x00, 0x01};
    static unsigned char const resp_2_1_1b[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.1.1B - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_1_1b, sizeof(data_2_1_1b) )
        << QByteArray( (char *)resp_2_1_1b, sizeof(resp_2_1_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 1 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_2_1a[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x00, 0x02};
    static unsigned char const resp_2_2_1a[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.2.1A - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_2_1a, sizeof(data_2_2_1a) )
        << QByteArray( (char *)resp_2_2_1a, sizeof(resp_2_2_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Color Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_2_1b[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x00, 0x02};
    static unsigned char const resp_2_2_1b[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.2.1B - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_2_1b, sizeof(data_2_2_1b) )
        << QByteArray( (char *)resp_2_2_1b, sizeof(resp_2_2_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Color Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 2 << true    // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_3_1a[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x01, 0x01};
    static unsigned char const resp_2_3_1a[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.3.1A - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_3_1a, sizeof(data_2_3_1a) )
        << QByteArray( (char *)resp_2_3_1a, sizeof(resp_2_3_1a) )
        << 0x0000       // Command performed successfully
        << QString( "Basic Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_3_1b[] =
        {0xD0, 0x54, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x0A, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x49, 0x63, 0x6F, 0x6E, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x01, 0x01};
    static unsigned char const resp_2_3_1b[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x04,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    QTest::newRow( "SEND USSD 2.3.1B - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_3_1b, sizeof(data_2_3_1b) )
        << QByteArray( (char *)resp_2_3_1b, sizeof(resp_2_3_1b) )
        << 0x0004       // Icon not displayed
        << QString( "Basic Icon" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings | QSimCommand::EncodeEmptyStrings );

    static unsigned char const data_2_4_1[] =
        {0xD0, 0x48, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x8A,
         0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92,
         0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59,
         0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3,
         0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF,
         0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5, 0x60, 0x9E, 0x02,
         0x01, 0x01};
    static unsigned char const resp_2_4_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x32};
    QTest::newRow( "SEND USSD 2.4.1 - GCF 27.22.4.12.2" )
        << QByteArray( (char *)data_2_4_1, sizeof(data_2_4_1) )
        << QByteArray( (char *)resp_2_4_1, sizeof(resp_2_4_1) )
        << 0x0032       // Command data not understood
        << QString( "" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "" )
        << 1 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    static unsigned char const data_3_1_1[] =
        {0xD0, 0x5F, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x19, 0x80, 0x04, 0x17, 0x04, 0x14, 0x04, 0x20, 0x04, 0x10, 0x04, 0x12,
         0x04, 0x21, 0x04, 0x22, 0x04, 0x12, 0x04, 0x23, 0x04, 0x19, 0x04, 0x22,
         0x04, 0x15, 0x8A, 0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58, 0x34, 0x1E, 0x91,
         0x49, 0xE5, 0x92, 0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9, 0x94, 0x5A, 0xB5,
         0x5E, 0xB1, 0x59, 0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB, 0xE6, 0x33, 0x3A,
         0xAD, 0x5E, 0xB3, 0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F, 0xD3, 0xEB, 0xF6,
         0x3B, 0x3E, 0xAF, 0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD, 0x76, 0xC3, 0xE5,
         0x60};
    static unsigned char const resp_3_1_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    static ushort const str_3_1_1[] =
        {0x0417, 0x0414, 0x0420, 0x0410, 0x0412, 0x0421, 0x0422, 0x0412, 0x0423,
         0x0419, 0x0422, 0x0415};
    QTest::newRow( "SEND USSD 3.1.1 - GCF 27.22.4.12.3" )
        << QByteArray( (char *)data_3_1_1, sizeof(data_3_1_1) )
        << QByteArray( (char *)resp_3_1_1, sizeof(resp_3_1_1) )
        << 0x0000       // Command performed successfully
        << QString::fromUtf16( str_3_1_1, sizeof(str_3_1_1) / sizeof(ushort) )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray() << QString() // No text attribute information
        << (int)( QSimCommand::PackedStrings );

    // Test only one of the text attribute test cases.  We assume that if
    // one works, then they will all work.  The DISPLAY TEXT command tests
    // the formatting rules.
    static unsigned char const data_4_1_1[] =
        {0xD0, 0x5C, 0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x81, 0x83, 0x85,
         0x10, 0x54, 0x65, 0x78, 0x74, 0x20, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62,
         0x75, 0x74, 0x65, 0x20, 0x31, 0x8A, 0x39, 0xF0, 0x41, 0xE1, 0x90, 0x58,
         0x34, 0x1E, 0x91, 0x49, 0xE5, 0x92, 0xD9, 0x74, 0x3E, 0xA1, 0x51, 0xE9,
         0x94, 0x5A, 0xB5, 0x5E, 0xB1, 0x59, 0x6D, 0x2B, 0x2C, 0x1E, 0x93, 0xCB,
         0xE6, 0x33, 0x3A, 0xAD, 0x5E, 0xB3, 0xDB, 0xEE, 0x37, 0x3C, 0x2E, 0x9F,
         0xD3, 0xEB, 0xF6, 0x3B, 0x3E, 0xAF, 0x6F, 0xC5, 0x64, 0x33, 0x5A, 0xCD,
         0x76, 0xC3, 0xE5, 0x60, 0xD0, 0x04, 0x00, 0x10, 0x00, 0xB4};
    static unsigned char const resp_4_1_1[] =
        {0x81, 0x03, 0x01, 0x12, 0x00, 0x82, 0x02, 0x82, 0x81, 0x83, 0x01, 0x00,
         0x8D, 0x1A, 0x00, 0xD5, 0xE9, 0x94, 0x08, 0x9A, 0xD3, 0xE5, 0x69, 0xF7,
         0x19, 0x24, 0x2F, 0x8F, 0xCB, 0x69, 0x7B, 0x99, 0x0C, 0x32, 0xCB, 0xDF,
         0x6D, 0xD0, 0x74, 0x0A};
    static unsigned char const attr_4_1_1[] =
        {0x00, 0x10, 0x00, 0xB4};
    QTest::newRow( "SEND USSD 4.1.1 - GCF 27.22.4.12.4" )
        << QByteArray( (char *)data_4_1_1, sizeof(data_4_1_1) )
        << QByteArray( (char *)resp_4_1_1, sizeof(resp_4_1_1) )
        << 0x0000       // Command performed successfully
        << QString( "Text Attribute 1" )
        << QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ-abcdefghijklmnopqrstuvwxyz-1234567890" )
        << QString( "USSD string received from SS" )
        << 0 << false   // Icon details
        << QByteArray( (char *)attr_4_1_1, sizeof(attr_4_1_1) )
        << QString( "<body bgcolor=\"#FFFF00\"><div align=\"left\"><font color=\"#008000\">Text Attribute 1</font></div></body>" )
        << (int)( QSimCommand::PackedStrings );
}