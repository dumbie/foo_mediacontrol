#pragma once

class monitor_input : public message_filter_impl_base
{
public:
	monitor_input(t_uint32 message = RegisterWindowMessageW(L"SHELLHOOK")) : message_filter_impl_base(message, message) { }

private:
	bool pretranslate_message(MSG* p_msg);
};