/**
 ******************************************************************************
 *
 * @file       escgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <QDebug>
#include <QtOpenGL/QGLWidget>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <stdlib.h>

#include "escgadgetwidget.h"

#define NO_PORT         0
#define SERIAL_PORT     1
#define USB_PORT        2

// constructor
EscGadgetWidget::EscGadgetWidget(QWidget *parent) :
    QWidget(parent),
    m_widget(NULL),
    m_ioDevice(NULL)
{
    m_widget = new Ui_EscWidget();
    m_widget->setupUi(this);

    // Listen to telemetry connection events
    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    if (pluginManager)
    {
        TelemetryManager *telemetryManager = pluginManager->getObject<TelemetryManager>();
        if (telemetryManager)
        {
            connect(telemetryManager, SIGNAL(myStart()), this, SLOT(onTelemetryStart()));
            connect(telemetryManager, SIGNAL(myStop()), this, SLOT(onTelemetryStop()));
            connect(telemetryManager, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
            connect(telemetryManager, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));
        }
    }

    getPorts();

//    connect(m_widget->connectButton, SIGNAL(clicked()), this, SLOT(connectDisconnect()));
//    connect(m_widget->refreshPorts, SIGNAL(clicked()), this, SLOT(getPorts()));
//    connect(m_widget->pushButton_Save, SIGNAL(clicked()), this, SLOT(saveToFlash()));
}

// destructor .. this never gets called :(
EscGadgetWidget::~EscGadgetWidget()
{
}

void EscGadgetWidget::resizeEvent(QResizeEvent *event)
{
}

void EscGadgetWidget::onComboBoxPorts_currentIndexChanged(int index)
{
}

bool sortSerialPorts(const QextPortInfo &s1, const QextPortInfo &s2)
{
    return (s1.portName < s2.portName);
}

void EscGadgetWidget::getPorts()
{
    disconnect(m_widget->comboBox_Ports, 0, 0, 0);

    m_widget->comboBox_Ports->clear();

    // Add item for connecting via FC
    m_widget->comboBox_Ports->addItem("FlightController");

    // Populate the telemetry combo box with serial ports
    QList<QextPortInfo> serial_ports = QextSerialEnumerator::getPorts();
    qSort(serial_ports.begin(), serial_ports.end(), sortSerialPorts);
    QStringList list;
    foreach (QextPortInfo port, serial_ports)
        m_widget->comboBox_Ports->addItem("com: " + port.friendName, SERIAL_PORT);

    connect(m_widget->comboBox_Ports, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxPorts_currentIndexChanged(int)));

    onComboBoxPorts_currentIndexChanged(m_widget->comboBox_Ports->currentIndex());

}

void EscGadgetWidget::onTelemetryStart()
{
    setEnabled(false);
}

void EscGadgetWidget::onTelemetryStop()
{
    setEnabled(true);
}

void EscGadgetWidget::onTelemetryConnect()
{
}

void EscGadgetWidget::onTelemetryDisconnect()
{
}

void EscGadgetWidget::disableTelemetry()
{
    // Suspend telemetry & polling
}

void EscGadgetWidget::enableTelemetry()
{	// Restart the polling thread
}


