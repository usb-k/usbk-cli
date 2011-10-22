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

#include "config.h"
#include "../../libusbk/src/libusbk.h"

#include <vector>
#include <string>

class UsbkDevice : public USBK
{
public:

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
	int currentKey() const;
	int autoactivateKeyNo() const;
	std::vector<std::string> keyNames() const;

	bool activateKey(int keyno);
	bool deactivate();
	bool changePassword(const std::string &old_password, const std::string &new_password);
	bool setDeviceLabel(const std::string &password, const std::string &label);
	bool setKeyname(const std::string &password, int keyno, const std::string &keyname);
	bool setKey(const std::string &password, int keyno, usbk_keysize_t keysize, const std::string &key);
	bool setAutoactivateKeyno(const std::string &password, int keyno);
	std::string getRandomKey(usbk_keysize_t keysize) const;
	bool refreshUsbkInfo();

	static bool isDebugEnabled();
	static void setDebug(bool enabled);
	static std::string libusbkVersion();

private:
	bool m_debugEnabled;
};

/*class UsbkDeviceList : public std::vector<UsbkDevice>
{

};*/

#endif /* LIBUSBK_H_ */

