/*
obs-midi
Copyright (C) 2016-2017	Stéphane Lepin <stephane.lepin@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <QtCore/QCryptographicHash>
#include <QtCore/QTime>
#include <QtWidgets/QSystemTrayIcon>

#include "forms/settings-dialog.h"
#include "utils.h"
#include "obs-midi.h"
#include "config.h"
#include "device-manager.h"

#define PARAM_DEVICES "MidiDevices"

#define QT_TO_UTF8(str) str.toUtf8().constData()

using namespace std;

Config::Config() : DebugEnabled(false), AlertsEnabled(true), SettingsLoaded(false)
{
	this->setParent(plugin_window);
	qsrand(QTime::currentTime().msec());
	GetConfigStore();
	obs_frontend_add_event_callback(on_frontend_event, this);
}

Config::~Config()
{
	obs_frontend_remove_event_callback(on_frontend_event, this);
}

/* Load the configuration from the OBS Config Store
 */
void Config::Load()
{
	auto deviceManager = GetDeviceManager();
	obs_data_array_t *deviceManagerData = obs_data_get_array(midiConfig, PARAM_DEVICES);
	deviceManager->Load(deviceManagerData);
	SettingsLoaded = true;
}

/* Save the configuration to the OBS Config Store
 */
void Config::Save()
{

	auto deviceManager = GetDeviceManager();
	auto data = deviceManager->GetData();
	obs_data_set_array(midiConfig, PARAM_DEVICES, data);
	const char *file = "obs-midi.json";
	auto path = obs_module_config_path(file);
	obs_data_save_json(midiConfig, path);
	bfree(path);
}


void Config::GetConfigStore()
{
	const char *file = "obs-midi.json";
	auto path = obs_module_config_path(NULL);
	auto filepath = obs_module_config_path(file);
	os_mkdirs(path);
	blog(LOG_DEBUG, "config path is %s", file);
	if (os_file_exists(filepath)) {
		midiConfig = obs_data_create_from_json_file(filepath);
	} else {
		midiConfig = obs_data_create();
		obs_data_save_json(midiConfig, filepath);
	}
	bfree(filepath);
}

void Config::on_frontend_event(obs_frontend_event event, void *param)
{
	if (event == OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
		auto deviceManager = GetDeviceManager();
		deviceManager->Unload();
		auto config = GetConfig();
		config->Load();
	}
}
