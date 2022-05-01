/**********************************************************************
**  Module:     support.c
**
**  Author:     (Redacted), Chris Rider
**          Copyright (c) 1991-2021
**
**********************************************************************/

#define DEBUG_LOG_SEQUENCING 0

/***** REDACTED *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

#include <unistd.h>
#include <time.h>

/***** REDACTED *****/

#include "support_signallight.h"

/***** REDACTED *****/

struct _event_check event_check[] = 
	{

	/***** REDACTED *****/

	{"SIGNAL LIGHT", EVENT_TYPE_SIGNAL_LIGHT_STOP, EVENT_STOP, 0},
	{NULL, 0, 0, 0}
	};

char banner_phone_site[SITE_LENGTH] = "";

static struct _intercom_zone *intercom_head;		/* intercom head linked list */

static struct _hardware *hardware_head;			/* hardware head linked list */
static struct _hardware *hw_ptr_verify_write;		/* hardware board verify continue writing */

#ifdef MSGNET_WIN32
char gr_ams7_buf[GR_NUMBER_PER_MSG][2];  		/* just need values no real data */
char gr_ams16_buf[GR_NUMBER_PER_MSG][2];     			
char gr_ams24_buf[GR_NUMBER_PER_MSG][2];     			
#else
char gr_ams7_buf[GR_NUMBER_PER_MSG][MAX_CHARS]; 	/* format buffer for 7 high ams */
char gr_ams16_buf[GR_NUMBER_PER_MSG][MAX_CHARS];	/* format buffer for 16 high ams */
char gr_ams24_buf[GR_NUMBER_PER_MSG][MAX_CHARS];	/* format buffer for 24 high ams */
#endif

char buffer[MAX_CHARS];     				/* storage for sending out messages */
char ba_buf[MAX_CHARS];     				/* format buffer for alpha boards */
char bb_buf[MAX_CHARS];     				/* format buffer for beta brite boards */
char bc_buf[MAX_CHARS];     				/* format buffer for color cell boards */
char ph14_buf[MAX_CHARS];   				/* format buffer for phoenix boards 14 character */
char ph20_buf[MAX_CHARS];   				/* format buffer for phoenix boards 20 character */
char sign_buf[MAX_CHARS];   				/* format buffer for all sign boards */
char multimedia_buf[MAX_CHARS];
char virtual_buf[MAX_CHARS];				/* format buffer for PC Alert boards */
char alert_buffer[MAX_CHARS];   			/* formatted alert message */
char formatbuf[MAX_CHARS];  				/* format buffer */
char camera_view[200] = "";

static int camera_show_flag = BB_CHOICE_NO;
static DBRECORD camera_record_number = 0;		/* camera to show for a message */

const char banb_sync_string[] = "\x01\x01\x01\x01\x01";
const char phoenix_sync_string[] = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xbb";
const char phoenix_time_sync_string[] = "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa";

int current_message_mode;
int current_message_position;

DBRECORD bbs_allsigns_recno = 0L;			/* record number for bbs_allsigns display location */
DBRECORD bbs_location_pc_alert_recno = 0L;		/* record number for bbs_destin_location_of_pc_alert display location */
DBRECORD bbs_all_pcalerts_recno = 0L;			/* record number for bbs_all_pcalerts display location */

int chain_skip_setvar_launch;				/* flag to skip setvar at launch SETVARL as the cgi has already done the setvar at launch (user may have changed the setvar values so use the new values dont reset again) */
char chain_from_pin[PIN_LENGTH];			/* who is sending the message */
char chain_yesno;       				/* Y if yes, N if no, D dont know, 0 otherwise */
char chain_testoverride;				/* test override setting for message in stream */
char chain_command_list[COMMAND_LIST_LENGTH];
char chain_list_directory[DIRECTORY_LENGTH];
char chain_label[LABEL_LENGTH];
char chain_stream_name[RECORD_AND_DTSEC_LENGTH];	/* stream name to send to launch codes */
char chain_help_note;
char chain_send_note;
char chain_reply_note;

static DBRECORD chain_help_note_record_number;
static DBRECORD chain_send_note_record_number;
static DBRECORD chain_reply_note_record_number;
static DBRECORD chain_attachment_record_number;
static DBRECORD chain_pre_attachment_record_number;
DBRECORD chain_link_parent;				/* bann record number of record that the command list is operating on */
DBRECORD chain_stream_number;				/* stream number to send to launch codes */
DBRECORD chain_parent_record;

DBRECORD chain_thread_recno;
UCHAR chain_thread_dtsec[DTSEC_LENGTH];

int reading_in_active_records = FALSE;  		/* true when restarting and reading ACTIVE records */

DTSEC general_schedule_dtsec = 0L;			/* next time to execute general schedules */

int tap_ack_timeout;					/* TAP protocol timeout */

#ifdef MSGNET_WIN32
int banner_forked_child_pid;				/* forked child number */
#else
pid_t banner_forked_child_pid;				/* forked child number */
#endif

#ifndef USHORT
#define USHORT unsigned short
#endif

/* prototypes */
/***** REDACTED *****/
static int BannerSignalLightDurationIsDone(void);

/***** REDACTED *****/

/******************************************************************
** static int TypeIsSignalLight(int type)
**
**  Signal light (such as Philips Hue)
**
******************************************************************/
static int TypeIsSignalLight(int type)
{
int ret = FALSE;

#ifdef USE_HUE_LIGHT
switch(type)
	{
	case DEVICE_HUE_LIGHT:
	case DEVICE_HUE_BRIDGE:
		ret = TRUE;
		break;

	default:
		ret = FALSE;
		break;
	}
#endif

return(ret);
}

/***** REDACTED *****/

/******************************************************************
** static int TypeIsSlowDevice(int type)
**
******************************************************************/
static int TypeIsSlowDevice(int type)
{
int ret;

if(check_fork_devices())
	{
	ret = TRUE;
	}
/***** REDACTED *****/
else if(TypeIsSignalLight(type))
	{
	return(TRUE);
	}

/***** REDACTED *****/

return(ret);
}

/***** REDACTED *****/

/********************************************************************
** static int pop_fifo_and_display(struct _hardware *hw_ptr, int active_seq, int force_pop_as_requested_from_keypad)
**
**	If active_seq is >=0 then dont pop anything off fifo if < 0 then pop.
**
********************************************************************/
static int pop_fifo_and_display(struct _hardware *hw_ptr, int active_seq, int force_pop_as_requested_from_keypad)
{
int seq;

char formatbuf[MAX_SIGN_SEQUENCE];

DIAGNOSTIC_FUNCTION("pop_fifo_and_display");

if(active_seq >= 0)
	{
	seq = active_seq;
	}

	/***** REDACTED *****/

		else if(TypeIsSignalLight(hw_ptr->type))
              		{
			#ifdef USE_INT64
			/* CHRIS */
			if(BannerSignalLightDurationIsDone() == FALSE)		/* if the defined 'light duration' has not yet passed... go ahead an activate the light */
				{
				struct _hardware *bridge_hw_ptr;
	
				bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light is associated with (we will need its IP and username to do anything with the light)
		
				if(bridge_hw_ptr)
					{
					send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
					if(DiagnosticCheck(DIAGNOSTIC_SIGNAL_LIGHTS)) DIAGNOSTIC_LOG_2("pop_fifo_and_display() called for signal light #%d (dbb_light_signal = %d)", hw_ptr->network_address, db_bann->dbb_light_signal);
					}
				else
					{
					DIAGNOSTIC_LOG_2("ERROR, no bridge_hw_ptr: pop_fifo_and_display(): Signal light #%d (%s) failed to resolve controller", hw_ptr->network_address, hw_ptr->hardware_deviceid);
					}
				}
			#endif
			}
		
		/***** REDACTED *****/

return(seq);
}

/***** REDACTED *****/

/*********************************************************************
** static int banner_any_messages_on_board(struct _hardware *hw_ptr, DBRECORD specific_record)
**
**	return TRUE if any messages showing on this board.
**
*********************************************************************/
static int banner_any_messages_on_board(struct _hardware *hw_ptr, DBRECORD specific_record)
{
int i;
int ret = FALSE;

if(hw_ptr && hw_ptr->board_ptr)
	{
	for(i = 0; i < hw_ptr->max_seq; i++)
		{
		if(specific_record > 0)
			{
			if(hw_ptr->board_ptr[i].bann_recno == specific_record)
				{
		        	ret = TRUE;
			        break;
				}
			}
		else if(TypeIsSignalLight(hw_ptr->type)
			&& hw_ptr->board_ptr[i].bann_recno > 0
			&& hw_ptr->board_ptr[i].signal_light_duration > 0 
			&& hw_ptr->board_ptr[i].hide_message == MESSAGE_SHOW
			&& hw_ptr->board_ptr[i].display_dtsec + hw_ptr->board_ptr[i].signal_light_duration <= cur_time)
			{
			/* this light duration is expired, so not really considered any message */
			}
		else if(hw_ptr->board_ptr[i].bann_recno > 0)
			{
		        ret = TRUE;
		        break;
			}
		}
	}

return(ret);
}

/***** REDACTED *****/

/********************************************************************
** static void BannerSignalLightAddStopEvent(int event_stop, int message_was_stopped, DBRECORD recno)
**
**	set timer for duration of signal Light
**
********************************************************************/
static void BannerSignalLightAddStopEvent(int event_stop, int message_was_stopped, DBRECORD recno)
{
#ifdef USE_INT64
if(event_stop || message_was_stopped)
	{
	struct _hardware *hw_ptr;

	DBRECORD cur_recno = db_bann_getcur();

	EventPurgeRecords(recno, EVENT_FOR_SIGNAL_LIGHT);

	/* stop_message() was called so also stop the contact, only if it was turned ON by this message */
	for(hw_ptr = hardware_head; hw_ptr; hw_ptr = hw_ptr->next)
	       	{
		if(TypeIsSignalLight(hw_ptr->type))
			{
			int i;

			struct _hardware *bridge_hw_ptr;

			bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light is associated with (we will need its IP and username to do anything with the light)
	
			if(bridge_hw_ptr)
				{
				int found = FALSE;
				int clear = FALSE;
				int any_signal_light = -1;

				/* see if there is ANY other signal lights still active */
				for(i = 0; i < hw_ptr->max_seq; i++)
					{
					if(hw_ptr->board_ptr[i].bann_recno > 0)
						{
						if(DiagnosticCheck(DIAGNOSTIC_SIGNAL_LIGHTS)) DIAGNOSTIC_LOG_3("BannerSignalLightAddStopEvent() deviceid=%s recno=%d light_duration=%d", hw_ptr->hardware_deviceid, hw_ptr->board_ptr[i].bann_recno, hw_ptr->board_ptr[i].signal_light_duration);
						}

					if(hw_ptr->board_ptr
						&& hw_ptr->board_ptr[i].bann_recno > 0
						&& hw_ptr->board_ptr[i].bann_recno == recno)
						{
						/* this is the one we are stopping */
						clear = TRUE;
						}
					else if(hw_ptr->board_ptr
						&& hw_ptr->board_ptr[i].bann_recno > 0
						&& hw_ptr->board_ptr[i].signal_light_duration > 0 
						&& hw_ptr->board_ptr[i].display_dtsec + hw_ptr->board_ptr[i].signal_light_duration > cur_time)
						{
						found = TRUE;
						any_signal_light = i;
						}
					}

				if(found)
					{
					/* send message to revert signal light */
					db_bann_setcur(hw_ptr->board_ptr[any_signal_light].bann_recno);
					send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
					}
				else if(clear)
					{
					if(find_record_in_db_bann(hw_ptr->default_directory, bb_valid_types[BBT_LF].string, hw_ptr->default_msg_name))
						{
						send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
						}
					else if(find_record_in_db_bann(db_sysp->dsy_default_msg_directory, bb_valid_types[BBT_LF].string, db_sysp->dsy_default_msg_name))
						{
						send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
						}
					else
						{
						DIAGNOSTIC_LOG_1("BannerSignalLightAddStopEvent() no default message found for %s", hw_ptr->hardware_deviceid);
						}
					}

				if(found || clear)
					{
					if(DiagnosticCheck(DIAGNOSTIC_SIGNAL_LIGHTS)) DIAGNOSTIC_LOG_3("BannerSignalLightAddStopEvent() deviceid=%s found=%d clear=%d", hw_ptr->hardware_deviceid, found, clear);
					}
				}
			}
		}

	db_bann_setcur(cur_recno);
	}
else if(db_bann->dbb_light_duration > 0
	&& BannerSignalLightDurationIsDone() == FALSE)
	{
	long duration;

	char stop_dtsec[DTSEC_LENGTH];

	if(db_bann->dbb_light_duration == 0)
		{
 		duration = db_bann->dbb_duration;		/* message duration */
		}
	else
		{
 		duration = db_bann->dbb_light_duration;		/* light duration */
		}

	get_dtsec_plus_offset(stop_dtsec, duration);
	EventPurgeRecords(db_bann_getcur(), EVENT_FOR_SIGNAL_LIGHT);
	banner_event_add(EVENT_TYPE_SIGNAL_LIGHT_STOP, EVENT_SIGNAL_LIGHT, EVENT_FOR_SIGNAL_LIGHT, NULL, stop_dtsec, FALSE, FALSE, silentm_sys_admin_pin, NULL, 0);
	EventCheckReset(EVENT_TYPE_SIGNAL_LIGHT_STOP);
	}
#endif
}

/***** REDACTED *****/

/********************************************************************
** static int stop_message(DBRECORD stop_recno, struct _hardware *hw_port_ptr, int seq, int fifo_seq, int remove, int update_type, int single, DBRECORD lst_class_parent_recno)
**
**  Will stop a message from being displayed and if remove is true
**  will also delete the record from the database or if update is 
**  true will update the database entry clearing out the seq# etc.
**
**  returns:
**      true when message deleted due to responded df_respond_directory.
**
********************************************************************/
static int stop_message(DBRECORD stop_recno, struct _hardware *hw_port_ptr, int seq, int stop_seq, int remove, int update_type, int single, DBRECORD lst_class_parent_recno)
{
int ret = FALSE;
int message_was_from_cc = 0;

/***** REDACTED *****/

	/* need to check every board_ptr and fifo_ptr to see if we are stopping it */
	for(hw_ptr = hardware_head; hw_ptr; hw_ptr = hw_ptr->next)
	    {
		
		/***** REDACTED *****/

					#ifdef USE_HUE_LIGHT
					if(hw_ptr->type == DEVICE_HUE_LIGHT)
						{
						#ifdef USE_INT64
						/* DEV-NOTE: do a test here with seq to avoid sending stop to lights if they already have some other message active -- but gdb reports seq not having anything?! */
						struct _hardware *bridge_hw_ptr;

						/*DEV-NOTE: For future makes/models of lights, the following logic may need to be adapted ~ especially if a signal light device doesn't use a bridge or controller (intermediary) device.*/
						bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light bulb is associated with (we will need its IP and username to do anything with the light)
	
						/* if we got a valid pointer to the hue light's bridge, then send the command to possibly do something to the light (the logic to check whether to actually act on it will be handled in check_banner_node_commands) */
						if(bridge_hw_ptr)
							{
							DIAGNOSTIC_LOG_4("Signal light stop_message() for light #%d (%s) on bridge IP %s (%s)", hw_ptr->network_address, hw_ptr->hardware_deviceid, bridge_hw_ptr->term_ip, bridge_hw_ptr->hardware_deviceid);

							send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, BANNER_SIGNALLIGHT_CMD_STOP);	//request the banner node to see if this light needs to be stopped (if the light doesn't have his msg recno, then it won't need action - all handled in check_banner_node_commands)

//DIAGNOSTIC_LOG_2("ZZZ %d | %d", hw_ptr->priority, hw_ptr->lowest_priority_displayable);
							hw_ptr->priority = hw_ptr->lowest_priority_displayable;						//CR-NOTE: not sure exactly what this does, but it's needed in order for priority on a single light bulb to work
							}
						else
							{
							/* this branch should only happen if, by some odd chance, this light bulb doesn't have a transmitter/controller/bridge defined */
							DIAGNOSTIC_LOG_2("ERROR, no bridge_hw_ptr: stop_message(): Signal light #%d (%s) failed to resolve controller", hw_ptr->network_address, hw_ptr->hardware_deviceid);
							}
						#endif
						}
					else
					#endif

			/***** REDACTED *****/

	/* reset after sending command to stop */
	BannerSignalLightAddStopEvent(FALSE, TRUE, recno);

	/***** REDACTED *****/

return(ret);
}

/***** REDACTED *****/

/********************************************************************
** void send_to_banner_node(struct _hardware *hw_ptr, char *ptr, int sequence_number, int message_type)
**
**	Send the information to a client banner node.
**
**	sequence_number is sequence numbers to show.
**
*********************************************************************/
void send_to_banner_node(struct _hardware *hw_ptr, char *ptr, int sequence_number, int message_type)
{
int i;
int comm_port;
int winclient;
int baudrate;
int number_of_sends = 1;
int send_wake_up = TRUE;

/***** REDACTED *****/

	else if(TypeIsSignalLight(hw_ptr->type))
		{
		#ifdef USE_INT64
		/* pass the message's record number to the forked banner process */
		wtc_stream_number = db_bann_getcur();

		/* if field value is 0 (meaning "Msg Dur"), use the message's duration... else, just use the value specified by this "Light Duration" field */
		if(db_bann->dbb_light_duration == 0)
			{
			db_wtc->dwc_baudrate = db_bann->dbb_duration;		//pass our desired light duration (which, in this case, is the message duration) to the forked banner process
			}
		else
			{
			db_wtc->dwc_baudrate = db_bann->dbb_light_duration;	//pass our desired light duration to the forked banner process
			}

		hw_ptr->update = FALSE;
		#endif
		}

/***** REDACTED *****/

send_to_banner_node_skip_to_end:
if(send_wake_up)
	{
	/***** REDACTED *****/
	}
}

/********************************************************************
** static void banner_hardware_reset_sign(struct _hardware *hw_ptr)
**
**	hardware reset is acheived by writing no data
**	to the memory configuration register.
**
********************************************************************/
static void banner_hardware_reset_sign(struct _hardware *hw_ptr)
{

/***** REDACTED *****/

else if(TypeIsSignalLight(hw_ptr->type))
	{
	DIAGNOSTIC_LOG("banner_hardware_reset_sign() called");
	}

/***** REDACTED *****/

}

/********************************************************************
** void banner_software_reset_sign(struct _hardware *hw_ptr)
**
********************************************************************/
void banner_software_reset_sign(struct _hardware *hw_ptr)
{

/***** REDACTED *****/

else if(TypeIsSignalLight(hw_ptr->type))
	{
	DIAGNOSTIC_LOG("banner_software_reset_sign() called");
	}

/***** REDACTED *****/

}

/***** REDACTED *****/

/*********************************************************************
** static int banner_clear(struct _hardware *hw_ptr)
**
**  Clear the indicated banner board by location or if location is
**  NULL then clear all the boards that are active.
**
**  1) look to the individual hardware board for a msg type and name.
**  2) look to the KF msg type
**  3) generate the default "..."
**
*********************************************************************/
static int banner_clear(struct _hardware *hw_ptr)
{

/***** REDACTED *****/

	else if(TypeIsSignalLight(hw_ptr->type))
		{
		#ifdef USE_INT64
		/* CHRIS Note: At this point, the loaded db_bann struct should contain our default message data (db_bann->dbb_light_signal) */

		struct _hardware *bridge_hw_ptr;

		bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light is associated with (we will need its IP and username to do anything with the light)

		if(bridge_hw_ptr)
			{
			/* send the defined default light state to the light... */
			send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
			if(DiagnosticCheck(DIAGNOSTIC_SIGNAL_LIGHTS)) DIAGNOSTIC_LOG_2("banner_clear() called for signal light #%d (dbb_light_signal = %d)", hw_ptr->network_address, db_bann->dbb_light_signal);
			}
		else
			{
			DIAGNOSTIC_LOG_2("ERROR, no bridge_hw_ptr: banner_clear(): Signal light #%d (%s) failed to resolve controller", hw_ptr->network_address, hw_ptr->hardware_deviceid);
			}

		hw_ptr->priority = LOWEST_PRIORITY;
		#endif
		}

/***** REDACTED *****/

return(FALSE);
}

/***** REDACTED *****/

/********************************************************************
** static int load_banner_data(struct _hardware *hw_ptr, int seq, int length, int fifo_seq, DBRECORD stream)
**
********************************************************************/
static int load_banner_data(struct _hardware *hw_ptr, int seq, int length, int fifo_seq, DBRECORD stream)
{

/***** REDACTED *****/

	#ifdef USE_HUE_LIGHT
	#ifdef USE_INT64
	if(db_bann->dbb_light_signal > BANNER_SIGNALLIGHT_CMD_NONE)
		{
		if(db_bann->dbb_light_duration == 0)
			{
		        board_ptr->signal_light_duration = db_bann->dbb_duration;
			}
		else
			{
		        board_ptr->signal_light_duration = db_bann->dbb_light_duration;
			}
		}
	else
		{
	        board_ptr->signal_light_duration = 0;
		}
	#endif
	#endif

/***** REDACTED *****/

board_ptr->signal_light_msg_active = 0;

/***** REDACTED *****/

return(0);
}

/***** REDACTED *****/

/*********************************************
** static void BannerUseSIGNID(struct _hardware *hw_ptr, int signid)
**
********************************************/
static void BannerUseSIGNID(struct _hardware *hw_ptr, int signid)
{

/***** REDACTED *****/

		/* CHRIS Multiple Hue lights may reside on a single bridge's IP address */
		#ifdef USE_INT64
		if(hw_ptr->type == DEVICE_HUE_BRIDGE
			&& notjustspace(db_hard->dhc_terminal_server_ip, IP_LENGTH) == FALSE)
			{
			/* we have to lookup the HUE addresses */
			if(signallight_find_address(hw_ptr))
				{
				strcpyl(db_hard->dhc_terminal_server_ip, hw_ptr->term_ip, IP_LENGTH);
				mn_snprintf(db_hard->dhc_terminal_server_port, IPPORT_LENGTH, "%d", hw_ptr->term_port);
				}
			}
		#endif

		/***** REDACTED *****/
    
terminate_msg();
}

/***** REDACTED *****/

/**********************************************************************
** static int BannerSignalLightDurationIsDone(void)
**
**	if restarting and reading active records and light duration
**	time is done (a BEGINX type thing) then skip return done.
**
**********************************************************************/
static int BannerSignalLightDurationIsDone(void)
{
int ret = FALSE;

#ifdef USE_INT64
if(AlphaToDTSEC(db_bann->dbb_rec_dtsec) + db_bann->dbb_light_duration < cur_time)
	{
	ret = TRUE;
	}
#endif

return(ret);
}

/**********************************************************************
** static _hardware *BannerFindMatchingAVPAContact(struct _hardware *hw_match)
**
**	Find the matching AVPA contact for this device or IPSPEAKER
**
**********************************************************************/
static struct _hardware *BannerFindMatchingAVPAContact(struct _hardware *hw_match)
{
struct _hardware *ret = NULL;
struct _hardware *hw_ptr;

for(hw_ptr = BannerGetHardwareHead(); hw_ptr; hw_ptr = hw_ptr->next)
	{
	if(hw_ptr->type == DEVICE_AND_CONTACT_OUT
		&& !strcmp(hw_match->hardware_device_username, hw_ptr->hardware_device_username))
		{
		ret = hw_match;
		break;
		}
	}

return(ret);
}

/**********************************************************************
** static int display_message(int clear_message_already_on_board, struct _hardware *hw_port_ptr, int pop_seq, int sound, int nosigns, char *location, int signid, int showdisplaygroups, int blank_message, int length, int *message_was_updated, int stream_record, DBRECORD device_record_number, int distribution_list_is_lsi, int load_directly_on_fifo, int sigle_rss_feed_post)
**
** Return:
**	if destination is found or not.
**
***********************************************************************/
static int display_message(int clear_message_already_on_board, struct _hardware *hw_port_ptr, int pop_seq, int sound, int nosigns, char *location, int signid, int showdisplaygroups, int blank_message, int length, int *message_was_updated, int stream_record, DBRECORD device_record_number, int distribution_list_is_lsi, int load_directly_on_fifo, int single_rss_feed_post)
{

/***** REDACTED *****/

			#ifdef USE_HUE_LIGHT
			else if(hw_ptr->type == DEVICE_HUE_LIGHT)		/* if this message includes a signal light device, */
				{
				#ifdef USE_INT64
       		     		if(db_bann->dbb_priority >= hw_ptr->lowest_priority_displayable 
       		     			&& db_bann->dbb_priority >= hw_ptr->priority)
					{
					if(BannerSignalLightDurationIsDone() == FALSE)		/* if the defined 'light duration' has not yet passed... go ahead an activate the light */
						{
						/*DEV-NOTE: For future makes/models of lights, the following logic may need 
						 * to be adapted ~ especially if a signal light device doesn't use a bridge
						 * or controller (intermediary) device. */
	
						struct _hardware *bridge_hw_ptr;
	
						/* hw_ptr is our light... using that, go find the bridge that it's associated with */
						bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light is associated with (we will need its IP and username to do anything with the light)
					
						/* if we got a valid pointer to the hue light bridge, send the command to do something to its light(s) */
						if(bridge_hw_ptr)
							{
							send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal); /*DEV-NOTE: make sure we're using a properly formatted data in dbb_light_signal -- integer vs. alpha? */
							}
		
						hw_ptr->distr_message_already_on_board = TRUE;
						hw_ptr->priority = db_bann->dbb_priority;
						hw_ptr->update = FALSE;
						}
					}

				#endif
				}
			#endif
			
/***** REDACTED *****/

return(destination_not_found);
}

/***** REDACTED *****/

/*********************************************************************
** static int banner_process_event(int struct_index, int *intercom_busy)
**
**	Handle events that are kicking off.
**
**	Return 0 go ahead and adjust next dtsec time.
**	Return 1 next dtsec is already set.
**
*********************************************************************/
static int banner_process_event(int struct_index, int *intercom_busy)
{

/***** REDACTED *****/

else if(db_evnt->det_type[0] == EVENT_TYPE_SIGNAL_LIGHT_STOP)
	{
	if(db_evnt->det_start_stop[0] == EVENT_STOP)
		{
		db_event_delete();

		BannerSignalLightAddStopEvent(TRUE, FALSE, recno);
		}
	}

/***** REDACTED *****/

}

/***** REDACTED *****/

/*******************************************************************
** static int update_seq_numbers_inline(char *ptr, char *only_seq_letters, struct _hardware *hw_ptr, int new_sequence_number, int message_type)
**
**  Will attach the new seq of messages (for board type) to ptr.
**
**  new_sequence_number of -1 will order sequences by dtsec. new_sequence_number
**  of 0 or more will put that sequence number first then order the remaining
**  by dtsec.
**
**  ptr is the start of where the seq information will go.
**
**  returns: 0 for all ok, 1 for board needs cleared.
**
*******************************************************************/
static int update_seq_numbers_inline(char *ptr, char *only_seq_letters, struct _hardware *hw_ptr, int new_sequence_number, int message_type)
{

/***** REDACTED *****/

/* look through sequence numbers for any data in them */
for(i = 0; i < hw_ptr->max_seq; i++)
	{
	/* if there is a message at this seq then display it */
	if(TypeIsSignalLight(hw_ptr->type)
		&& hw_ptr->board_ptr[i].bann_recno > 0
		&& hw_ptr->board_ptr[i].signal_light_duration > 0 
		&& hw_ptr->board_ptr[i].hide_message == MESSAGE_SHOW
		&& hw_ptr->board_ptr[i].display_dtsec + hw_ptr->board_ptr[i].signal_light_duration <= cur_time)
		{
		/* this light duration is expired */
		}
	
/***** REDACTED *****/

return(ret);
}

/*******************************************************************
** int update_seq_numbers(struct _hardware *hw_port_ptr)
**
**  If hw_ptr is NULL then look to all attached boards otherwise only
**  check the requested board.
**
*******************************************************************/
int update_seq_numbers(struct _hardware *hw_port_ptr)
{

/***** REDACTED *****/

		else if(TypeIsSignalLight(hw_ptr->type))
			{
			#ifdef USE_INT64
			char light_ptr[10];
			
			/* signal lights */
			clear_board = update_seq_numbers_inline(sign_buf, light_ptr, hw_ptr, -1, FALSE);

			/* data remains on the board so send new seq numbers */
			if(clear_board == 0)
				{
				int light_seq = light_ptr[0];

				if(db_bann_setcur(hw_ptr->board_ptr[light_seq].bann_recno) > 0)
					{
					struct _hardware *bridge_hw_ptr;
					bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	//get a pointer to the bridge device that our light bulb is associated with (we will need its IP and username to do anything with the light)
					if(bridge_hw_ptr)
						{
						send_to_banner_node(bridge_hw_ptr, "", hw_ptr->network_address, db_bann->dbb_light_signal);
						}
					else
						{
						DIAGNOSTIC_LOG("update_seq_numbers(): bridge_hw_ptr not found, cannot send command to signal light");
						}
					}
				}
			#endif
			}
		
/***** REDACTED *****/

return(error);
}

/***** REDACTED *****/

/******************************************************************
** static void execute_hardware_new(void)
**
******************************************************************/
static void execute_hardware_new(void)
{

/***** REDACTED *****/

	#ifdef USE_HUE_LIGHT
	/* for HUE lights look up the bridge - if one not found then nothing to send */
	/* otherwise send the light information to the bridge */
	if(hw_ptr
		&& hw_ptr->type == DEVICE_HUE_LIGHT)
		{
		struct _hardware *bridge_hw_ptr;
	
		bridge_hw_ptr = HardwareFindRecordNumber(hw_ptr->transmitter_record_number);	/* get a pointer to the bridge device that our light is associated with */
		
		if(bridge_hw_ptr)
			{
			hw_ptr->remote_pid = bridge_hw_ptr->remote_pid;
			hw_ptr->remote_child_pid = bridge_hw_ptr->remote_child_pid;
			hw_ptr->has_asked_sync = bridge_hw_ptr->has_asked_sync;
			hw_ptr->active_status = bridge_hw_ptr->active_status;
			}
		else
			{
			DIAGNOSTIC_LOG_1("execute_hardwar_new() ERROR, no bridge found for signal light %s", hw_ptr->hardware_deviceid);
			hw_ptr = NULL;
			}
		}
	#endif

	/***** REDACTED *****/
}

/***** REDACTED *****/

/********************************************************************
** int check_banner_node_commands(void)
** CR NOTE: This is the forked banner node for a given device. Device is selected by conditional tests and wtc data.
**
**  return:
**	0 is no error
**	1 is slow device was forked.
**      error is < 0
**
********************************************************************/
int check_banner_node_commands(void)
{

/***** REDACTED *****/

/* CR Added for Signal Light device support...
 * Interval note: Specifying this interval may be tricky. The Hue bridge enforces rate-limiting, which can be problematic when sending multiple
 * commands to multiple lights in rapid sequence. When the bridge rate-limits, it seems to drop commands (at best) or totally lock-up (at worst ~happened once requiring a complete power cycling).
 * 4/30/2013,CR: I tested up to 10 Hue bulbs on a simple network... seems to work fine with an interval of down to ~750ms, any shorter was problematic. Going with 800ms to be safe?
 * 5/2/2013,CR: With 10 bulbs, some weren't turning off completely... increasing to 900ms fixed that. With 20 bulbs, might need to increase further?
 * 6/4/2013,CR: With 3 bridges (5-7 lights each, total of 20 lights), even with newly implemented banner sleeps (mn_delay), flashing was sometimes erratic... increasing to 1 full second fixed that. Kevin wants less, but we reliably can NOT. */
#define MAX_SIGNAL_LIGHTS	(10)				/* max number of lights per bridge     DEV-NOTE: (50 is officially supported by Philips, but we may need to make it smaller ~ say 20? to make more manageable and effective) */
int signal_light_interval_flash_usec = 800*1000;/* bare minimum to have ~10 lights work as they should is about 800ms */
int signal_light_interval_fade_usec = 1600*1000;/* wait 1.6 seconds between sending fade-up and fade-down commands */
int lights_i;									/* counter for looping through lights */
static int signal_lights_active = -1;			/* flag whether any signal lights are supposed to be active (this determines whether this process sleeps or not) */
static int signal_lights_signal[MAX_SIGNAL_LIGHTS+1] = {-1};		/* declare an array that will be used to keep track of the bridge's lights' modes */
static int signal_lights_lastcmd_state[MAX_SIGNAL_LIGHTS+1];		/* declare an array that will be used to keep track of the bridge's lights' flashing states (on or off) */
unsigned long long signal_lights_lastcmd_time[MAX_SIGNAL_LIGHTS+1] = {-1};	/* declare an array that will be used to keep track of the last time a command was sent to a light */
static DBRECORD signal_lights_msg_recno[MAX_SIGNAL_LIGHTS+1];		/* declare an array that will be used to keep track of msg recno, so we can do a proper stop_message on any light */
static long long signal_lights_duration[MAX_SIGNAL_LIGHTS+1];		/* declare an array that will be used to keep track of the bridge's lights' durations */
unsigned long long signal_lights_launched_time[MAX_SIGNAL_LIGHTS+1];/* declare an array that will be used to keep track of the time this message was received by this logic */
long curtime_sec;	/* declare a local variable that we must use in order to get current time data, so we can then use it to build a timestamp, below */
long curtime_usec;	/* declare a local variable that we must use in order to get current time data, so we can then use it to build a timestamp, below */
unsigned long long curtime_time;	/* declare a local variable that we will use to store the assembled timestamp */

/***** REDACTED *****/

	/* CHRIS (DON'T DELETE COMMENTS BELOW!) */
	/* For each signal light, see what we need to do, and do it if necessary. */
	/* What this is about...
	 *   At least the first version of signal lights (Philips Hue) requires us to explicitly control flashing. They don't work like 
	 *   the AND PoE signs' flashers (where we just send the command to flash and it then takes care of it, on its own). So, the next
	 *   section of code basically takes care of making the lights look like they're flashing (alternating on... off... on... etc.).
	 * Pseudo-code...
	 *   When this code is encountered on runtime, we're either in our 1st iteration of this loop, or subsequent ones. Subsequent iterations must be explicitly allowed to happen by setting certain flags (next block). We use signal_lights_active in this case.
	 *   We control both the initial state (turning on) of our light flash, as well as subsequent states (off... on... off... etc.) here (to give the impression of flashing). That's what this logic basically determines and controls (among a few other things).
	 *   Steps:
	 *    - For each cycle of this banner-node (but with mn_delay), loop through each of the possible signal lights associated with this banner-node's bridge
	 *    - If any light is supposed to be flashing (indicated by the defined 'light signal' mode of the particular message using it), examine it further...
	 *    - If that light's last command timestamp has passed the time interval specified (signal_light_interval_flash/fade_usec), proceed to send the next command to it...
	 *    - Loop through hardware devices until we find the bridge that commands that light (this is just so we can get the hardware pointer for the bridge, which we need to send the command)
	 *    - Do one last check (light_duration) to make sure the light(s) are defined to still be flashing... if not, then set appropriate flags, turn off, etc.... and skip to the next light
	 *    - If we haven't skipped (above), now we should go ahead and send a command, so send either an 'on' or an 'off,' depending on what the last state of the light was (the goal is to alternate them so it 'flashes' */

	signal_lights_active = FALSE;					//ensures that we only keep looping (the main while loop) if we need to... (we explicitly re-flag this to keep going, below, within the for loop, before the next test of this flag)

	#ifdef USE_INT64
	for(lights_i=1; lights_i<=MAX_SIGNAL_LIGHTS; lights_i++)	//go through the list of bulbs (so we can check whether they need to be doing anything)... //DEV-NOTE: to improve speed, need to restrict to however many are actually defined, rather than max possible
		{
		if(signal_lights_signal[lights_i] < BANNER_SIGNALLIGHT_CMD_ON_END)
			{
			#ifdef DONT_THINK_NEED_THIS_FOR_CONSTANT
			/* check if currently on and need to stop */
			signal_lights_active = TRUE;					//reset our flag to true, so the forked banner client's main while-loop knows to keep this light flashing here (basically, disallow forked process from sleeping - except the special sleeping defined below, of course)

			GetBusyTime(&curtime_sec, &curtime_usec);			//save the current time to this function's local variables, so they can be used to construct a current timestamp that we can use to see if it's time to send an alternate command yet
			curtime_time = (curtime_sec * 1000000) + curtime_usec;		//combine sec and usec to form a long timestamp value that we can use to calculate anything we want, down to microsecond precision (just in case we'll ever need it for fine-tuning)

			/* If this light's is on steady, check to see if past duration ... */
			if(signal_lights_signal[lights_i] > BANNER_SIGNALLIGHT_CMD_NONE
				&& signal_lights_signal[lights_i] < BANNER_SIGNALLIGHT_CMD_ON_END)
				{
				/* Loop through our hardware (this is just for the intent of getting a pointer to this bridge) */
				//DEV-NOTE: is there some other way to do this, which might avoid having to do this loop?
				for(hw_ptr = hardware_head; hw_ptr; hw_ptr = hw_ptr->next)
					{
					//DEV-NOTE: if we must loop, is there some way to test for a pointer that matches this particular forked banner process (so we don't catch every bridge)?? or does it, in fact, only do this bridge?
					if(hw_ptr->type == DEVICE_HUE_BRIDGE) 				//if this hardware device is a signal light Hue bridge (the pointer that we will need to send the command), continue...
						{
						/* If the current time is now past however long this light should be doing something (defined by light-duration field for the message), and it has an expiration, then catch it here */
						if(curtime_time > (signal_lights_launched_time[lights_i] + (signal_lights_duration[lights_i] * 1000000))
							&& signal_lights_duration[lights_i] != 0)	/* no-expire messages are 0, so ignore them here (they never should stop by this mechanism) */
							{
							/* turn off the light ---- it should, afterward, go back to whatever it needs to be doing... Banner should take care of this */
							send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_OFF_FADEDEF);
							DIAGNOSTIC_LOG_2("Signal light duration has ended: Light #%d, message_recno="FORMAT_DBRECORD_STR, lights_i, signal_lights_msg_recno[lights_i]);

							/* let our for-loop's initial test know that this light should no longer keep our process awake, by setting it's mode to nothing */
							signal_lights_signal[lights_i] = BANNER_SIGNALLIGHT_DURATION_ENDED;

							/* reset appropriate flags just to be clean and safe */
							signal_lights_lastcmd_state[lights_i] = 0;
			
							/* break out of this and go on to to the next light's iteration */
							break;
							}

						GetBusyTime(&curtime_sec, &curtime_usec);
						signal_lights_lastcmd_time[lights_i] = (curtime_sec * 1000000) + curtime_usec;

						break;		//exit the hw_ptr for-loop since we only cared about finding the right bridge and we've since found it, used it, and are done with it for the moment
						}
					}
				}
			#endif
			}

		/* If this light is supposed to be fading... (NOTE: be sure the test conditions matche the enum defined in support_signallight.h) */
		else if(signal_lights_signal[lights_i] < BANNER_SIGNALLIGHT_CMD_FADING_END) /*refer to enum list in support_signallight.h */
			{
			signal_lights_active = TRUE;					//reset our flag to true, so the forked banner client's main while-loop knows to keep this light flashing here (basically, disallow forked process from sleeping - except the special sleeping defined below, of course)

			GetBusyTime(&curtime_sec, &curtime_usec);			//save the current time to this function's local variables, so they can be used to construct a current timestamp that we can use to see if it's time to send an alternate command yet
			curtime_time = (curtime_sec * 1000000) + curtime_usec;		//combine sec and usec to form a long timestamp value that we can use to calculate anything we want, down to microsecond precision (just in case we'll ever need it for fine-tuning)

			/* If this light's last command has passed our desired fade interval... */
			if(signal_lights_lastcmd_time[lights_i] != -1
				&& curtime_time - signal_lights_lastcmd_time[lights_i] >= signal_light_interval_fade_usec
				/*&& DEV-NOTE: also consider adding a test for Hue-specific lights if they're the only one we need to control flashing like this*/)
				{
				/* Loop through our hardware (this is just for the intent of getting a pointer to this bridge) */
				//DEV-NOTE: is there some other way to do this, which might avoid having to do this loop?
				for(hw_ptr = hardware_head; hw_ptr; hw_ptr = hw_ptr->next)
					{
					//DEV-NOTE: if we must loop, is there some way to test for a pointer that matches this particular forked banner process (so we don't catch every bridge)?? or does it, in fact, only do this bridge?
					if(hw_ptr->type == DEVICE_HUE_BRIDGE) 				//if this hardware device is a signal light Hue bridge (the pointer that we will need to send the command), continue...
						{

						/* If the current time is now past however long this light should be doing something (defined by light-duration field for the message), and it has an expiration, then catch it here */
						if(curtime_time > (signal_lights_launched_time[lights_i] + (signal_lights_duration[lights_i] * 1000000))
							&& signal_lights_duration[lights_i] != 0)	/* no-expire messages are 0, so ignore them here (they never should stop by this mechanism) */
							{
							/* turn off the light ---- it should, afterward, go back to whatever it needs to be doing... Banner should take care of this */
							send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_OFF_FADEDEF);
							DIAGNOSTIC_LOG_2("Signal light duration has ended: Light #%d, message_recno="FORMAT_DBRECORD_STR, lights_i, signal_lights_msg_recno[lights_i]);

							/* let our for-loop's initial test know that this light should no longer keep our process awake, by setting it's mode to nothing */
							signal_lights_signal[lights_i] = BANNER_SIGNALLIGHT_DURATION_ENDED;

							/* reset appropriate flags just to be clean and safe */
							signal_lights_lastcmd_state[lights_i] = 0;
			
							/* break out of this and go on to to the next light's iteration */
							break;
							}

						/* If this fading light was last represented as being faded-on, then send the command to turn fade-down and then reset our flag to represent that */
						if(signal_lights_lastcmd_state[lights_i] == 1)
							{
							send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM); 
							signal_lights_lastcmd_state[lights_i] = 0;	//flag that this light was commanded to fade down
							}
						/* Else this fading light should be commanded to fade to full brightness */
						else
							{
							send_to_signallight_device(hw_ptr, lights_i, determine_signallight_cmd_str(signal_lights_signal[lights_i]));
							signal_lights_lastcmd_state[lights_i] = 1;	//flag that this light was commanded to fade up
							}

						GetBusyTime(&curtime_sec, &curtime_usec);
						signal_lights_lastcmd_time[lights_i] = (curtime_sec * 1000000) + curtime_usec;

						break;		//exit the hw_ptr for-loop since we only cared about finding the right bridge and we've since found it, used it, and are done with it for the moment
						}
					}
				}
			}

		/* If this light is supposed to be flashing... (NOTE: be sure the test conditions matche the enum defined in support_signallight.h) */
		else if(signal_lights_signal[lights_i] < BANNER_SIGNALLIGHT_CMD_FLASHING_END) /*refer to enum list in support_signallight.h */
			{
			signal_lights_active = TRUE;					//reset our flag to true, so the forked banner client's main while-loop knows to keep this light flashing here (basically, disallow forked process from sleeping - except the special sleeping defined below, of course)

			GetBusyTime(&curtime_sec, &curtime_usec);			//save the current time to this function's local variables, so they can be used to construct a current timestamp that we can use to see if it's time to send an alternate command yet
			curtime_time = (curtime_sec * 1000000) + curtime_usec;		//combine sec and usec to form a long timestamp value that we can use to calculate anything we want, down to microsecond precision (just in case we'll ever need it for fine-tuning)

			/* If this light's last command has passed our desired flash interval... */
			if(signal_lights_lastcmd_time[lights_i] != -1
				&& curtime_time - signal_lights_lastcmd_time[lights_i] >= signal_light_interval_flash_usec
				/*&& DEV-NOTE: also consider adding a test for Hue-specific lights if they're the only one we need to control flashing like this*/)
				{

				/* Loop through our hardware (this is just for the intent of getting a pointer to this bridge) */
				//DEV-NOTE: is there some other way to do this, which might avoid having to do this loop?
				for(hw_ptr = hardware_head; hw_ptr; hw_ptr = hw_ptr->next)
					{
					//DEV-NOTE: if we must loop, is there some way to test for a pointer that matches this particular forked banner process (so we don't catch every bridge)?? or does it, in fact, only do this bridge?
					if(hw_ptr->type == DEVICE_HUE_BRIDGE) 				//if this hardware device is a signal light Hue bridge (the pointer that we will need to send the command), continue...
						{

						/* If the current time is now past however long this light should be doing something (defined by light-duration field for the message), and it has an expiration, then catch it here */
						if(curtime_time > (signal_lights_launched_time[lights_i] + (signal_lights_duration[lights_i] * 1000000))
							&& signal_lights_duration[lights_i] != 0)	/* no-expire messages are 0, so ignore them here (they never should stop by this mechanism) */
							{
							/* turn off the light ---- it should, afterward, go back to whatever it needs to be doing... Banner should take care of this */
							send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_OFF_FADEDEF);
							DIAGNOSTIC_LOG_2("Signal light duration has ended: Light #%d, message_recno="FORMAT_DBRECORD_STR, lights_i, signal_lights_msg_recno[lights_i]);

							/* let our for-loop's initial test know that this light should no longer keep our process awake, by setting it's mode to nothing */
							signal_lights_signal[lights_i] = BANNER_SIGNALLIGHT_DURATION_ENDED;

							/* reset appropriate flags just to be clean and safe */
							signal_lights_lastcmd_state[lights_i] = 0;
			
							/* break out of this and fall back to to the next light's iteration */
							break;
							}

						/* If this flashing light was last represented as being on, then send the command to turn it off and then reset our flag to represent that */
						if(signal_lights_lastcmd_state[lights_i] == 1)
							{
							//send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_OFF_FADEDEF); 
							//send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM); 	/* Per Kevin 6/25/13: Updated to go to instantly to dim rather than fade off */
							send_to_signallight_device(hw_ptr, lights_i, STR_SIGNALLIGHT_HUE_OFF_FADE200); 
							signal_lights_lastcmd_state[lights_i] = 0;	//flag that this light should now be off
							}
						/* Else this flashing light should be commanded to turn on */
						else
							{
							send_to_signallight_device(hw_ptr, lights_i, determine_signallight_cmd_str(signal_lights_signal[lights_i]));
							signal_lights_lastcmd_state[lights_i] = 1;	//flag that this light should now be on
							}

						GetBusyTime(&curtime_sec, &curtime_usec);
						signal_lights_lastcmd_time[lights_i] = (curtime_sec * 1000000) + curtime_usec;

						break;		//exit the hw_ptr for-loop since we only cared about finding the right bridge and we've since found it, used it, and are done with it for the moment
						}
					}
				}
			}
		}

	/* If lights flashing, then Wait a tiny bit on each main loop iteration, so as not to hammer the CPU core that runs this forked banner process */
	if(signal_lights_active > 0)
		{
		mn_delay(50);	//might be best that this number is less than (say, by half) or equal to our flash interval (signal_light_interval_flash_usec / 100000)... ex: if flash interval is 800*1000, that divided by 100000 is 8 -- not required, but gives optimal performance
		}
	#endif
	
	/***** REDACTED *****/

				/* CHRIS The following stuff happens (for each bulb?) whenever a message is sent to the forked banner process (each Hue bridge has its own process), but never when the forked process initializes */
				/* UPDATE: It appears that this test becomes true when a message is launched which vectors toward a Hue Bridge device (whenever that message is supposed to make its associated lights do things */
				else if(TypeIsSignalLight(HardwareDecodeDevice(db_wtc->dwc_return_node)))
					{
					#ifdef USE_INT64
					int light_bulb = db_wtc->dwc_operation;					//get light bulb's number from WTC (we repurposed dwc_operation for this)
					int light_bulb_signal = db_wtc->dwc_message_type;			//also get the mode that the light bulb is defined to do with this message
					int light_bulb_duration = db_wtc->dwc_baudrate;				//get the light_duration as defined in the message (we're passing down to forked process via dwc_baudrate - see send_to_banner_node)

					DBRECORD light_bulb_this_msg_recno = db_wtc->dwc_stream_number;		//get this message's record number (will use it to know which lights to affect if this message does something - like stop)

					if(light_bulb < 1)				//make sure we weren't given an invalid light number (they must begin with 1)
						{
						DIAGNOSTIC_LOG_1("check_banner_node_commands() ERROR signal light hardware cannot be defined < 1 (light_bulb = %d)", light_bulb);
						}

					/* DEV-NOTE: might be nice to make this test for the maximum light that is actually defined for this bridge, if possible (might eliminate any unnecessary iterations?) */
					else if(light_bulb > MAX_SIGNAL_LIGHTS)	//make sure we weren't given an invalid light number (due to Philips Hue rate limits, we desire a maximum amount of lights per bridge to avoid problems)
						{
						DIAGNOSTIC_LOG_2("check_banner_node_commands() ERROR signal light hardware cannot be defined > MAX_SIGNAL_LIGHTS (light_bulb = %d) (MAX_SIGNAL_LIGHTS = %d)", light_bulb, MAX_SIGNAL_LIGHTS);
						}

					else						//else this must logically be within the valid range of light bulb numbers for Hue that we desire (0 < lights_index <= MAX_SIGNAL_LIGHTS)
						{
						DIAGNOSTIC_LOG_3("Signal light new message/command received by banner node: Evaluating for light #%d... message_type=%d, message_recno="FORMAT_DBRECORD_STR, light_bulb, light_bulb_signal, light_bulb_this_msg_recno);

						// In case this light's corresponding element in the modes array isn't initialized the way we want it yet, initialize it to 0 (also do other arrays, since it's assumed they need it, too)
						if(signal_lights_signal[light_bulb] < 0)
							{
							memset(signal_lights_signal, 0, sizeof(signal_lights_signal));			//initialize array element
							memset(signal_lights_lastcmd_state, 0, sizeof(signal_lights_lastcmd_state));	//might as well also initialize this array's element, since it probably isn't yet, either (but not using here; only for flashing).
							memset(signal_lights_lastcmd_time, 0, sizeof(signal_lights_lastcmd_time));	//might as well also initialize this array's element, since it probably isn't yet, either (but not using here; only for flashing).
							memset(signal_lights_msg_recno, 0, sizeof(signal_lights_msg_recno));		//might as well also initialize this array's element, since it probably isn't yet, either.
							memset(signal_lights_duration, 0, sizeof(signal_lights_duration));		//might as well also initialize this array's element, since it probably isn't yet, either.
							memset(signal_lights_launched_time, 0, sizeof(signal_lights_launched_time));	//might as well also initialize this array's element, since it probably isn't yet, either.
							}

						// Handle sending the appropriate command(s) to this light bulb (probably preferred to put static one-time commands here, but flashing commands up above)
						//DEV-NOTE: for future makes/models of bulbs, beyond Philips Hue, you may need to alter the following logic to adapt to their (presumably) different way of getting commands?

						/* Stop message command received from banner and this light was remembered as displaying the signal for the message that needs to stop... */
						if(light_bulb_signal == BANNER_SIGNALLIGHT_CMD_STOP)
							{
							if(signal_lights_msg_recno[light_bulb] == light_bulb_this_msg_recno)
								{
								/* What happens when we need to stop a light-bulb message: (per Kevin, 5/6/2013)
								 * 1) Once it stops, light(s) turn OFF.
								 * 2) After that, the lights' custom default message takes back over (as with many other devices - e.g. a MediaPort can have a default message)
								 * 3) If a custom default message is not defined, then the light should fall back to our default-default - e.g. DIM BLUE (hue:47100, sat:255, bri:1) */
	
								/* send the command to turn the light off */
								send_to_signallight_device(hw_ptr, light_bulb, STR_SIGNALLIGHT_HUE_OFF_FADEDEF);
								DIAGNOSTIC_LOG_2("  BANNER_SIGNALLIGHT_CMD_STOP (stop_message) for Light #%d, msg_recno="FORMAT_DBRECORD_STR, light_bulb, light_bulb_this_msg_recno);
	
								/* save this light's message-defined signal mode to the array index for this light, so any subsequent iterations (e.g. flashing) know what the light should be doing */
								//DEV-NOTE: do we need to account for any other potentially active messages for this light here? is this handled by banner instead?
								signal_lights_signal[light_bulb] = light_bulb_signal;
	
								/* reset our arrays to initialized states (kind of makes sense, since this isn't really a message) */
								signal_lights_lastcmd_state[light_bulb] = 0;
								signal_lights_lastcmd_time[light_bulb] = 0;
								signal_lights_msg_recno[light_bulb] = 0;
								signal_lights_duration[light_bulb] = 0;
								signal_lights_launched_time[light_bulb] = 0;
								}

							/* since either this light's msg has stopped or the stop doesn't apply to this light, just skip to the next light */
							goto SKIP_TO_SIGNALLIGHT_END;

							}

						/* This message's defined light signal is 'none' */
						if(light_bulb_signal == BANNER_SIGNALLIGHT_CMD_NONE)
							{
							/* no need to send any command to the light for this message */
							DIAGNOSTIC_LOG_3("  BANNER_SIGNALLIGHT_CMD_NONE for Light #%d, msg_recno="FORMAT_DBRECORD_STR " duration=%d", light_bulb, light_bulb_this_msg_recno, light_bulb_duration);
							}

						/* This message's defined light signal is a constant-on (or off) type (not flashing)... NOTE: if changing anything, double-check this range by looking at the enum list in support_signallight.h */
						else if(light_bulb_signal > BANNER_SIGNALLIGHT_CMD_NONE 
							&& light_bulb_signal < BANNER_SIGNALLIGHT_CMD_ON_END)
							{
							send_to_signallight_device(hw_ptr, light_bulb, determine_signallight_cmd_str(light_bulb_signal));
							DIAGNOSTIC_LOG_4("  BANNER_SIGNALLIGHT_CMD_[constant-type] (%s) for Light #%d, msg_recno="FORMAT_DBRECORD_STR " duration=%d", determine_signallight_cmd_str(light_bulb_signal), light_bulb, light_bulb_this_msg_recno, light_bulb_duration);
							}

						/* This message's defined light signal is a fading type... NOTE: if changing anything, double-check this range by looking at the enum list in support_signallight.h */ 
						else if(light_bulb_signal < BANNER_SIGNALLIGHT_CMD_FADING_END)
							{
							/* no need to send any command to the light here, because it will get handled in the main loop above, immediately following this branch of execution */
							DIAGNOSTIC_LOG_4("  BANNER_SIGNALLIGHT_CMD_[fading-type] (%s) for Light #%d, msg_recno="FORMAT_DBRECORD_STR " duration=%d", determine_signallight_cmd_str(light_bulb_signal), light_bulb, light_bulb_this_msg_recno, light_bulb_duration);
							}

						/* This message's defined light signal is a flashing type... NOTE: if changing anything, double-check this range by looking at the enum list in support_signallight.h */ 
						else if(light_bulb_signal < BANNER_SIGNALLIGHT_CMD_FLASHING_END)
							{
							/* no need to send any command to the light here, because it will get handled in the main loop above, immediately following this branch of execution */
							DIAGNOSTIC_LOG_4("  BANNER_SIGNALLIGHT_CMD_[flashing-type] (%s) for Light #%d, msg_recno="FORMAT_DBRECORD_STR " duration=%d", determine_signallight_cmd_str(light_bulb_signal), light_bulb, light_bulb_this_msg_recno, light_bulb_duration);
							}

						else
							{
							/* NOTE: if this ever happens, make sure this branch's test conditions match what is being read from dwc_message_type -- also double-check the enum list in support_signallight.h for any changes there */
							DIAGNOSTIC_LOG_3("support.c: check_banner_node_commands() WARNING unexpected signal-mode (msg-type) received from Banner for signal-light #%d (dwc_message_type = %d) duration=%d", light_bulb, light_bulb_signal, light_bulb_duration);
							goto SKIP_TO_SIGNALLIGHT_END;
							}

						/* Unless told to skip-over this with a goto, update our local arrays so this node process knows what this light should be doing (some of these may be unnecessary, but save them anyway - just in case, shouldn't hurt */
						signal_lights_signal[light_bulb] = light_bulb_signal;			//for this light, remember this message's defined light-signal mode
						signal_lights_duration[light_bulb] = light_bulb_duration;		//for this light, remember this message's defined light-duration
						signal_lights_msg_recno[light_bulb] = light_bulb_this_msg_recno;	//for this light, remember this message's record number
						GetBusyTime(&curtime_sec, &curtime_usec);				//calculate and save the current time to this function's local variables, so they can be used to construct a current timestamp
						curtime_time = (curtime_sec * 1000000) + curtime_usec;			//combine those calculated sec/usec values to form a long timestamp value that we can use to calculate anything we want, down to microsecond precision
						signal_lights_launched_time[light_bulb] = curtime_time;			//for this light, remember the current time (i.e. the time that one of the above commands was sent to the light)

						}

					SKIP_TO_SIGNALLIGHT_END:;	/* gcc complains about this if there is no semicolon (probably because there's nothing after this) */

					#endif
					}//end else-if for TypeIsSignalLight

				break;
				}
			}

/***** REDACTED *****/

return(error);
}
