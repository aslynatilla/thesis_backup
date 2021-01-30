set(events_SOURCES	event.h event.cpp
			keyboard_events.h keyboard_events.cpp
			mouse_events.h mouse_events.cpp
			window_events.h window_events.cpp
			)

list(TRANSFORM events_SOURCES PREPEND "events/")