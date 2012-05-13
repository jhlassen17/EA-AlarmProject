// Change default storage class for local variables: on the stack
#class auto

// Define State
#define ON     0
#define OFF    1

//
#define LED0   0
#define LED1   1
#define LED2   2
#define LED3   3

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
#define TCPCONFIG 					3
#define TCP_BUF_SIZE 				4096
#define HTTP_MAXSERVERS 			1
#define MAX_TCP_SOCKET_BUFFERS 		10
#define USE_RABBITWEB 1

// RTOS Settings
#define OS_TASK_CHANGE_PRIO_EN   1
#define OS_TIME_DLY_RESUME_EN 	 1
#define OS_SEM_EN 				 1
#define OS_MAX_EVENTS			 8
#define MSG_QUEUE_SIZE     		 25

// RTOS Priorities, Higher # means lower priority
#define TASK_START_PRIORITY      10
#define TASK_HTTP_PRIORITY		 15
#define TASK_SWITCH_PRIORITY   	 12
#define TASK_BUZZER_PRIORITY     17
#define TASK_HIGHEST_PRIORITY	 9


#define SMTP_SERVER "mail.yalost.me"
#define SMTP_PORT		26
#define USE_SMTP_AUTH
//#define SMTP_VERBOSE
//#define SMTP_DEBUG
#define SMTP_AUTH_FAIL_IF_NO_AUTH


// ucos library must be explicity used
#memmap xmem
#use "ucos2.lib"
#use "dcrtcp.lib"
#use "http.lib"
#use "smtp.lib"


/*
 * All events
 */
OS_EVENT  *msgQueuePtr;
OS_EVENT  *buzzerSemaphore;
OS_EVENT  *zone0Sem;
OS_EVENT  *zone1Sem;
OS_EVENT  *zone2Sem;
OS_EVENT  *zone3Sem;
OS_EVENT  *pushedSem;

/*
 * The structure that holds an email message
 */
typedef struct {
	char *from;
	char *to;
	char *subject;
	char *body;
} Email;

/*
 * Holds the email messages
 */
Email emailArray[4];

#define FROM     	"alarm@yalost.me"
#define TO       	"9176489840@vtext.com"
#define SUBJECT  	"You've got mail!"
#define BODY     	"Visit the Rabbit Semiconductor web site.\r\n" \
				 		"There you'll find the latest news about Dynamic C."

/*
 * Message queue data
 */
void      *msgQueueData[25];

/*
 * Function Prototypes
 */
int 	isPrime				(int n);
void 	httpTask				(void *data);
void 	switchTask			(void *data);
void 	startTask   		(void *data);

void 	createOtherTasks	();
void  checkSwitch       ();

void 	initHttp				();
void 	initEmails			();
void 	initStmpMethod		();
void 	initCarrierDomains();

int   sendEmail			(int email);

/*
 * Zone state
 * 		1 -> disarm
 * 		0 -> arm
 */
int zone0State;
#web zone0State
int zone1State;
#web zone1State
int zone2State;
#web zone2State
int zone3State;
#web zone3State

/*
 * Zone alarm state
 * 		1 -> ok
 * 		0 -> alarming
 */
int zone0Alarm;
#web zone0Alarm
int zone1Alarm;
#web zone1Alarm
int zone2Alarm;
#web zone2Alarm
int zone3Alarm;
#web zone3Alarm
int alarming;
#web alarming

//phonenumber and carrier info vars
char carrierDomains[4][32];
#web carrierDomains
int carrierChoice;
#web carrierChoice
char phoneNumber[16];
#web phoneNumber;

//
int pushed;

// Web Pages
#ximport "/alarm.zhtml" index_zhtml
#ximport "/zones.zhtml" zones_zhtml

// Web MIME
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".zhtml", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/alarm.zhtml", index_zhtml),
  	SSPEC_RESOURCE_XMEMFILE("/zones.zhtml", zones_zhtml)
SSPEC_RESOURCETABLE_END

nodebug void clearScreen() {
   printf("\x1Bt");            	// Space Opens Window
}

nodebug void dispStr(int x, int y, char *s) {
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void printDelayMsg() {
	dispStr(5, 5, "|");
   dispStr(5, 5, "/");
   dispStr(5, 5, "-");
   dispStr(5, 5, "\\");
}

void initEmails() {
	emailArray[0].from = "alarm@yalost.me";
	emailArray[0].to = TO;
	emailArray[0].subject = "Alarm Alert!";
	emailArray[0].body = "Your house just exploded";

	emailArray[1].from = "alarm@yalost.me";
	emailArray[1].to = TO;
	emailArray[1].subject = "Alarm Alert!";
	emailArray[1].body = "Your car is being broken into";

	emailArray[2].from = "alarm@yalost.me";
	emailArray[2].to = TO;
	emailArray[2].subject = "Alarm Alert!";
	emailArray[2].body = "Missle heading toward you!!!";

	emailArray[3].from = "alarm@yalost.me";
	emailArray[3].to = TO;
	emailArray[3].subject = "Alarm Alert!";
	emailArray[3].body = "Time to sleep";
}

// "###@messaging.sprintpcs.com"
// "###@txt.att.net"
// "###@vtext.com"
// "###@tmomail.net"
void initCarrierDomains() {
   strcpy(carrierDomains[0],"@txt.att.net");
   strcpy(carrierDomains[1],"@messaging.sprintpcs.com");
   strcpy(carrierDomains[2],"@vtext.com");
   strcpy(carrierDomains[3],"@tmomail.net");

   strcpy(phoneNumber,"9176489840");
   carrierChoice=2;
}

int sendEmail(int email) {
	// Full email address
   char fullEmail[50];


   if(email < 0 )
   {
   	printf("Now you done fucked up son.");
   	 return -1;
   }

   //Copy the phone numner
   strcpy(fullEmail,phoneNumber);

   // Send the email
	smtp_sendmail(
		//emailArray[email].to,
      strcat(fullEmail,carrierDomains[carrierChoice]), //check that ph# + choice have values?
      emailArray[email].from,
   	    emailArray[email].subject,
		emailArray[email].body);


	// Wait until the message has been sent
	while (smtp_mailtick() == SMTP_PENDING) {
    	clearScreen();
		printf("pending ! ! ! ! !\n");
		continue;
	}

	// Check to see if the message was sent successfully
	if (smtp_status() == SMTP_SUCCESS) {
		printf("\n\rMessage sent\n\r");
	}
	else {
		printf("\n\rError sending the email message\n\r");
	}

   printf("Phonenumber: %s\n", phoneNumber);
   printf("Carrier: %s\n", carrierDomains[carrierChoice]);

	return 0;
}

/**
 *  Run the HTTP server in the background
 *
 *	@param: pointer to void
 */
void httpTask(void *data) {
	static int result;

	INT8U err;
   INT8U aErr;

	int localZone0;
	int localZone1;
	int localZone2;
	int localZone3;

	localZone0 = ON;
	localZone1 = ON;
	localZone2 = ON;
	localZone3 = ON;

	while (1) {
		printDelayMsg();


		// interact with the web
		http_handler();

		if (!digIn(ID_SWITCH_1)) {
			result = 16;

			// take the fucking semaphore
 			OSSemPend(zone0Sem, 0, &err);
				localZone0 = zone0State;
        	OSSemPost(zone0Sem);

			// send email
			if (localZone0 == ON)
         {
         	// take the fucking semaphore
 				OSSemPend(pushedSem, 0, &aErr);
					pushed = 0;
        		OSSemPost(pushedSem);
         }

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 2 seconds
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else if (!digIn(ID_SWITCH_2)) {
			result = 17;

			// take the fucking semaphore
 			OSSemPend(zone1Sem, 0, &err);
				localZone1 = zone1State;
        	OSSemPost(zone1Sem);

			// send email
			if (localZone1 == ON)
         {
           // take the fucking semaphore
 				OSSemPend(pushedSem, 0, &aErr);
					pushed = 1;
        		OSSemPost(pushedSem);
         }

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 2 seconds
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else if (!digIn(ID_SWITCH_3)) {
			result = 18;

			// take the fucking semaphore
 			OSSemPend(zone2Sem, 0, &err);
				localZone2 = zone2State;
        	OSSemPost(zone2Sem);

			// send email
			if (localZone2 == ON)
			{
 				OSSemPend(pushedSem, 0, &aErr);
					pushed = 2;
        		OSSemPost(pushedSem);
         }

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 2 seconds
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else if (!digIn(ID_SWITCH_4)) {
			result = 19;

			// take the fucking semaphore
 			OSSemPend(zone3Sem, 0, &err);
				localZone3 = zone3State;
        	OSSemPost(zone3Sem);

			// send email
			if (localZone3 == ON)
         {
              // take the fucking semaphore
 				OSSemPend(pushedSem, 0, &aErr);
					pushed = 3;
        		OSSemPost(pushedSem);
         }

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 1 second
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else {

		}

      OSSemPend(pushedSem, 0, &aErr);

	      if(alarming == OFF)
   	   {
				digOut(ID_BUZZER, OFF);
	         digOut(LED_0_ID, OFF);
   	      digOut(LED_1_ID, OFF);
      	   digOut(LED_2_ID, OFF);
         	digOut(LED_3_ID, OFF);
	      }
      OSSemPost(pushedSem);
	}

}


void buzzerTask(void* data) {
	INT8U err;
   INT8U aErr;

   int localPushed;

   localPushed = -1;

	while (1) {
		/*
		 * Zone 1 check
		 */
 		OSSemPend(zone0Sem, 0, &err);
		if (zone0State == ON && pushed == 0) {
			printf("\nzone 00000000000000000\n");
         digOut(ID_BUZZER, ON);
         digOut(LED_0_ID, ON);
         zone0Alarm = ON;
         alarming = ON;
		}
        OSSemPost(zone0Sem);

		/*
		 * Zone 2 check
		 */
 		OSSemPend(zone1Sem, 0, &err);
		if (zone1State == ON && pushed == 1) {
			printf("\nzone 1111111111111111\n");
         digOut(ID_BUZZER, ON);
         digOut(LED_1_ID, ON);
         zone1Alarm = ON;
         alarming = ON;
		}
        OSSemPost(zone1Sem);

		/*
		 * Zone 3 check
		 */
 		OSSemPend(zone2Sem, 0, &err);
		if (zone2State == ON && pushed == 2) {
			printf("\nzone 22222222222222222\n");
         digOut(ID_BUZZER, ON);
         digOut(LED_2_ID, ON);
         zone2Alarm = ON;
         alarming = ON;
		}
        OSSemPost(zone2Sem);

		/*
		 * Zone 4 check
		 */
 		OSSemPend(zone3Sem, 0, &err);
		if (zone3State == ON && pushed == 3) {
			printf("\nzone 33333333333333333\n");
         digOut(ID_BUZZER, ON);
         digOut(LED_3_ID, ON);
         zone3Alarm = ON;
         alarming = ON;
		}
        OSSemPost(zone3Sem);

          // take the fucking semaphore
 		OSSemPend(pushedSem, 0, &aErr);
      	localPushed = pushed;
      OSSemPost(pushedSem);

      if(localPushed > -1)
      {
	 		  sendEmail(localPushed);
      }

      OSSemPend(pushedSem, 0, &aErr);
      	pushed = -1;
      OSSemPost(pushedSem);

		// now leave the buzzer
		OSTaskChangePrio(TASK_HIGHEST_PRIORITY, TASK_BUZZER_PRIORITY);
		OSTimeDly(OS_TICKS_PER_SEC * 1);
	}
}

void startTask(void *data) {
	// create a message queue
  	msgQueuePtr = OSQCreate(&msgQueueData[0], MSG_QUEUE_SIZE);
	// create other tasks
	createOtherTasks();
	// wait for 5 seconds
	OSTimeDly(OS_TICKS_PER_SEC * 5);
}

void createOtherTasks() {
	OSTaskCreate(httpTask, NULL, 512, TASK_HTTP_PRIORITY);
	// OSTaskCreate(switchTask, NULL, 512, TASK_SWITCH_PRIORITY);
	OSTaskCreate(buzzerTask, NULL, 512, TASK_BUZZER_PRIORITY);
}

void initHttp() {
	char buffer[16];

   	brdInit();

	// initialize socket
   	sock_init();

	// wait for IP address to show up
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}

	printf("My IP address is %s\n", inet_ntoa(buffer, gethostid()));


   	http_init();
   	tcp_reserveport(80);
}

void initStmpMethod() {
	initEmails();

}

void turnOffAllLeds() {
	digOutConfig(0x01F0);
	digOut(LED_0_ID, OFF);
   digOut(LED_1_ID, OFF);
   digOut(LED_2_ID, OFF);
   digOut(LED_3_ID, OFF);
}

void main (void) {
	/*
   	All zones are armed initially
      	0 = armed
         1 = disarmed
	*/
	zone0State = ON;
	zone1State = ON;
	zone2State = ON;
	zone3State = ON;
   zone0Alarm = OFF;
	zone1Alarm = OFF;
	zone2Alarm = OFF;
	zone3Alarm = OFF;
   alarming = OFF;
   pushed = -1;

   initHttp();
   turnOffAllLeds();

	// verification for email server
	#ifdef USE_SMTP_AUTH
		smtp_setauth ("alarm+yalost.me", "Al@rm");
	#endif

	initStmpMethod();
   initCarrierDomains();

	// start multitasking
    OSInit();
		buzzerSemaphore = OSSemCreate(1);
		zone0Sem  = OSSemCreate(1);
		zone1Sem  = OSSemCreate(1);
		zone2Sem  = OSSemCreate(1);
		zone3Sem  = OSSemCreate(1);
      pushedSem = OSSemCreate(1);
		OSTaskCreate(startTask, NULL, 512, TASK_START_PRIORITY);
    OSStart();
}


