/*********************************************************************
**	Module:		support_signallight.h
**
**	Author:		Chris Rider
**			Copyright (c) 2013
**	Revisions:
**
** 28 Mar, 2013 - csr creation
**
*********************************************************************/

/* This list should be in the same order as (and match up with) light_signal_values as defined in db_banne.c!
 *  (these corresponding values get sent across as the message_type argument, and are converted from ASCII/hex to decimal) */
/* WARNING: Be careful when/if updating this list!
 * 	Logic (test conditions) contained in support.c:check_banner_node_commands() depends on a certain order here, because we check for > and < type of stuff. 
 * 	If you want to change the order, you may need to adjust that logic, if not careful. See comments below. */
typedef enum
	{
	BANNER_SIGNALLIGHT_CMD_NONE = 32,		/* 32 value due to hex-to-dec conversion in BannerEncodeSelectList and BannerDecodeSelectList (the first select-list item in the msg editor field for this has an ASCII value of ' ') */
	BANNER_SIGNALLIGHT_CMD_OFF,			/* signal to turn a light logically off */
	BANNER_SIGNALLIGHT_CMD_ON_STANDBY = 35,		/* the "default" or "standby" state - (initially conceived to be a very dim blue/ultraviolet-esque color) */

	/* The following are static or "steady-on" commands that get sent to the light JUST ONCE and don't need special attention (like flashing) beyond that */
	BANNER_SIGNALLIGHT_CMD_ON_BLUE_DIM = 40,
	BANNER_SIGNALLIGHT_CMD_ON_BLUE_MED,
	BANNER_SIGNALLIGHT_CMD_ON_BLUE_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_GREEN_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_GREEN_MED,
	BANNER_SIGNALLIGHT_CMD_ON_GREEN_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_ORANGE_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_ORANGE_MED,
	BANNER_SIGNALLIGHT_CMD_ON_ORANGE_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_PINK_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_PINK_MED,
	BANNER_SIGNALLIGHT_CMD_ON_PINK_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_PURPLE_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_PURPLE_MED,
	BANNER_SIGNALLIGHT_CMD_ON_PURPLE_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_RED_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_RED_MED,
	BANNER_SIGNALLIGHT_CMD_ON_RED_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_MED,
	BANNER_SIGNALLIGHT_CMD_ON_WHITECOOL_BRI = 63,	/* skipped using some potentially problematic characters (see db_banne.c) */
	BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_MED,
	BANNER_SIGNALLIGHT_CMD_ON_WHITEPURE_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_MED,
	BANNER_SIGNALLIGHT_CMD_ON_WHITEWARM_BRI,
	BANNER_SIGNALLIGHT_CMD_ON_YELLOW_DIM,
	BANNER_SIGNALLIGHT_CMD_ON_YELLOW_MED,
	BANNER_SIGNALLIGHT_CMD_ON_YELLOW_BRI,

	BANNER_SIGNALLIGHT_CMD_ON_END,
	
	/* The following are fading commands that require attention for each on/off cycle (banner node while-loop) to make the light fade in and out */
	BANNER_SIGNALLIGHT_CMD_FADING_BLUE = 85,
	BANNER_SIGNALLIGHT_CMD_FADING_GREEN,
	BANNER_SIGNALLIGHT_CMD_FADING_ORANGE,
	BANNER_SIGNALLIGHT_CMD_FADING_PINK,
	BANNER_SIGNALLIGHT_CMD_FADING_PURPLE,
	BANNER_SIGNALLIGHT_CMD_FADING_RED,
	BANNER_SIGNALLIGHT_CMD_FADING_WHITECOOL,
	BANNER_SIGNALLIGHT_CMD_FADING_WHITEPURE,
	BANNER_SIGNALLIGHT_CMD_FADING_WHITEWARM,
	BANNER_SIGNALLIGHT_CMD_FADING_YELLOW,

	BANNER_SIGNALLIGHT_CMD_FADING_END,

	/* The following are flashing commands that require attention for each on/off cycle (banner node while-loop) to give the impression of flashing */
	BANNER_SIGNALLIGHT_CMD_FLASHING_BLUE = 100,
	BANNER_SIGNALLIGHT_CMD_FLASHING_GREEN,
	BANNER_SIGNALLIGHT_CMD_FLASHING_ORANGE,
	BANNER_SIGNALLIGHT_CMD_FLASHING_PINK,
	BANNER_SIGNALLIGHT_CMD_FLASHING_PURPLE,
	BANNER_SIGNALLIGHT_CMD_FLASHING_RED,
	BANNER_SIGNALLIGHT_CMD_FLASHING_WHITECOOL,
	BANNER_SIGNALLIGHT_CMD_FLASHING_WHITEPURE,
	BANNER_SIGNALLIGHT_CMD_FLASHING_WHITEWARM,
	BANNER_SIGNALLIGHT_CMD_FLASHING_YELLOW,

	BANNER_SIGNALLIGHT_CMD_FLASHING_END

	} BANNER_SIGNALLIGHT_CMD;

/* Defined constants that we don't want to include in the above list, due to it messing up the correlation with the message's defined light_signal */
#define BANNER_SIGNALLIGHT_CMD_STOP (0)
#define BANNER_SIGNALLIGHT_DURATION_ENDED (1)

/* Philips Hue notes ********************************************************************************************************************************************
** Colors
** 	-HUE colors use primarily a combination of hue (0-65535) and saturation (0-255).
** 	-Imagine a color wheel overlaid on a clock... 12:00 is 0-degrees... 6:00 is 180-degrees... etc.
** 	-Red is at the top (0 degrees) and you should distribute "ROYGBIVR" (red, orange, yellow, green, blue, indigo, violet, red) around the circle.
** 	-"hue" = DEGREES * 182.04  (rounded to the nearest integer)
**	-"sat" is simply how saturated the hue(color) is... 0 is white and 255 is the richest color
**	-Examples:
**		Rich Red:	"hue":0 (or "hue":65535) and "sat":255
**		Washed-out Red:	"hue":0 (or "hue":65535) and "sat":150
**
** Blue...
** Hue cannot do a very pure saturated blue.
** 	 -The more rich (toward hue:47000) you go, the more violet it becomes.
** 	 -The more blue (toward hue:40000) you go, the less saturated it becomes.
**
** Green...
** Hue cannot do a very pure saturated green. The closest you can get is a kind of "lime" color.
**
** White...
** If you use "ct" rather than "hue" & "sat" you may see better results. (Remember, the accepted range is 154-500)
** The "ct" value is measured in a unit called "mireds" (it's equal to 1,000,000 divided by degrees Kelvin).
** 	For example, candle light (2,000 K) would be about "ct":500 (1,000,000 divided by 2,000)
** 	For example, incandescent bulb (2,800 K) would be about "ct":357 (1,000,000 divided by 2,800)
** 	For example, halogen bulb (3,000 K) would be about "ct":333 (1,000,000 divided by 3,000)
** 	(for more examples, just search the internet for "white color temperature" or similar)
** 	Discovered best results:
** 		- "Daylight" type of white (blue sunny sky in the shade):	"ct":154
** 		- Pure white w/highest CRI (best white for color accuracy):	"ct":200
** 		- Warm white, incandescent (best white for reading/comfort):	"ct":357
**
** Commands
**  -The bridge receives commands as JSON strings.
**  -They must be syntaxed exactly as below (double quotes must be used for JSON strings - which is why they're escaped) */
#define SIGNALLIGHT_HUE_JSON_OFF_FADE200		"{\"transitiontime\":2,\"on\":false}"						//fades completely off in around 200ms
#define SIGNALLIGHT_HUE_JSON_OFF_FADEDEF		"{\"on\":false}"								//fades completely off in around 400ms (which is the Philips Hue default)
#define SIGNALLIGHT_HUE_JSON_OFF_FADE800		"{\"transitiontime\":8,\"on\":false}"						//fades completely off in around 800ms
#define SIGNALLIGHT_HUE_JSON_OFF_INSTANT		"{\"transitiontime\":0,\"on\":false}"						//instantly turns completely off
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM		"{\"on\":true,\"transitiontime\":0,\"bri\":1}"					//instantly goes to minimum brightness
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM		"{\"on\":true,\"bri\":1}"							//fades to minimum brightness in around 400ms
#define SIGNALLIGHT_HUE_JSON_ON_FADE800_DIM		"{\"on\":true,\"transitiontime\":8,\"bri\":1}"					//fades to minimum brightness in around 800ms
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM		"{\"on\":true,\"transitiontime\":16,\"bri\":1}"					//fades to minimum brightness in around 1600ms

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_UV		"{\"on\":true,\"bri\":1,\"hue\":47100,\"sat\":255}"				//fades to minimum brightness deep irridescent blue (almost like a blacklight) in around 400ms
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_UV		"{\"on\":true,\"transitiontime\":0,\"bri\":1,\"hue\":47100,\"sat\":255}"	//instantly turns on to deep irridescent blue (almost like a blacklight)

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_BLUE	"{\"on\":true,\"hue\":43000,\"sat\":255,\"bri\":30}"				//fade 400ms on dim blue
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_BLUE	"{\"on\":true,\"transitiontime\":16,\"hue\":43000,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim blue
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_BLUE	"{\"on\":true,\"transitiontime\":0,\"hue\":43000,\"sat\":255,\"bri\":30}"	//instant on dim blue
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_BLUE	"{\"on\":true,\"hue\":43000,\"sat\":255,\"bri\":150}"				//fade 400ms on medium blue
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_BLUE	"{\"on\":true,\"transitiontime\":16,\"hue\":43000,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium blue
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_BLUE	"{\"on\":true,\"transitiontime\":0,\"hue\":43000,\"sat\":255,\"bri\":150}"	//instant on medium blue
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_BLUE	"{\"on\":true,\"hue\":43000,\"sat\":255,\"bri\":255}"				//fade 400ms on bright blue 
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_BLUE	"{\"on\":true,\"transitiontime\":16,\"hue\":43000,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright blue 
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_BLUE	"{\"on\":true,\"transitiontime\":0,\"hue\":43000,\"sat\":255,\"bri\":255}"	//instant on bright blue 

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_GREEN	"{\"on\":true,\"hue\":25000,\"sat\":255,\"bri\":30}"				//fade 400ms on dim green
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_GREEN	"{\"on\":true,\"transitiontime\":16,\"hue\":25000,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim green
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_GREEN	"{\"on\":true,\"transitiontime\":0,\"hue\":25000,\"sat\":255,\"bri\":30}"	//instant on dim green
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_GREEN	"{\"on\":true,\"hue\":25000,\"sat\":255,\"bri\":150}"				//fade 400ms on medium green
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_GREEN	"{\"on\":true,\"transitiontime\":16,\"hue\":25000,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium green
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_GREEN	"{\"on\":true,\"transitiontime\":0,\"hue\":25000,\"sat\":255,\"bri\":150}"	//instant on medium green
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_GREEN	"{\"on\":true,\"hue\":25000,\"sat\":255,\"bri\":255}"				//fade 400ms on bright green
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_GREEN	"{\"on\":true,\"transitiontime\":16,\"hue\":25000,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright green
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_GREEN	"{\"on\":true,\"transitiontime\":0,\"hue\":25000,\"sat\":255,\"bri\":255}"	//instant on bright green

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_ORANGE	"{\"on\":true,\"hue\":9000,\"sat\":255,\"bri\":30}"				//fade 400ms on dim orange
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_ORANGE	"{\"on\":true,\"transitiontime\":16,\"hue\":9000,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim orange
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_ORANGE	"{\"on\":true,\"transitiontime\":0,\"hue\":9000,\"sat\":255,\"bri\":30}"	//instant on dim orange
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_ORANGE	"{\"on\":true,\"hue\":9000,\"sat\":255,\"bri\":150}"				//fade 400ms on medium orange
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_ORANGE	"{\"on\":true,\"transitiontime\":16,\"hue\":9000,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium orange
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_ORANGE	"{\"on\":true,\"transitiontime\":0,\"hue\":9000,\"sat\":255,\"bri\":150}"	//instant on medium orange
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_ORANGE	"{\"on\":true,\"hue\":9000,\"sat\":255,\"bri\":255}"				//fade 400ms on bright orange
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_ORANGE	"{\"on\":true,\"transitiontime\":16,\"hue\":9000,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright orange
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_ORANGE	"{\"on\":true,\"transitiontime\":0,\"hue\":9000,\"sat\":255,\"bri\":255}"	//instant on bright orange

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_PINK	"{\"on\":true,\"hue\":61500,\"sat\":255,\"bri\":30}"				//fade 400ms on dim pink
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_PINK	"{\"on\":true,\"transitiontime\":16,\"hue\":61500,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim pink
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_PINK	"{\"on\":true,\"transitiontime\":0,\"hue\":61500,\"sat\":255,\"bri\":30}"	//instant on dim pink
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_PINK	"{\"on\":true,\"hue\":61500,\"sat\":255,\"bri\":150}"				//fade 400ms on medium pink
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_PINK	"{\"on\":true,\"transitiontime\":16,\"hue\":61500,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium pink
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_PINK	"{\"on\":true,\"transitiontime\":0,\"hue\":61500,\"sat\":255,\"bri\":150}"	//instant on medium pink
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_PINK	"{\"on\":true,\"hue\":61500,\"sat\":255,\"bri\":255}"				//fade 400ms on bright pink
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_PINK	"{\"on\":true,\"transitiontime\":16,\"hue\":61500,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright pink
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_PINK	"{\"on\":true,\"transitiontime\":0,\"hue\":61500,\"sat\":255,\"bri\":255}"	//instant on bright pink

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_PURPLE	"{\"on\":true,\"hue\":49879,\"sat\":255,\"bri\":30}"				//fade 400ms on dim purple
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_PURPLE	"{\"on\":true,\"transitiontime\":16,\"hue\":49879,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim purple
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_PURPLE	"{\"on\":true,\"transitiontime\":0,\"hue\":49879,\"sat\":255,\"bri\":30}"	//instant on dim purple
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_PURPLE	"{\"on\":true,\"hue\":49879,\"sat\":255,\"bri\":150}"				//fade 400ms on medium purple
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_PURPLE	"{\"on\":true,\"transitiontime\":16,\"hue\":49879,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium purple
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_PURPLE	"{\"on\":true,\"transitiontime\":0,\"hue\":49879,\"sat\":255,\"bri\":150}"	//instant on medium purple
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_PURPLE	"{\"on\":true,\"hue\":49879,\"sat\":255,\"bri\":255}"				//fade 400ms on bright purple
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_PURPLE	"{\"on\":true,\"transitiontime\":16,\"hue\":49879,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright purple
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_PURPLE	"{\"on\":true,\"transitiontime\":0,\"hue\":49879,\"sat\":255,\"bri\":255}"	//instant on bright purple

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_RED		"{\"on\":true,\"hue\":0,\"sat\":255,\"bri\":30}"				//fade 400ms on dim red
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_RED	"{\"on\":true,\"transitiontime\":16,\"hue\":0,\"sat\":255,\"bri\":30}"		//fade 1600ms on dim red
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_RED		"{\"on\":true,\"transitiontime\":0,\"hue\":0,\"sat\":255,\"bri\":30}"		//instant on dim red
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_RED		"{\"on\":true,\"hue\":0,\"sat\":255,\"bri\":150}"				//fade 400ms on medium red
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_RED	"{\"on\":true,\"transitiontime\":16,\"hue\":0,\"sat\":255,\"bri\":150}"		//fade 1600ms on medium red
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_RED		"{\"on\":true,\"transitiontime\":0,\"hue\":0,\"sat\":255,\"bri\":150}"		//instant on medium red
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_RED		"{\"on\":true,\"hue\":0,\"sat\":255,\"bri\":255}"				//fade 400ms on bright red
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_RED	"{\"on\":true,\"transitiontime\":16,\"hue\":0,\"sat\":255,\"bri\":255}"		//fade 1600ms on bright red
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_RED		"{\"on\":true,\"transitiontime\":0,\"hue\":0,\"sat\":255,\"bri\":255}"		//instant on bright red

/* DEV-NOTE -- The "ct" method doesn't seem to work on the newer rope-strip light ("friends of Hue")
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITECOOL	"{\"on\":true,\"ct\":154,\"bri\":30}"						//fade 400ms on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"ct\":154,\"bri\":30}"			//fade 1600ms on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"ct\":154,\"bri\":30}"			//instant on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITECOOL	"{\"on\":true,\"ct\":154,\"bri\":150}"						//fade 400ms on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"ct\":154,\"bri\":150}"			//fade 1600ms on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"ct\":154,\"bri\":150}"			//instant on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITECOOL	"{\"on\":true,\"ct\":154,\"bri\":255}"						//fade 400ms on bright cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"ct\":154,\"bri\":255}"			//fade 1600ms on bright cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"ct\":154,\"bri\":255}"			//instant on bright cool white

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEPURE	"{\"on\":true,\"ct\":200,\"bri\":30}"						//fade 400ms on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"ct\":200,\"bri\":30}"			//fade 1600ms on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"ct\":200,\"bri\":30}"			//instant on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEPURE	"{\"on\":true,\"ct\":200,\"bri\":150}"						//fade 400ms on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"ct\":200,\"bri\":150}"			//fade 1600ms on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"ct\":200,\"bri\":150}"			//instant on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEPURE	"{\"on\":true,\"ct\":200,\"bri\":255}"						//fade 400ms on bright pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"ct\":200,\"bri\":255}"			//fade 1600ms on bright pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"ct\":200,\"bri\":255}"			//instant on bright pure white

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEWARM	"{\"on\":true,\"ct\":357,\"bri\":30}"						//fade 400ms on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"ct\":357,\"bri\":30}"			//fade 1600ms on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"ct\":357,\"bri\":30}"			//instant on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEWARM	"{\"on\":true,\"ct\":357,\"bri\":150}"						//fade 400ms on medium warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"ct\":357,\"bri\":150}"			//fade 1600ms on medium warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"ct\":357,\"bri\":150}"			//instant on medium warm white
//#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEWARM	"{\"on\":true,\"ct\":357,\"bri\":255}"						//fade 400ms on bright warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"ct\":357,\"bri\":255}"			//fade 1600ms on bright warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"ct\":357,\"bri\":255}"			//instant on bright warm white
*/
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITECOOL	"{\"on\":true,\"hue\":34515,\"sat\":236,\"bri\":30}"				//fade 400ms on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"hue\":34515,\"sat\":236,\"bri\":30}"	//fade 1600ms on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"hue\":34515,\"sat\":236,\"bri\":30}"	//instant on dim cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITECOOL	"{\"on\":true,\"hue\":34515,\"sat\":236,\"bri\":150}"				//fade 400ms on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"hue\":34515,\"sat\":236,\"bri\":150}"	//fade 1600ms on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"hue\":34515,\"sat\":236,\"bri\":150}"	//instant on medium cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITECOOL	"{\"on\":true,\"hue\":34515,\"sat\":236,\"bri\":255}"				//fade 400ms on bright cool white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITECOOL	"{\"on\":true,\"transitiontime\":16,\"hue\":34515,\"sat\":236,\"bri\":255}"	//fade 1600ms on bright cool white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITECOOL	"{\"on\":true,\"transitiontime\":0,\"hue\":34515,\"sat\":236,\"bri\":255}"	//instant on bright cool white

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEPURE	"{\"on\":true,\"hue\":34106,\"sat\":133,\"bri\":30}"				//fade 400ms on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"hue\":34106,\"sat\":133,\"bri\":30}"	//fade 1600ms on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"hue\":34106,\"sat\":133,\"bri\":30}"	//instant on dim pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEPURE	"{\"on\":true,\"hue\":34106,\"sat\":133,\"bri\":150}"				//fade 400ms on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"hue\":34106,\"sat\":133,\"bri\":150}"	//fade 1600ms on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"hue\":34106,\"sat\":133,\"bri\":150}"	//instant on medium pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEPURE	"{\"on\":true,\"hue\":34106,\"sat\":133,\"bri\":255}"				//fade 400ms on bright pure white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEPURE	"{\"on\":true,\"transitiontime\":16,\"hue\":34106,\"sat\":133,\"bri\":255}"	//fade 1600ms on bright pure white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEPURE	"{\"on\":true,\"transitiontime\":0,\"hue\":34106,\"sat\":133,\"bri\":255}"	//instant on bright pure white

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_WHITEWARM	"{\"on\":true,\"hue\":15163,\"sat\":131,\"bri\":30}"				//fade 400ms on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"hue\":15163,\"sat\":131,\"bri\":30}"	//fade 1600ms on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"hue\":15163,\"sat\":131,\"bri\":30}"	//instant on dim warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_WHITEWARM	"{\"on\":true,\"hue\":15163,\"sat\":131,\"bri\":150}"				//fade 400ms on medium warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"hue\":15163,\"sat\":131,\"bri\":150}"	//fade 1600ms on medium warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"hue\":15163,\"sat\":131,\"bri\":150}"	//instant on medium warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_WHITEWARM	"{\"on\":true,\"hue\":15163,\"sat\":131,\"bri\":30}"				//fade 400ms on bright warm white
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_WHITEWARM	"{\"on\":true,\"transitiontime\":16,\"hue\":15163,\"sat\":131,\"bri\":255}"	//fade 1600ms on bright warm white
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_WHITEWARM	"{\"on\":true,\"transitiontime\":0,\"hue\":15163,\"sat\":131,\"bri\":255}"	//instant on bright warm white

#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_DIM_YELLOW	"{\"on\":true,\"hue\":15000,\"sat\":255,\"bri\":30}"				//fade 400ms on dim yellow
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_DIM_YELLOW	"{\"on\":true,\"transitiontime\":16,\"hue\":15000,\"sat\":255,\"bri\":30}"	//fade 1600ms on dim yellow
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_DIM_YELLOW	"{\"on\":true,\"transitiontime\":0,\"hue\":15000,\"sat\":255,\"bri\":30}"	//instant on dim yellow
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_MED_YELLOW	"{\"on\":true,\"hue\":15000,\"sat\":255,\"bri\":150}"				//fade 400ms on medium yellow
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_MED_YELLOW	"{\"on\":true,\"transitiontime\":16,\"hue\":15000,\"sat\":255,\"bri\":150}"	//fade 1600ms on medium yellow
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_MED_YELLOW	"{\"on\":true,\"transitiontime\":0,\"hue\":15000,\"sat\":255,\"bri\":150}"	//instant on medium yellow
#define SIGNALLIGHT_HUE_JSON_ON_FADEDEF_BRI_YELLOW	"{\"on\":true,\"hue\":15000,\"sat\":255,\"bri\":255}"				//fade 400ms on bright yellow
#define SIGNALLIGHT_HUE_JSON_ON_FADE1600_BRI_YELLOW	"{\"on\":true,\"transitiontime\":16,\"hue\":15000,\"sat\":255,\"bri\":255}"	//fade 1600ms on bright yellow
#define SIGNALLIGHT_HUE_JSON_ON_INSTANT_BRI_YELLOW	"{\"on\":true,\"transitiontime\":0,\"hue\":15000,\"sat\":255,\"bri\":255}"	//instant on bright yellow

/* Prototype these global constants that can be used for passing the above JSON strings anywhere (they actually get assigned in support_signallight.c) */
extern const char STR_SIGNALLIGHT_HUE_OFF_FADEDEF[];
extern const char STR_SIGNALLIGHT_HUE_OFF_FADE200[];
extern const char STR_SIGNALLIGHT_HUE_OFF_FADE800[];
extern const char STR_SIGNALLIGHT_HUE_OFF_INSTANT[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE800_DIM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_UV[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_UV[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_BLUE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_BLUE[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_GREEN[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_GREEN[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_ORANGE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_ORANGE[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PINK[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PINK[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_PURPLE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_PURPLE[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_RED[];	
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_RED[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_RED[];	
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_RED[];	
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_RED[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_RED[];	
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_RED[];	
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_RED[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_RED[];	

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITECOOL[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITECOOL[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEPURE[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEPURE[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_WHITEWARM[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_WHITEWARM[];

extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_DIM_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_DIM_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_DIM_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_MED_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_MED_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_MED_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADEDEF_BRI_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_FADE1600_BRI_YELLOW[];
extern const char STR_SIGNALLIGHT_HUE_ON_INSTANT_BRI_YELLOW[];

/* Prototype these function that can be used anywhere */
extern const char *determine_signallight_cmd_str(int message_type);
int send_to_signallight_device(struct _hardware *hw_ptr, int light_number, const char *bann_light_command);
