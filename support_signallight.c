/********************************************************************
** 	Module:	support_signallight.c
**
**	Author:	Chris Rider
**		Copyright (2013)
**
** 28 Mar 2013, csr - creation
**	
** HUE HTTP commands:
**	GET - used for reading info about various things
**	PUT - the majority of commands to make the lights do things, use this.
**	POST - only used for some things - mostly adding (e.g. adding a white-listed username).
**	DELETE - deletes the entity specified in the URL. 
**
** HUE Addresses:
** 	Bridge control base-URL: 	http://<bridge-ip>/api/<whitelisted-username>
** 	Bridge control config:		http://<bridge-ip>/api/<whitelisted-username>/config
** 	Get all lights info:		GET	http://<bridge-ip>/api/<whitelisted-username>/lights
** 	Scan for new lights:		POST http://<bridge-ip>/api/<whitelisted-username>/lights
** 	Get new lights info:		GET	http://<bridge-ip>/api/<whitelisted-username>/lights/new
**								(Perhaps useful for getting all new lights added from most recent scan)
**	Unique light info/control:	PUT	http://<bridge-ip>/api/<whitelisted-username>/lights/<int-lightnumber>/state
**								(This is used to do stuff with individual lights)
** 	Group-control base-URL: 	http://<bridge-ip>/api/<whitelisted-username>/groups
**								(Likely won't be used with our system, as we use sign-groups)
** 	All lights control:			PUT	http://<bridge-ip>/api/<whitelisted-username>/groups/0/action
**								(Perhaps useful for doing something every single light associated with the specified bridge)
** 	Schedules control base-URL: http://<bridge-ip>/api/<whitelisted-username>/schedule
**								(Likely won't be used with our system)
**
** HUE examples:
**	Add a username to a bridge's whitelist (must press button):
**		curl -X POST "http://<bridge-ip>/api" -d '{"devicetype":"<description-of-device/usr>","username":"<username>"}'
**	Get system-wide information:
**		curl "http://<bridge-ip>/api/<whitelisted-username>"
**	Make the bridge scan for new lights within range:
**		curl -X POST "http://<bridge-ip>/api/<whitelisted-username>/lights"
**	Turn all lights on with full bright white:
**		curl -X PUT "http://<bridge-ip>/api/<whitelisted-username>/groups/0/action" -d '{"on":true,"bri":255,"hue":35300,"sat":128}'
**	Turn light #1 on with full bright white:
**		curl -X PUT "http://<bridge-ip>/api/<whitelisted-username>/lights/1/state" -d '{"on":true,"bri":255,"hue":35300,"sat":128}'
**	Get all info about a particular light:
**		curl "http://<bridge-ip>/api/<whitelisted-username>/lights/1[2,3,...]"
**	 
********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

/***** REDACTED *****/

#include "support_signallight.h"


/* bring the JSON string definitions from the header file into these local constants to help speed things up for banner */
const char STR_SIGNALLIGHT_HUE_OFF_FADEDEF[] 			= SIGNALLIGHT_HUE_JSON_OFF_FADEDEF;
const char STR_SIGNALLIGHT_HUE_OFF_FADE200[] 			= SIGNALLIGHT_HUE_JSON_OFF_FADE200;
const char STR_SIGNALLIGHT_HUE_OFF_FADE800[] 			= SIGNALLIGHT_HUE_JSON_OFF_FADE800;
const char STR_SIGNALLIGHT_HUE_OFF_INSTANT[] 			= SIGNALLIGHT_HUE_JSON_OFF_INSTANT;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM[]			= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM;
const char STR_SIGNALLIGHT_HUE_ON_FADE800_DIM[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE800_DIM;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_UV[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_UV;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_UV[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_UV;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_BLUE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_BLUE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_BLUE;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_GREEN;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_GREEN[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_GREEN;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_ORANGE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_ORANGE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_ORANGE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_ORANGE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_ORANGE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_ORANGE;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_PINK;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_PINK;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_PINK;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_PINK;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_PINK;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_PINK;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_PINK;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_PINK;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PINK[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_PINK;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_PURPLE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_PURPLE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PURPLE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_PURPLE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PURPLE[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_PURPLE;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_RED;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_RED;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_RED;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_RED;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_RED;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_RED;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_RED;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_RED;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_RED[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_RED;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITECOOL;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITECOOL[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITECOOL;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEPURE;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEPURE[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEPURE;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEWARM;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEWARM[] 	= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEWARM;

const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_YELLOW[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_YELLOW[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_YELLOW[] 	= SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_YELLOW;
const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_YELLOW[] 		= SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_YELLOW;


//static int signallight_get_color(char *message);


/*****************************************************************
** static char signallight_getHueLights(struct _hardware *hw_ptr)
** 
**	Returns all of the light bulbs currently assigned to 
**	the specified Hue bridge.
**	
**	Returned string is in JSON format.
**	
****************************************************************/
//static char signallight_getHueLights(struct _hardware *hw_ptr)
//{
//}


/*****************************************************************
** static char signallight_getHueGroups(struct _hardware *hw_ptr)
** 
**	Returns all of the groups currently defined in the 
**	specified Hue bridge. 
**	
**	Returned string is in JSON format.
**	
****************************************************************/
//static char signallight_getHueGroups(struct _hardware *hw_ptr)
//{
//}


/*****************************************************************
** static char signallight_getHueLightStates(struct _hardware *hw_ptr)
** 
**	Returns all states of all of the light bulbs currently 
**	assigned to the specified Hue Bridge. 
**	
**	Returned string is in JSON format.
**	
****************************************************************/
//static char signallight_getHueLightStates(struct _hardware *hw_ptr)
//{
//}


#ifdef FUTURE_
/*****************************************************************
** static int signallight_scanForNewHueBulbs(struct _hardware *hw_ptr)
** 
**	Make the Philips Hue bridge do a scan for any new light bulbs.
**
**	This is done by initiating a blank POST command 
**	to the bridge's 'lights' API.
**	
****************************************************************/
static int signallight_scanForNewHueBulbs(struct _hardware *hw_ptr)
{
int ret = -1;

char bridge_port[3] = "80";
char bridge_command_buffer[MAX_CHARS] = "";

int socket = -1;

/* Check to make sure we have an IP address for the bridge of the light */
if(signallight_find_address(hw_ptr) == FALSE)
	{
	/* error already printed by function call */
	return(0);
	}

/* Clean up the username (API key) so we can plug it into the URL/path when we send our command (don't think we care that we're shortening the main variable in this case) */
remove_trailing_space(hw_ptr->hardware_device_username);

/* Construct POST request... example: "POST /api/0123456789/lights HTTP/1.1", etc. */
strcatl(bridge_command_buffer, "POST /api/", sizeof(bridge_command_buffer));
strcatl(bridge_command_buffer, hw_ptr->hardware_device_username, sizeof(bridge_command_buffer));
strcatl(bridge_command_buffer, "/lights", sizeof(bridge_command_buffer));
strcatl(bridge_command_buffer, " HTTP/1.1\r\n", sizeof(bridge_command_buffer));
strcatl(bridge_command_buffer, "Content-Type: application/xml\r\n", sizeof(bridge_command_buffer));
strcatl(bridge_command_buffer, "\r\n", sizeof(bridge_command_buffer));	//double line feed conventionally signals the end of the header and the start of the payload

socket = SystemSocketConnect("", hw_ptr->term_ip, bridge_port, 5, FALSE);

if(socket < 0)
	{
	/* took too long to send data to port so lets disable it */
	DIAGNOSTIC_LOG_1("signallight_scanForNewHueBulbs(): SystemSocketConnect() report error %s", HardwareReportPortError(hw_ptr));

	HardwareReportSystemAlerts(hw_ptr);
	HardwareDisablePort(hw_ptr, TRUE, TRUE);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_CLOSED);
	}
else
	{
	HardwareSystemAlertClear(hw_ptr);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_ACTIVE);

	if(SystemSocketWrite(socket, bridge_command_buffer, sizeof(bridge_command_buffer)) > 0)
		{
		if(SystemSocketReadTimeout(socket, bridge_command_buffer, sizeof(bridge_command_buffer), 5) > 0)
			{
			ret = TRUE;
			}
		}

	SystemSocketClose(socket);
	}

return ret;
}
#endif


/*****************************************************************
** static char signallight_getHueLightStates(struct _hardware *hw_ptr)
**	
**	Returns the complete state information of the specified
**	Hue light bulb on the specified bridge.
**	
**	Returned string is in JSON format.
**	
****************************************************************/
//static char signallight_getHueLightState(struct _hardware *hw_ptr, int light_number)
//{
//char ret[MAX_CHARS];

//char ip_port[3] = "80";
//char tmp[200];
//char light_command_buffer[MAX_CHARS] = "";
//int socket = -1;

/* Construct HTTP request  -- example: "GET /api/0123456789/lights/1/state HTTP/1.1"  */
//strcatl(light_command_buffer, "GET /api/", sizeof(light_command_buffer));
//remove_trailing_space(hw_ptr->hardware_device_username);
//strcatl(light_command_buffer, hw_ptr->hardware_device_username, sizeof(light_command_buffer));
//strcatl(light_command_buffer, "/lights/", sizeof(light_command_buffer));
//sprintf(tmp, "%d", light_number);
//strcatl(light_command_buffer, tmp, sizeof(light_command_buffer));
//strcatl(light_command_buffer, " HTTP/1.1\r\n\r\n", sizeof(light_command_buffer));

//return(ret);
//}


#ifdef FUTURE_
/*****************************************************************
** static int signallight_reassignHueBulbToBridge(struct _hardware *hw_ptr)
**
**	struct _hardware *hw_ptr	(pointer to the bridge hardware that you want to assign a bulb to)
**
** 	Philips Hue bulbs that come in a starter pack (at least 
**	the early versions as of 1H/2013) are tied to their own
**	bridge (the one that comes with them in the pack). So, in
**	order to assign a starter-pack bulb to a different bridge,
**	this function needs to be called. This would also apply
**	to a situation in which you may want to move one bulb to
**	another bridge, in general.
**
**	Sources:
**	 - http://www.everyhue.com/vanilla/discussion/92/link-your-living-colors-and-hue-bulbs-from-other-starter-pack-without-a-remote/p1
**	 - http://www.everyhue.com/vanilla/discussion/comment/2046#Comment_2046
**
**	Hopefully, this will get resolved and made unnecessary in future firmware?
**
** 	Process:
** 	 1) Place bulb physically within 30cm of the bridge (SERIOUSLY! Might even need to almost touch it!)
** 	 2) Telnet into the bridge on port 30000
** 	 3) Send the following command, exactly:  [Link,TouchLink]
** 	 4) After a couple seconds, the bulb should flash a couple times
** 	 5) At this point, it's either linked to the bridge or is ready for a normal scan?
**
****************************************************************/
static int signallight_reassignHueBulbToBridge(struct _hardware *hw_ptr)
{
int ret = FALSE;
int socket = -1;

char bridge_port[6] = "30000";			//this is the 'secret' maintenance port that was discovered by the community
char bridge_command[17] = "[Link,Touchlink]";	//this is the command that reassigns any bulb in proximity to a bridge

socket = SystemSocketConnect("", hw_ptr->term_ip, bridge_port, 5, FALSE);

if(socket < 0)
	{
	/* took too long to send data to port so lets disable it */
	DIAGNOSTIC_LOG_1("signallight_reassignHueBulbToBridge(): SystemSocketConnect() report error %s", HardwareReportPortError(hw_ptr));

	HardwareReportSystemAlerts(hw_ptr);
	HardwareDisablePort(hw_ptr, TRUE, TRUE);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_CLOSED);
	}
else
	{
	HardwareSystemAlertClear(hw_ptr);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_ACTIVE);

	if(SystemSocketWrite(socket, bridge_command, sizeof(bridge_command)) > 0)
		{
		if(SystemSocketReadTimeout(socket, bridge_command, sizeof(bridge_command), 10) > 0)
			{
			ret = TRUE;
			}
		}

	SystemSocketClose(socket);
	}

return(ret);
}
#endif

/*******************************************************
** int signallight_find_address(struct _hardware *hw_ptr)
**
**	This might be more intended for seeing if there is any
**	IP address AT ALL... rather than getting it for sending-
**	messages purposes.??
** 
**	return 0  no address
**	return 1 address found 
**
*******************************************************/
int signallight_find_address(struct _hardware *hw_ptr)
{
int found = FALSE;

/* just to provide a better looking log */
char bridgeid[DEVICEID_LENGTH];
strcpy(bridgeid,hw_ptr->hardware_deviceid);
remove_trailing_space(bridgeid);


/* DEV-NOTE: may need to finalize whether bridge and/or light gets IP address in its hardware definition...
 * if only bridge gets defined with it, then we'll need to get a light's IP association somehow 
 * UPDATE: Jerry says we can use the analogy of a paging transmitter and pagers, in which there is an 
 * existing function to grab the transmitter's IP using the associated nodename of the pagers? This MAY be
 * done in the place where the send_to_signallight_device function is called from? */

#ifdef linux
if(notjustspace(hw_ptr->term_ip, IP_LENGTH))
	{
	found = TRUE;
	}
else	/*IP address is not defined (so try to find it?) */
	{
	/* DEV-NOTE: possibilities for finding it include: 
	 *  1) Do a 'broadcast' ping then 'arp -a' and cross-reference MAC 
	 *  2) Use Philips' service (do a GET from www.meethue.com/api/nupnp) */

	/* DEV-NOTE: once you find it, be sure to put it in hw_ptr->term_ip so it can be used? (see example syntax below) */
	//strcpyl(hw_ptr->term_ip, [ip_data], IP_LENGTH);
	//remove_trailing_space(hw_ptr->term_ip); 
	//found = TRUE; */

	found = FALSE;

	if(found == FALSE)
		{
		DIAGNOSTIC_LOG_1("signallight_find_address(): ERROR no IP address found for '%s'", hw_ptr->hardware_deviceid);
		}
	//else
	//	{
	//	DIAGNOSTIC_LOG_2("Signal Light: %s using address %s", bridgeid, hw_ptr->term_ip);
	//	}
	}
#endif

return(found);
}


/*****************************************************************
 * int hue_bulb_is_currently_on(struct _hardware *hw_ptr, int light_number)
 *
 *	Returns whether a particular bulb is currently reporting
 *	a status of on or not.
 *
 ****************************************************************/
//int hue_bulb_is_currently_on(struct _hardware *hw_ptr, int light_number)
//{
//int ret = -1;	//represents no data returned or some other error

//char str_json[MAX_CHARS] = "";

//strcatl(str_json, signallight_getHueLightState(hw_ptr, light_number), sizeof(str_json));

/* Parse the JSON string that was returned, to see whether the light has a true on state */

//return(ret);
//}


/*****************************************************************
 * const char determine_signallight_cmd_str(int dwc_message_type)
 *
 * 	Takes the banner/wtc command as an argument and
 * 	returns	the light's appropriate JSON command string.
 *
 * 	Note: this function will probably be mostly agnostic as
 * 	to whether the end-result is flashing, steady, etc. 
 * 	In the case of a flashing light, just think of this 
 * 	as the initial "on" state. Except for fading, which is
 * 	unique in that it fades on instead of instant on.
 *
 ****************************************************************/
const char *determine_signallight_cmd_str(int message_type)
{
switch(message_type)
	{
	case BANNER_SIGNALLIGHT_CMD_NONE:
		return 0;

	case BANNER_SIGNALLIGHT_CMD_OFF:
		return STR_SIGNALLIGHT_HUE_OFF_FADEDEF;

	case BANNER_SIGNALLIGHT_CMD_ON_STANDBY:
		return STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_UV;

	case BANNER_SIGNALLIGHT_CMD_ON_BLUE_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_BLUE;
	case BANNER_SIGNALLIGHT_CMD_ON_BLUE_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_BLUE;
	case BANNER_SIGNALLIGHT_CMD_ON_BLUE_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_BLUE:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_BLUE;
	case BANNER_SIGNALLIGHT_CMD_FADING_BLUE:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_BLUE;

	case BANNER_SIGNALLIGHT_CMD_ON_GREEN_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_GREEN;
	case BANNER_SIGNALLIGHT_CMD_ON_GREEN_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_GREEN;
	case BANNER_SIGNALLIGHT_CMD_ON_GREEN_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_GREEN:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_GREEN;
	case BANNER_SIGNALLIGHT_CMD_FADING_GREEN:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_GREEN;

	case BANNER_SIGNALLIGHT_CMD_ON_ORANGE_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_ORANGE;
	case BANNER_SIGNALLIGHT_CMD_ON_ORANGE_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_ORANGE;
	case BANNER_SIGNALLIGHT_CMD_ON_ORANGE_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_ORANGE:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_ORANGE;
	case BANNER_SIGNALLIGHT_CMD_FADING_ORANGE:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_ORANGE;

	case BANNER_SIGNALLIGHT_CMD_ON_PINK_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PINK;
	case BANNER_SIGNALLIGHT_CMD_ON_PINK_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PINK;
	case BANNER_SIGNALLIGHT_CMD_ON_PINK_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_PINK:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PINK;
	case BANNER_SIGNALLIGHT_CMD_FADING_PINK:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PINK;

	case BANNER_SIGNALLIGHT_CMD_ON_PURPLE_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PURPLE;
	case BANNER_SIGNALLIGHT_CMD_ON_PURPLE_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PURPLE;
	case BANNER_SIGNALLIGHT_CMD_ON_PURPLE_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_PURPLE:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PURPLE;
	case BANNER_SIGNALLIGHT_CMD_FADING_PURPLE:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PURPLE;

	case BANNER_SIGNALLIGHT_CMD_ON_RED_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_RED;
	case BANNER_SIGNALLIGHT_CMD_ON_RED_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_RED;
	case BANNER_SIGNALLIGHT_CMD_ON_RED_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_RED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_RED;
	case BANNER_SIGNALLIGHT_CMD_FADING_RED:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_RED;

	case BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITECOOL;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITECOOL;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_WHITECOOL:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITECOOL;
	case BANNER_SIGNALLIGHT_CMD_FADING_WHITECOOL:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITECOOL;

	case BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEPURE;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEPURE;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_WHITEPURE:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEPURE;
	case BANNER_SIGNALLIGHT_CMD_FADING_WHITEPURE:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEPURE;

	case BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEWARM;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEWARM;
	case BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_WHITEWARM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEWARM;
	case BANNER_SIGNALLIGHT_CMD_FADING_WHITEWARM:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEWARM;

	case BANNER_SIGNALLIGHT_CMD_ON_YELLOW_DIM:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_YELLOW;
	case BANNER_SIGNALLIGHT_CMD_ON_YELLOW_MED:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_YELLOW;
	case BANNER_SIGNALLIGHT_CMD_ON_YELLOW_BRI:
	case BANNER_SIGNALLIGHT_CMD_FLASHING_YELLOW:
		return STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_YELLOW;
	case BANNER_SIGNALLIGHT_CMD_FADING_YELLOW:
		return STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_YELLOW;

	default:
		DIAGNOSTIC_LOG("ERROR: support_signallight.c: determine_signallight_cmd_str() received disallowed or invalid command as an argument");
		return 0;
	}
}


/*****************************************************************
** int send_to_signallight_device(struct _hardware *hw_ptr, int light_number, int message_type)
**
**	Will send commands and data to the signal-light device
**
** return: normally 0 - on for BANNER_IPSPEAKER_CHECKING_CONNECT is 0 or 1.
**
*******************************************************************/
int send_to_signallight_device(struct _hardware *hw_ptr, int light_number, const char *bann_light_command)
{
int ret = 0;

char ip_port[3] = "80";
char tmp[200];
char light_command_buffer[MAX_CHARS] = "";
char light_command_buffer_payload[MAX_CHARS] = "";

int socket = -1;

/* Check to make sure we have an IP address for the bridge of the light */
if(signallight_find_address(hw_ptr) == FALSE)
	{
	/* error already printed by function call */
	return(0);
	}

/* Clean up the username (API key) so we can plug it into the URL/path when we send our command */
remove_trailing_space(hw_ptr->hardware_device_username);

/* Construct the HTTP request payload that will get sent to the light(s) */
strcatl(light_command_buffer_payload, bann_light_command, sizeof(light_command_buffer_payload));

/* Construct PUT request... example: "PUT /api/0123456789/lights/1/state HTTP/1.1", etc. */
strcatl(light_command_buffer, "PUT /api/", sizeof(light_command_buffer));
strcatl(light_command_buffer, hw_ptr->hardware_device_username, sizeof(light_command_buffer));
strcatl(light_command_buffer, "/lights/", sizeof(light_command_buffer));
sprintf(tmp, "%d", light_number);
strcatl(light_command_buffer, tmp, sizeof(light_command_buffer));
strcatl(light_command_buffer, "/state HTTP/1.1\r\n", sizeof(light_command_buffer));
sprintf(tmp, "Content-Length: "FORMAT_STRLEN_STR"\r\n", strlen(light_command_buffer_payload));
strcatl(light_command_buffer, tmp, sizeof(light_command_buffer));
strcatl(light_command_buffer, "Content-Type: application/xml\r\n", sizeof(light_command_buffer));
strcatl(light_command_buffer, "\r\n", sizeof(light_command_buffer));	//double line feed conventionally signals the end of the header and the start of the payload
strcatl(light_command_buffer, light_command_buffer_payload, sizeof(light_command_buffer));

/* Handle sending the PUT request */
socket = SystemSocketConnect("", hw_ptr->term_ip, ip_port, 5, FALSE);

if(socket < 0)							// if the socket connection was not successful
	{
	/* took too long to send data to port so lets disable it */
	DIAGNOSTIC_LOG_1("send_to_signallight_device(): SystemSocketConnect() report error %s", HardwareReportPortError(hw_ptr));

	HardwareReportSystemAlerts(hw_ptr);
	HardwareDisablePort(hw_ptr, TRUE, TRUE);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_CLOSED);
	}
else
	{
	HardwareSystemAlertClear(hw_ptr);
	HardwareUpdateDeviceStatus(hw_ptr, DEVICE_CONNECTION_ACTIVE);

	if(SystemSocketWrite(socket, light_command_buffer, sizeof(light_command_buffer)) > 0)
		{
		if(SystemSocketReadTimeout(socket, light_command_buffer, sizeof(light_command_buffer), 5) > 0)
			{
			ret = TRUE;
			}
		}

	SystemSocketClose(socket);
	}

mn_delay(100);

return(ret);
}

/***SAMPLE PUT REQUEST-HEADER:*************************************/
// PUT /api/0123456789/lights/1/state HTTP/1.1
// Host: 192.168.1.221
// Connection: keep-alive
// Content-Length: 13
// Cache-Control: no-cache
// Pragma: no-cache
// Origin: http://192.168.1.221
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.22 (KHTML, like Gecko) Chrome/25.0.1364.172 Safari/537.22
// Content-Type: application/xml
// Accept: */*
// Referer: http://192.168.1.221/debug/clip.html
// Accept-Encoding: gzip,deflate,sdch
// Accept-Language: en-US,en;q=0.8
// Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3
/***SAMPLE PUT REQUEST-PAYLOAD:************************************/
// {"bri":255}
/***SAMPLE PUT REQUEST-RESPONSE:***********************************/
// HTTP/1.1 200 OK
// Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0
// Pragma: no-cache
// Expires: Mon, 1 Aug 2011 09:00:00 GMT
// Connection: close
// Access-Control-Max-Age: 0
// Access-Control-Allow-Origin: *
// Access-Control-Allow-Credentials: true
// Access-Control-Allow-Methods: POST, GET, OPTIONS, PUT, DELETE
// Access-Control-Allow-Headers: Content-Type
// Content-type: application/json
