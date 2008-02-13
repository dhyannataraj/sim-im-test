#ifndef __EV_TYPES_H__
#define __EV_TYPES_H__

enum
{
/* Protocol events */
	MESSAGE_EVENTS,
	SPingMsg,
	SLogin,
	SLoginRej,
	SUpholdConnect,
	END_OF_MESSAGE_EVENTS,
	
/* GUI events */
	GUI_EVENTS,
// Windows
	DialogWnd,
	ContactList, // may not use
	SearchDialog,
	UserInfoWnd,
	SettingsWnd,
// Menus
	ContactMenu,
	GroupMenu,
	MainMenu,
	ContactListMenu,
	END_OF_GUI_EVENTS,

/* End events enum */
	END_OF_EVENTS
};

#endif // __EV_TYPES_H__
