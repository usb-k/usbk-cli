/*
 * @file libusbk.hpp
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
 *
 * USBK c++ binding header
 */

#ifndef LIBUSBK_HPP_
#define LIBUSBK_HPP_

#include "config.hpp"
#include "../../libusbk/src/libusbk.h"

#include <vector>
#include <string>
#include <ostream>

/* Debugging informations */
extern void libusbk_plusplus_enable_debug();
extern void libusbk_plusplus_disable_debug();

/* libusbk version */
extern const char* libusbk_plusplus_plusplus_version();

class UsbkDevice
{
public:
	UsbkDevice();
	UsbkDevice(const std::string &device_node);
	UsbkDevice(USBK *usbk_device);

	~UsbkDevice();

	/**
	 * All getter functions
	 * */

	usbk_lo_t lastOperation() const;
	bool supported() const;
	std::string deviceNode() const;
	std::string deviceNodePath() const;
	std::string backdiskNode() const;
	std::string backdiskNodePath() const;
	std::string product() const;
	std::string model() const;
	std::string serial() const;
	std::string usbSerial() const;
	std::string firmwareVersion() const;
	std::string deviceLabel() const;
	uint8_t multikeyCapability() const;
	usbk_ds_t deviceState() const;
	int retryNumber() const;
	int currentKey() const;
	int autoactivateKeyNo() const;
	std::string keyName(int keyno) const;

	/**
	 * Other control functions
	 * */
	bool activateKey(const std::string &password, int keyno);
	bool deactivate();
	bool changePassword(const std::string &oldPassword, const std::string &newPassword);
	bool setDeviceLabel(const std::string &password, const std::string &label);
	bool setKeyname(const std::string &password, int keyno, const std::string &keyname);
	bool setKey(const std::string &password, int keyno, usbk_keysize_t keysize, const std::string &key);
	bool setAutoactivateKeyno(const std::string &password, int keyno);
	std::string getRandomKey(usbk_keysize_t keysize) const;
	bool refreshUsbkInfo();

	friend std::ostream &operator<<(std::ostream &out, UsbkDevice &device);

private:
	USBK *m_usbk_device;
};

class UsbkDeviceList : public std::vector<UsbkDevice *>
{
public:
	UsbkDeviceList();

	friend std::ostream &operator<<(std::ostream &out, UsbkDeviceList &device_list);
};

#endif /* LIBUSBK_H_ */

