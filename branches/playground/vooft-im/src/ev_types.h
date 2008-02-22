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
	SGenContact,
	END_OF_MESSAGE_EVENTS,
	
	GUI_EVENTS,
	SMsgWndProto,
	SMsgWndUserInfo,
	SMsgBtns,
	END_OF_GUI_EVENTS,
	
/* End events enum */
	END_OF_EVENTS
};

#endif // __EV_TYPES_H__
