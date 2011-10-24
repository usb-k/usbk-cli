/*
 * @file libusbk.cpp
 *
 * Copyright (C) 2010 USB-K Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * See http://www.gnu.org/licenses/ for more information
 *
 ****************************************************************************
 * This is a utility program for listing SCSI devices and hosts (HBAs)
 * in the Linux operating system. It is applicable to kernel versions
 * 2.6.1 and greater.
 *
 */

#include "config.hpp"
#include "libusbk.hpp"

#include <iostream>
#include <sstream>
#include <stdlib.h>

static bool _libusbk_plusplus_debug_enabled = false;
static const char* _libusbk_plusplus_version = USBK_CPP_LIB_VERSION_FULL;

/* Debugging informations */
void libusbk_plusplus_enable_debug() { _libusbk_plusplus_debug_enabled = true; }
void libusbk_plusplus_disable_debug() { _libusbk_plusplus_debug_enabled = false; }

/* libusbk version */
const char* libusbk_plusplus_plusplus_version() { return _libusbk_plusplus_version; }


UsbkDevice::UsbkDevice(const std::string &device_node)
{
	USBK *usbk_device = usbk_new(device_node.c_str());

	if (!usbk_device) {
		std::cerr << "no such device " << device_node;

		return;
	}

	this->m_usbk_device = usbk_device;
}

UsbkDevice::UsbkDevice(USBK *usbk_device)
{
	this->m_usbk_device = usbk_device;
}

UsbkDevice::~UsbkDevice()
{
	usbk_release(m_usbk_device);
}

usbk_lo_t UsbkDevice::lastOperation() const
{
	return usbk_get_lastopr_status(m_usbk_device);
}

bool UsbkDevice::supported() const
{
	return usbk_check_support(m_usbk_device);
}

std::string UsbkDevice::deviceNode() const
{
	return usbk_get_dev(m_usbk_device);
}

std::string UsbkDevice::deviceNodePath() const
{
	return usbk_get_dev_path(m_usbk_device);
}

std::string UsbkDevice::backdiskNode() const
{
	return usbk_get_backdisk(m_usbk_device);
}

std::string UsbkDevice::backdiskNodePath() const
{
	return usbk_get_backdisk_path(m_usbk_device);
}

std::string UsbkDevice::product() const
{
	return usbk_get_product(m_usbk_device);
}

std::string UsbkDevice::model() const
{
	return usbk_get_model(m_usbk_device);
}

std::string UsbkDevice::serial() const
{
	return usbk_get_serial(m_usbk_device);
}

std::string UsbkDevice::usbSerial() const
{
	return usbk_get_usb_serial(m_usbk_device);
}

std::string UsbkDevice::firmwareVersion() const
{
	return usbk_get_firmware_ver(m_usbk_device);
}

std::string UsbkDevice::deviceLabel() const
{
	return usbk_get_dev_label(m_usbk_device);
}

uint8_t UsbkDevice::multikeyCapability() const
{
	return usbk_get_multikeycap(m_usbk_device);
}

usbk_ds_t UsbkDevice::deviceState() const
{
	return usbk_get_state(m_usbk_device);
}

int UsbkDevice::retryNumber() const
{
	return usbk_get_retry_number(m_usbk_device);
}

int UsbkDevice::currentKey() const
{
	return usbk_get_current_keyno(m_usbk_device);
}

int UsbkDevice::autoactivateKeyNo() const
{
	return usbk_get_autoactivation_keyno(m_usbk_device);
}

std::string UsbkDevice::keyName(int keyno) const
{
	if (keyno < 0 || keyno > multikeyCapability() - 1)
		return std::string();

	return usbk_get_keyname(m_usbk_device, keyno);
}

bool UsbkDevice::activateKey(const std::string &password, int keyno)
{
	return usbk_key_activate(m_usbk_device, password.c_str(), keyno);
}

bool UsbkDevice::deactivate()
{
	return usbk_key_deactivate(m_usbk_device);
}

bool UsbkDevice::changePassword(const std::string &oldPassword, const std::string &newPassword)
{
	return usbk_change_password(m_usbk_device, oldPassword.c_str(), newPassword.c_str());
}

bool UsbkDevice::setDeviceLabel(const std::string &password, const std::string &label)
{
	return usbk_set_devicelabel(m_usbk_device, password.c_str(), label.c_str());
}

bool UsbkDevice::setKeyname(const std::string &password, int keyno, const std::string &keyname)
{
	return usbk_set_keyname(m_usbk_device, password.c_str(), keyno, keyname.c_str());
}

bool UsbkDevice::setKey(const std::string &password, int keyno, usbk_keysize_t keysize, const std::string &key)
{
	return usbk_set_key_text(m_usbk_device, password.c_str(), keyno, keysize, key.c_str());
}

bool UsbkDevice::setAutoactivateKeyno(const std::string &password, int keyno)
{
	return usbk_set_autact(m_usbk_device, password.c_str(), keyno);
}

/* FIXME: not implemented yet */
std::string UsbkDevice::getRandomKey(usbk_keysize_t keysize) const
{
	return std::string();
}

/* FIXME: not implemented yet */
bool UsbkDevice::refreshUsbkInfo()
{
	int ret = usbk_refresh_usbkinfo(m_usbk_device);

	return (ret < 0) ? false : true;
}

std::ostream &operator<<(std::ostream &out, UsbkDevice &device)
{
	device.refreshUsbkInfo();

	std::string title;
	std::string retryNum;
	std::string backdisk = "-";
	std::string status;
	std::string autoactive;

	if (device.supported()) {
		title = "USBK Information ";

		switch (device.deviceState()) {
		case USBK_DS_ACTIVATE:
		case USBK_DS_ACTIVATE_WITH_BACKDISK:
			{
				std::stringstream s;

				s << "active [#"
				  << (int)device.currentKey()
				  << ": " << device.keyName((int)device.currentKey()-1)
				  << "]";
				status = s.str();
			}

			if (device.deviceState()==USBK_DS_ACTIVATE)
				backdisk = "none";
			else
				backdisk = "Plugged In";

			break;

		case USBK_DS_DEACTIVATE:
			status = "deactive";
			break;

		case USBK_DS_FABRIC_DEFAULT:
			status = "USBK in fabric default. Please first set your password.";
			break;

		case USBK_DS_MUST_REMOVE:
			status = "Must remove. Please remove and re-plug the USBK.";
			break;

		default:
			status = "Unknown Status ";
			status += device.deviceState();
			break;
		}

		autoactive = (device.autoactivateKeyNo() == 0)
				? "Disable"
				: "Enabled key with #" + device.autoactivateKeyNo();
		retryNum = device.retryNumber();
	} else {
		title = "Unknown Device!!!";

		status = "n/a";
		autoactive = "n/a";
		backdisk = "n/a";
		retryNum = "n/a";
	}

	out << title
		<< std::endl << "\tProduct          : " << device.product()
		<< std::endl << "\tModel            : " << device.model()
		<< std::endl << "\tSerial Number    : " << device.serial()
		<< std::endl << "\tFirmware Version : " << device.firmwareVersion()
		<< std::endl
		<< std::endl << "\tDeviceLabel      : " << device.deviceLabel()
		<< std::endl << "\tDevice Node      : " << device.deviceNodePath()
		<< std::endl << "\tBackDisk Node    : " << device.backdiskNodePath()
		<< std::endl << "\tStatus           : " << status
		<< std::endl << "\tRetry Number     : " << retryNum
		<< std::endl << "\tBackDisk         : " << backdisk
		<< std::endl << "\tAuto Activation  : " << autoactive
		<< std::endl;

	out << std::endl << "\tMax. Key Capacity : ";
	out << (int)device.multikeyCapability() << std::endl;

    for (int i=0; i<device.multikeyCapability(); i++) {
    	out << "\t" <<
    			(i == device.currentKey() - 1 &&
    			 (device.deviceState() == USBK_DS_ACTIVATE ||
    			  device.deviceState() == USBK_DS_ACTIVATE_WITH_BACKDISK)
    			    			             ? "[*]" : "   ");
        out << " Key #" << i+1
        	<< " , Name " << device.keyName(i);

        out << std::endl;
    }

	return out;
}

UsbkDeviceList::UsbkDeviceList()
{
	USBK_LIST *_usbk_list = usbk_list_new();
    USBK* _usbk_entry;

    usbk_list_entry_foreach (_usbk_entry, _usbk_list) {
        UsbkDevice *new_usbk_device = new UsbkDevice(_usbk_entry);
    	this->push_back(new_usbk_device);
    }

    usbk_list_release(_usbk_list);
}

std::ostream &operator<<(std::ostream &out, UsbkDeviceList &deviceList)
{
	out << "All USBK devices:" << std::endl;

	for (int i=0; i<deviceList.capacity(); i++) {
		UsbkDevice *device = deviceList[i];

		out << "\t " << device->deviceLabel()
			<< std::endl << "\t Device   :" << device->deviceNode()
			<< std::endl << "\t BackDisk :" << device->backdiskNode()
			<< std::endl << "\t Product  :" << device->product()
			<< std::endl << "\t Model    :" << device->model()
			<< std::endl << "\t Serial   :" << device->serial()
			<< std::endl << "\t Firmware :" << device->firmwareVersion()
			<< std::endl << std::endl;
	}

	out << deviceList.capacity() << " USBK devices found." << std::endl;

	return out;
}
