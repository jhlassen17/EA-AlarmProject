// Change default storage class for local variables: on the stack
#class auto

// Define State
#define ON     0
#define OFF    1

// PIN IDs for the LEDs
#define LED_0_ID	4
#define LED_1_ID 	5
#define LED_2_ID  6
#define LED_3_ID 	7

// PIN IDs for the Switches
#define ID_SWITCH_1 0
#define ID_SWITCH_2 1
#define ID_SWITCH_3 2
#define ID_SWITCH_4 3

// PIN ID for the Buzzer
#define ID_BUZZER 8


// TCP / HTTP Config
#define TCPCONFIG 					3		// DHCP
#define UDP_BUF_SIZE 				2048
#define TCP_BUF_SIZE 				2048
#define HTTP_MAXSERVERS 			1
#define MAX_UDP_SOCKET_BUFFERS 		4
#define MAX_TCP_SOCKET_BUFFERS 		4		//10
#define MAX_SOCKETS 				6
#define SOCK_BUF_SIZE 				1024
#define MAXBUFS 					10
#define USE_RABBITWEB 1

// RTOS Settings
#define OS_MAX_EVENTS 				10 		// Maximum number of events (semaphores, queues, mailboxes)
#define OS_MAX_TASKS 				10 		// Maximum number of tasks  (less stat and idle tasks)
#define OS_MAX_QS 					2 		// Max number of queues in system
#define OS_MAX_MEM_PART 			1 		// Max number of memory partitions
#define OS_TASK_CREATE_EN 			1 		// Enable normal task creation
#define OS_TASK_CREATE_EXT_EN 		0 		// Disable extended task creation
#define OS_TASK_DEL_EN 				0 		// Disable task deletion
#define OS_TASK_STAT_EN 			0 		// Disable statistics task creation
#define OS_Q_EN 					1 		// Enable queue usage
#define OS_MEM_EN 					0 		// Disable memory manager
#define OS_MBOX_EN 					1 		// Enable mailboxes
#define OS_SEM_EN 					1 		// Enable semaphores
#define OS_TICKS_PER_SEC 			64 		// number of ticks in one second
#define STACK_CNT_256 				1 		// number of 256 byte stacks (idle task stack)
#define STACK_CNT_512 				OS_MAX_TASKS + 1 		// number of 512 byte stacks (task stacks + initial program stack)
#define OS_TASK_CHANGE_PRIO_EN   	1
#define OS_TIME_DLY_RESUME_EN 	 	1
#define MSG_QUEUE_SIZE     		 	25

// RTOS Priorities, Higher # means lower priority
#define TASK_HTTP_PRIORITY		 	19
#define TASK_SWITCH_PRIORITY   	 	17
#define TASK_ALARM_PRIORITY			15

// SMTP Settings
#define SMTP_SERVER "mail.yalost.me"
#define SMTP_PORT		26
#define USE_SMTP_AUTH
#define SMTP_AUTH_FAIL_IF_NO_AUTH
#define FROM     	"alarm@yalost.me"
#define TO       	"heavyjeff17@gmail.com"
#define SUBJECT  	"You've got alarms!"

#define BODY0     	"You better watch it, zone 1.\r\n" \
				 			"You are being robbed son."

#define BODY1     	"You better watch it, zone 2.\r\n" \
				 			"You are being robbed son."

#define BODY2     	"You better watch it, zone 3.\r\n" \
				 			"You are being robbed son."

#define BODY3     	"You better watch it, zone 4.\r\n" \
				 			"You are being robbed son."

#define CARRIER0	   "@txt.att.net"
#define CARRIER1	   "@messaging.sprintpcs.com"
#define CARRIER2	   "@vtext.com"
#define CARRIER3	   "@tmomail.net"
#define CARRIER4	   "@gmail.com"

// Authentication Settings
#define USE_HTTP_DIGEST_AUTHENTICATION		1
#define SSPEC_USERSPERRESOURCE 				6
#web_groups 								admin


// Imports
#memmap xmem
#use "ucos2.lib"
#use "dcrtcp.lib"
#use "http.lib"
#use "smtp.lib"


// OS Events - Messages
OS_EVENT  		*msgQueuePtr;

// OS Events - Task Control
OS_EVENT		*httpToSwitch;
OS_EVENT		*switchToHTTP;

// OS Events - Shared data
OS_EVENT  *zoneSem;
OS_EVENT  *alarmSem;

// Message Queue
void      *msgQueueData[MSG_QUEUE_SIZE];

/*
 * Function Prototypes
 */

 void initCarrierDomains();			// Initializes carrier domains
 void setupHttp();					// Initializes HTTP and network
 void setupSMTP();					// Sets up SMTP related items
 void setupDigOut();				// Sets up Dig out items
 int sendEmail(int email);			// Sends Email


 void alarmTask(void *data);		// Alarm Task
 void httpTask(void *data);			// HTTP Task
 void switchTask(void *data);		// Swith Task

 void main(void);					// Main Method

 //

 /*
  * Global Variables
  */
// Zones
int zone0State;						//Zone 1 Arm/Disarm State
int zone1State;						//Zone 2 Arm/Disarm State
int zone2State;						//Zone 3 Arm/Disarm State
int zone3State;						//Zone 4 Arm/Disarm State
int zone0Alarm;						//Zone 1 Alarm State
int zone1Alarm;						//Zone 2 Alarm State
int zone2Alarm;						//Zone 3 Alarm State
int zone3Alarm;						//Zone 4 Alarm State
int alarming;						//Are we alarming?
int pushed;							//Was a button pressed
// Alerts
char carrierDomains[5][32];			//Carrier Domain Names
int carrierChoice;					//Current Carrier Selection
char phoneNumber[16];				//Current Phone Number Selection
char emailBody[4][256];				//List of email messages

 /*
  * WEB Related Variables
  */
#web zone0State
#web zone1State
#web zone2State
#web zone3State
#web zone0Alarm
#web zone1Alarm
#web zone2Alarm
#web zone3Alarm
#web alarming
#web carrierDomains
#web carrierChoice
#web phoneNumber;

/*
 * WEB Items
 */
#ximport "/alarm.zhtml" index_zhtml			//Import the Alarm Page
#ximport "/zones.zhtml" zones_zhtml			//Import AJAX Page

/*
 * WEB MIME Setup
 */
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".zhtml", "text/html"),
SSPEC_MIMETABLE_END

/*
 * WEB Resource Setup
 */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/alarm.zhtml", index_zhtml),
  	SSPEC_RESOURCE_XMEMFILE("/zones.zhtml", zones_zhtml)
SSPEC_RESOURCETABLE_END




/**
 * Initializes Carrier Domain Array
 */
nodebug void initCarrierDomains() {
	// Copy the domain names
	strcpy(carrierDomains[0], CARRIER0);
	strcpy(carrierDomains[1], CARRIER1);
	strcpy(carrierDomains[2], CARRIER2);
	strcpy(carrierDomains[3], CARRIER3);
	strcpy(carrierDomains[4], CARRIER4);

	//Set initial phone selection
	strcpy(phoneNumber, "heavyjeff17");
	carrierChoice = 4;
	//strcpy(phoneNumber,"9176489840");
	//carrierChoice=2;
}

/**
 * Does network and HTTP setup
 */
void setupHttp() {
	// IP Buffer
	char buffer[16];
	// User ID
	int userid;

	// Initialize the board
   	brdInit();

	// Initialize the socket
   	sock_init();

	// Wait for IP address to be obtained
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}

	// Output the IP Address
	printf("My IP address is %s\n", inet_ntoa(buffer, gethostid()));

	//Set up the HTTP Server
   	http_init();
   	tcp_reserveport(80);

	// Set redirect
	http_set_path("/", "/index.zhtml");

	// Set up authentication
	sspec_addrule("/", "Admin", admin, admin, SERVER_ANY, SERVER_AUTH_BASIC, NULL);

   // Add our users
   // Ario
   userid = sauth_adduser("ario", "fish", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);
   // Chan
   userid = sauth_adduser("chan", "bar", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);
   // Jeff
   userid = sauth_adduser("jeff", "bar7", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);
   // Shea
   userid = sauth_adduser("shea", "bar2", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);
   // Toby
   userid = sauth_adduser("toby", "bar3", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);

   //Done
}

/**
 * Initializes the SMTP Settings
 */
void setupSMTP() {
	// Set Auth
	smtp_setauth("alarm+yalost.me", "Al@rm");

	// Copy the bodies
	strcpy(emailBody[0], BODY0);
	strcpy(emailBody[1], BODY1);
	strcpy(emailBody[2], BODY2);
	strcpy(emailBody[3], BODY3);
}

/**
 * Configures digital outputs
 */
void setupDigOut() {
	// Set dig out mask
	digOutConfig(0x01F0);

	// Turn off everything
	digOut(LED_0_ID, OFF);
	digOut(LED_1_ID, OFF);
	digOut(LED_2_ID, OFF);
	digOut(LED_3_ID, OFF);
	digOut(ID_BUZZER, OFF);
}


// Tasks

void switchTask(void *data) {
	//
	INT8U err;

	//
	auto int localZone0;
	auto int localZone1;
	auto int localZone2;
	auto int localZone3;
	//
	auto int sw0State;
	auto int sw1State;
	auto int sw2State;
	auto int sw3State;
	//
	static char result;

	//
	localZone0 = ON;
	localZone1 = ON;
	localZone2 = ON;
	localZone3 = ON;
	//
	sw0State = OFF;
	sw1State = OFF;
	sw2State = OFF;
	sw3State = OFF;

	//
	while(1) {
		// Debug
		//printf("Switch Task Run\n");

		// Check switches
		sw0State = digIn(ID_SWITCH_1);
		sw1State = digIn(ID_SWITCH_2);
		sw2State = digIn(ID_SWITCH_3);
		sw3State = digIn(ID_SWITCH_4);

		// Check to see if we have any switches
		if(!sw0State || !sw1State || !sw2State || !sw3State ) {

			OSSemPend(zoneSem, 0, &err);
				localZone0 = zone0State;
				localZone1 = zone1State;
				localZone2 = zone2State;
				localZone3 = zone3State;
			OSSemPost(zoneSem);

			if(!sw0State && localZone0 == ON) {
				// Msg Queue Zone 0?
				result = '0';
			}
			else if(!sw1State && localZone1 == ON) {
				// Msg Queue Zone 1?
				result = '1';
			}
			else if(!sw2State && localZone2 == ON) {
				// Msg Queue Zone 2?
				result = '2';
			}
			else if(!sw3State && localZone3 == ON) {
				// Msg Queue Zone 3?
				result = '3';
			}
			else {
				//Nope
				result = 'A';
			}

			// Post the message
			printf("Posting alarm message \n");
			OSQPost(msgQueuePtr, (void *)&result);

		}

		// Update Alarm State ?
		OSSemPend(alarmSem, 0, &err);

			if(alarming == OFF) {
				digOut(ID_BUZZER, OFF);
				digOut(LED_0_ID, OFF);
				digOut(LED_1_ID, OFF);
				digOut(LED_2_ID, OFF);
				digOut(LED_3_ID, OFF);
			}
		OSSemPost(alarmSem);

		//Done
		OSSemPost(switchToHTTP);
		// Try to take a semaphore
 		OSSemPend(httpToSwitch, 0, &err);
	}
}

void httpTask(void *data) {
	//
	INT8U err;

	//
	while(1) {
		//HTTP
		// Debug
		// printf("HTTP Task Run\n");

		// Try to take a semaphore
 		OSSemPend(switchToHTTP, 0, &err);

		// Interact with the web
		http_handler();


		//Done
		OSSemPost(httpToSwitch);
	}
}

void alarmTask(void *data) {
	//
	INT8U err;
	//
	char *result;
	auto char realResult;

	while(1) {
		//Read MSG Queue, set off alarm, etc
		// wait forever for message
		result = (char *)OSQPend(msgQueuePtr, 0, &err);
		realResult = *result;

		// Debug
		printf("Alarm Task Run\n");

		if(realResult == '0') {
			printf("\nZone 1 Tripped!!!!\n");
			digOut(ID_BUZZER, ON);
			digOut(LED_0_ID, ON);

			OSSemPend(alarmSem, 0, &err);
				zone0Alarm = ON;
				alarming = ON;
			OSSemPost(alarmSem);

			//
			sendEmail(0);
		} else if(realResult == '1') {
			printf("\nZone 12Tripped!!!!\n");
			digOut(ID_BUZZER, ON);
			digOut(LED_1_ID, ON);

			OSSemPend(alarmSem, 0, &err);
				zone1Alarm = ON;
				alarming = ON;
			OSSemPost(alarmSem);

			//
			sendEmail(1);
		} else if(realResult == '2') {
			printf("\nZone 3 Tripped!!!!\n");
			digOut(ID_BUZZER, ON);
			digOut(LED_2_ID, ON);

			OSSemPend(alarmSem, 0, &err);
				zone2Alarm = ON;
				alarming = ON;
			OSSemPost(alarmSem);

			//
			sendEmail(2);
		} else if(realResult == '3') {
			printf("\nZone 4 Tripped!!!!\n");
			digOut(ID_BUZZER, ON);
			digOut(LED_3_ID, ON);

			OSSemPend(alarmSem, 0, &err);
				zone3Alarm = ON;
				alarming = ON;
			OSSemPost(alarmSem);

			//
			sendEmail(3);
		}
	}
}




int sendEmail(int email) {
	// Full email address
   char fullEmail[50];


   if (email < 0) {
   	printf("Now you done fucked up son.");
   	return -1;
   }

   //Copy the phone numner
   strcpy(fullEmail, phoneNumber);

   // Send the email
	smtp_sendmail(
		strcat(fullEmail, carrierDomains[carrierChoice]), FROM,
		SUBJECT, emailBody[email]);



	// Wait until the message has been sent
	while (smtp_mailtick() == SMTP_PENDING) {
		continue;
	}

	// Check to see if the message was sent successfully
	if (smtp_status() == SMTP_SUCCESS) {
		printf("\n\rMessage sent\n\r");
	}
	else {
		printf("\n\rError sending the email message\n\r");
	}

	return 0;
}


/**
 * Main Method
 */
void main (void) {
	// Set state
	zone0State = ON;
	zone1State = ON;
	zone2State = ON;
	zone3State = ON;
	zone0Alarm = OFF;
	zone1Alarm = OFF;
	zone2Alarm = OFF;
	zone3Alarm = OFF;
	alarming = OFF;

	// Do setup
	setupHttp();
	setupDigOut();
	setupSMTP();
	initCarrierDomains();

	// Setup multitasking
    OSInit();

	// Setup semaphores
	httpToSwitch = OSSemCreate(1);
	switchToHTTP  = OSSemCreate(1);
	zoneSem  = OSSemCreate(1);
	alarmSem  = OSSemCreate(1);

	// Setup Message Queue
	msgQueuePtr = OSQCreate(&msgQueueData[0], MSG_QUEUE_SIZE);

	//Setup tasks
	OSTaskCreate(httpTask, NULL, 512, TASK_HTTP_PRIORITY);
	OSTaskCreate(switchTask, NULL, 512, TASK_SWITCH_PRIORITY);
	OSTaskCreate(alarmTask, NULL, 512, TASK_ALARM_PRIORITY);

	// Start Multitasking
    OSStart();

	// Toby
    printf("Toby is great!");
}