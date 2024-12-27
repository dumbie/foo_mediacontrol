#pragma once
#include "stdafx.h"
#include "monitor_input.h"
#include "monitor_player.h"

class myinitquit : public initquit
{
public:
	void on_init()
	{
		//Change foobar2000 settings
		auto configStore = fb2k::configStore::get();
		configStore->setConfigInt("ui.appCommand.global", 1); //Enable: Process global system media key events
		configStore->setConfigInt("core.useUVC", 0); //Disable: Integrate with Windows Universal Volume Control

		//Initialize monitor classes
		m_monitor_input = new monitor_input();
		m_monitor_player = new monitor_player();
	}
	void on_quit()
	{
		delete m_monitor_input;
		delete m_monitor_player;
	}

private:
	monitor_input* m_monitor_input;
	monitor_player* m_monitor_player;
};

FB2K_SERVICE_FACTORY(myinitquit);