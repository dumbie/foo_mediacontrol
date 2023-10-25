#pragma once

class media_position
{
public:
	media_position();
	~media_position();

private:
	void update_thread();
	bool update_allowed = false;
};