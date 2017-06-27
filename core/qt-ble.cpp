// SPDX-License-Identifier: GPL-2.0
#include <errno.h>

#include <QtBluetooth/QBluetoothAddress>
#include <QLowEnergyController>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <QDebug>

#include <libdivecomputer/version.h>

#include "libdivecomputer.h"
#include "core/qt-ble.h"

#if defined(SSRF_CUSTOM_IO)

#include <libdivecomputer/custom_io.h>

extern "C" {

void waitFor(int ms) {
	Q_ASSERT(QCoreApplication::instance());
	Q_ASSERT(QThread::currentThread());

	QElapsedTimer timer;
	timer.start();

	do {
		QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
		QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
		QThread::msleep(10);
	} while (timer.elapsed() < ms);
}

void BLEObject::serviceStateChanged(QLowEnergyService::ServiceState s)
{
	QList<QLowEnergyCharacteristic> list;

	auto service = qobject_cast<QLowEnergyService*>(sender());
	if (service)
		list = service->characteristics();

	Q_FOREACH(QLowEnergyCharacteristic c, list) {
		qDebug() << "   " << c.uuid().toString();
	}
}

void BLEObject::characteristcStateChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
	receivedPackets.append(value);
	waitForPacket.exit();
}

void BLEObject::writeCompleted(const QLowEnergyDescriptor &d, const QByteArray &value)
{
	qDebug() << "BLE write completed";
}

void BLEObject::addService(const QBluetoothUuid &newService)
{
	qDebug() << "Found service" << newService;
	bool isStandardUuid = false;
	newService.toUInt16(&isStandardUuid);
	if (isStandardUuid) {
		qDebug () << " .. ignoring standard service";
		return;
	}

	auto service = controller->createServiceObject(newService, this);
	qDebug() << " .. created service object" << service;
	if (service) {
		services.append(service);
		connect(service, &QLowEnergyService::stateChanged, this, &BLEObject::serviceStateChanged);
		connect(service, &QLowEnergyService::characteristicChanged, this, &BLEObject::characteristcStateChanged);
		connect(service, &QLowEnergyService::descriptorWritten, this, &BLEObject::writeCompleted);
		service->discoverDetails();
	}
}

BLEObject::BLEObject(QLowEnergyController *c, dc_user_device_t *d)
{
	controller = c;
	device = d;
}

BLEObject::~BLEObject()
{
	qDebug() << "Deleting BLE object";
}

/* Yeah, I could do the C++ inline member thing */
static int device_is_shearwater(dc_user_device_t *device)
{
	return !strcmp(device->vendor, "Shearwater");
}

dc_status_t BLEObject::write(const void *data, size_t size, size_t *actual)
{
	QList<QLowEnergyCharacteristic> list = preferredService()->characteristics();
	QByteArray bytes((const char *)data, (int) size);

	if (!list.isEmpty()) {
		const QLowEnergyCharacteristic &c = list.constFirst();
		QLowEnergyService::WriteMode mode;

		mode = (c.properties() & QLowEnergyCharacteristic::WriteNoResponse) ?
			QLowEnergyService::WriteWithoutResponse :
			QLowEnergyService::WriteWithResponse;

		if (device_is_shearwater(device))
			bytes.prepend("\1\0", 2);

		preferredService()->writeCharacteristic(c, bytes, mode);
		return DC_STATUS_SUCCESS;
	}

	return DC_STATUS_IO;
}

dc_status_t BLEObject::read(void *data, size_t size, size_t *actual)
{
	if (receivedPackets.isEmpty()) {
		QList<QLowEnergyCharacteristic> list = preferredService()->characteristics();
		if (list.isEmpty())
			return DC_STATUS_IO;

		const QLowEnergyCharacteristic &c = list.constLast();

		QTimer timer;
		int msec = 5000;
		timer.setSingleShot(true);

		waitForPacket.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
		timer.start(msec);
		waitForPacket.exec();
	}

	// Still no packet?
	if (receivedPackets.isEmpty())
		return DC_STATUS_IO;

	QByteArray packet = receivedPackets.takeFirst();

	if (device_is_shearwater(device))
		packet.remove(0,2);

	if (size > packet.size())
		size = packet.size();
	memcpy(data, packet.data(), size);
	*actual = size;
	return DC_STATUS_SUCCESS;
}

dc_status_t qt_ble_open(dc_custom_io_t *io, dc_context_t *context, const char *devaddr)
{
	/*
	 * LE-only devices get the "LE:" prepended by the scanning
	 * code, so that the rfcomm code can see they only do LE.
	 *
	 * We just skip that prefix (and it doesn't always exist,
	 * since the device may support both legacy BT and LE).
	 */
	if (!strncmp(devaddr, "LE:", 3))
		devaddr += 3;

	QBluetoothAddress remoteDeviceAddress(devaddr);

	// HACK ALERT! Qt 5.9 needs this for proper Bluez operation
	qputenv("QT_DEFAULT_CENTRAL_SERVICES", "1");

	QLowEnergyController *controller = new QLowEnergyController(remoteDeviceAddress);

	qDebug() << "qt_ble_open(" << devaddr << ")";

	if (device_is_shearwater(io->user_device))
		controller->setRemoteAddressType(QLowEnergyController::RandomAddress);

	// Try to connect to the device
	controller->connectToDevice();

	// Create a timer. If the connection doesn't succeed after five seconds or no error occurs then stop the opening step
	int msec = 5000;
	while (msec > 0 && controller->state() == QLowEnergyController::ConnectingState) {
		waitFor(100);
		msec -= 100;
	};

	switch (controller->state()) {
	case QLowEnergyController::ConnectedState:
		qDebug() << "connected to the controller for device" << devaddr;
		break;
	default:
		qDebug() << "failed to connect to the controller " << devaddr << "with error" << controller->errorString();
		report_error("Failed to connect to %s: '%s'", devaddr, controller->errorString().toUtf8().data());
		controller->disconnectFromDevice();
		delete controller;
		return DC_STATUS_IO;
	}

	/* We need to discover services etc here! */
	BLEObject *ble = new BLEObject(controller, io->user_device);
	ble->connect(controller, SIGNAL(serviceDiscovered(QBluetoothUuid)), SLOT(addService(QBluetoothUuid)));

	qDebug() << "  .. discovering services";

	controller->discoverServices();

	msec = 5000;
	while (msec > 0 && controller->state() == QLowEnergyController::DiscoveringState) {
		waitFor(100);
		msec -= 100;
	};

	qDebug() << " .. done discovering services";
	if (ble->preferredService() == nullptr) {
		qDebug() << "failed to find suitable service on" << devaddr;
		report_error("Failed to find suitable service on '%s'", devaddr);
		controller->disconnectFromDevice();
		delete controller;
		return DC_STATUS_IO;
	}

	qDebug() << " .. discovering details";
	msec = 5000;
	while (msec > 0 && ble->preferredService()->state() == QLowEnergyService::DiscoveringServices) {
		waitFor(100);
		msec -= 100;
	};

	if (ble->preferredService()->state() != QLowEnergyService::ServiceDiscovered) {
		qDebug() << "failed to find suitable service on" << devaddr;
		report_error("Failed to find suitable service on '%s'", devaddr);
		controller->disconnectFromDevice();
		delete controller;
		return DC_STATUS_IO;
	}


	qDebug() << " .. enabling notifications";

	/* Enable notifications */
	QList<QLowEnergyCharacteristic> list = ble->preferredService()->characteristics();

	if (!list.isEmpty()) {
		const QLowEnergyCharacteristic &c = list.constLast();
		QList<QLowEnergyDescriptor> l = c.descriptors();

		qDebug() << "Descriptor list with" << l.length() << "elements";

		QLowEnergyDescriptor d;
		foreach(d, l)
			qDebug() << "Descriptor:" << d.name() << "uuid:" << d.uuid().toString();


		if (!l.isEmpty()) {
			d = l.first();
			qDebug() << "now writing \"0x0100\" to the first descriptor";

			ble->preferredService()->writeDescriptor(d, QByteArray::fromHex("0100"));
		}
	}

	// Fill in info
	io->userdata = (void *)ble;
	return DC_STATUS_SUCCESS;
}

dc_status_t qt_ble_close(dc_custom_io_t *io)
{
	BLEObject *ble = (BLEObject *) io->userdata;

	io->userdata = NULL;
	delete ble;

	return DC_STATUS_SUCCESS;
}

dc_status_t qt_ble_read(dc_custom_io_t *io, void* data, size_t size, size_t *actual)
{
	BLEObject *ble = (BLEObject *) io->userdata;
	return ble->read(data, size, actual);
}

dc_status_t qt_ble_write(dc_custom_io_t *io, const void* data, size_t size, size_t *actual)
{
	BLEObject *ble = (BLEObject *) io->userdata;
	return ble->write(data, size, actual);
}

} /* extern "C" */
#endif
