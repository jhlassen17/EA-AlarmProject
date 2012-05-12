// Change default storage class for local variables: on the stack
#class auto

// MACROS for the LED's
#define ON     0
#define OFF    1

#define LED0   0
#define LED1   1
#define LED2   2
#define LED3   3

#define LED_0_ID	4
#define LED_1_ID 	5
#define LED_2_ID  6
#define LED_3_ID 	7

#define ID_SWITCH_1 0
#define ID_SWITCH_2 1
#define ID_SWITCH_3 2
#define ID_SWITCH_4 3

#define ID_BUZZER 8


#define TCPCONFIG 					3
#define TCP_BUF_SIZE 				2048
#define HTTP_MAXSERVERS 			1
#define MAX_TCP_SOCKET_BUFFERS 		2
#define REDIRECTHOST 				"130.166.150.191"
#define REDIRECTTO  "http://" REDIRECTHOST "/index.zhtml"
#define USE_RABBITWEB 1

#define OS_TASK_CHANGE_PRIO_EN   1
#define OS_TIME_DLY_RESUME_EN 	 1
#define OS_SEM_EN 				 1

#define OS_MAX_EVENTS			 6

#define TASK_START_PRIORITY      10
#define TASK_HTTP_PRIORITY		 15
#define TASK_SWITCH_PRIORITY   	 12
#define TASK_BUZZER_PRIORITY     17
#define TASK_LOWEST_PRIORITY	 9

#define MSG_QUEUE_SIZE     		 20

#define SMTP_SERVER "mail.yalost.me"
//#define SMTP_SERVER "100.42.54.124"

#define SMTP_PORT		26

//#define SMTP_SERVER 			 "64.136.44.45"

#define USE_SMTP_AUTH

#define SMTP_VERBOSE

#define SMTP_DEBUG

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
void      *msgQueueData[20];

/*
 * Function Prototypes
 */
int 	isPrime				(int n);
void 	httpTask				(void *data);
void 	switchTask			(void *data);
void 	startTask   		(void *data);

void 	createOtherTasks	();
void  checkSwitch       ();

int 	ledToggle0			(HttpState* state);
int 	ledToggle1			(HttpState* state);
int 	ledToggle2			(HttpState* state);
int 	ledToggle3			(HttpState* state);

void 	initHttp				();
void 	initEmails			();
void 	initStmpMethod		();
void 	initCarrierDomains();

int   sendEmail			(int email);

int   ledToggleAll      ();
#web ledToggleAll

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

char led_LED0[15];
char led_LED1[15];
char led_LED2[15];
char led_LED3[15];

int pushed;



//#ximport "samples/BL2600/tcpip/pages/rabbit1.gif"     rabbit1_gif
//#ximport "samples/BL2600/tcpip/pages/ledon.gif"       ledon_gif
//#ximport "samples/BL2600/tcpip/pages/ledoff.gif"      ledoff_gif
//#ximport "samples/BL2600/tcpip/pages/button.gif"      button_gif
//#ximport "samples/BL2600/tcpip/pages/showsrc.shtml"   showsrc_shtml
#ximport "/alarm.zhtml" index_zhtml

SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".zhtml", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/alarm.zhtml", index_zhtml),
//	SSPEC_RESOURCE_XMEMFILE("/showsrc.shtml", showsrc_shtml),
//	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
//	SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
//	SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
//	SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif),
 //	SSPEC_RESOURCE_XMEMFILE("/ssi.c", ssi_c),
//	SSPEC_RESOURCE_ROOTVAR("led_LED0", led_LED0, PTR16, "%s"),
//	SSPEC_RESOURCE_ROOTVAR("led_LED1", led_LED1, PTR16, "%s"),
//	SSPEC_RESOURCE_ROOTVAR("led_LED2", led_LED2, PTR16, "%s"),
//	SSPEC_RESOURCE_ROOTVAR("led_LED3", led_LED3, PTR16, "%s"),
//	SSPEC_RESOURCE_FUNCTION("/led_LED0.cgi", ledToggle0),
//	SSPEC_RESOURCE_FUNCTION("/led_LED1.cgi", ledToggle1),
//	SSPEC_RESOURCE_FUNCTION("/led_LED2.cgi", ledToggle2),
//	SSPEC_RESOURCE_FUNCTION("/led_LED3.cgi", ledToggle3)
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
	int result;
   char fullEmail[50];
   	result = -1;
   strcpy(fullEmail,phoneNumber);

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
		result = 0;
	}
	else {
		printf("\n\rError sending the email message\n\r");
	}

   printf("Phonenumber: %s\n", phoneNumber);
   printf("Carrier: %s\n", carrierDomains[carrierChoice]);

	return result;
}

/*

int ledToggle0(HttpState* state) {
	INT8U err;
	if (strcmp(led_LED0, "ledon.gif") == 0) {
    	strcpy(led_LED0, "ledoff.gif");

		// take the fucking semaphore
 		OSSemPend(zone0Sem, 0, &err);
      		// digOut(LED_0_ID, 1);
			zone0State = 1;
			printf("led_LED0: OFF");
        OSSemPost(zone0Sem);
   	}
   	else {
      	strcpy(led_LED0, "ledon.gif");

		// take the fucking semaphore
 		OSSemPend(zone0Sem, 0, &err);
			zone0State = 0;
			printf("led_LED0: ON");
        OSSemPost(zone0Sem);
   	}

	cgi_redirectto(state, REDIRECTTO);
   	return 0;
}

int ledToggleAll()
{
	//
}

int ledToggle1(HttpState* state) {
	INT8U err;
	if (strcmp(led_LED1, "ledon.gif") == 0) {
      	strcpy(led_LED1, "ledoff.gif");

		// take the fucking semaphore
 		OSSemPend(zone1Sem, 0, &err);
			zone1State = 1;
			printf("led_LED1: OFF");
        OSSemPost(zone1Sem);
   	}
   	else {
		strcpy(led_LED1, "ledon.gif");

		// take the fucking semaphore
 		OSSemPend(zone1Sem, 0, &err);
			zone1State = 0;
			printf("led_LED1: ON");
        OSSemPost(zone1Sem);
   	}

	cgi_redirectto(state,REDIRECTTO);
   	return 0;
}

int ledToggle2(HttpState* state) {
	INT8U err;
   	if (strcmp(led_LED2, "ledon.gif") == 0) {
      	strcpy(led_LED2, "ledoff.gif");

		// take the fucking semaphore
 		OSSemPend(zone2Sem, 0, &err);
			zone2State = 1;
			printf("led_LED2: OFF");
        OSSemPost(zone2Sem);
   	}
   	else {
      	strcpy(led_LED2, "ledon.gif");

		// take the fucking semaphore
 		OSSemPend(zone2Sem, 0, &err);
			zone2State = 0;
			printf("led_LED2: ON");
        OSSemPost(zone2Sem);
   }

   cgi_redirectto(state, REDIRECTTO);
   return 0;
}

int ledToggle3(HttpState* state) {
	INT8U err;
   	if (strcmp(led_LED3, "ledon.gif") == 0) {
      	strcpy(led_LED3, "ledoff.gif");

		// take the fucking semaphore
 		OSSemPend(zone3Sem, 0, &err);
			zone3State = 1;
			printf("led_LED3: OFF");
        OSSemPost(zone3Sem);
   }
   else {
      	strcpy(led_LED3,"ledon.gif");

		// take the fucking semaphore
 		OSSemPend(zone3Sem, 0, &err);
			zone3State = ON;
			printf("led_LED3: ON");
        OSSemPost(zone3Sem);
   }

   cgi_redirectto(state,REDIRECTTO);
   return 0;
} */

/**
 *  Run the HTTP server in the background
 *
 *	@param: pointer to void
 */
void httpTask(void *data) {
	static int result;

	INT8U err;

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

      // turn off


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
         	pushed = 0;
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
         	pushed = 1;
				//sendEmail(1);
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
         	pushed = 2;
         	//sendEmail(2);
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
         	pushed = 3;
				//sendEmail(3);
         }

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 1 second
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else {

		}

      if(alarming == OFF)
      {
			digOut(ID_BUZZER, OFF);
         digOut(LED_0_ID, OFF);
         digOut(LED_1_ID, OFF);
         digOut(LED_2_ID, OFF);
         digOut(LED_3_ID, OFF);
      }
	}

}

void switchTask(void *data) {
	/*
	while (1) {
		printf("\nswitchTask1() done\n");
		if (kbhit()) {
			if (getchar() == 'r') {
				printf("resume");
				OSTimeDlyResume(5);
			}
		}
	}
	*/
	auto int *result;
	auto INT8U err;
	while (1) {
		// wait forever for message
		result = (int *)OSQPend(msgQueuePtr, 0, &err);
		// receive message from queue
		printf("\nMessage received is %d\n", *result);
		// OSTaskChangePrio(TASK_BUZZER_PRIORITY, TASK_LOWEST_PRIORITY);
		switch (*result) {
			case ID_SWITCH_1:
				OSSemPend(zone0Sem, 0, &err);
				if (zone0State == ON) {
					printf("\nzone 000 \n");
				}
				OSSemPost(zone0Sem);
				break;

			case ID_SWITCH_2:
				OSSemPend(zone1Sem, 0, &err);
				if (zone1State == ON) {
					printf("\nzone 111 \n");
				}
				OSSemPost(zone1Sem);
				break;

			case ID_SWITCH_3:
				OSSemPend(zone2Sem, 0, &err);
				if (zone2State == ON) {
					printf("\nzone 222 \n");
				}
				OSSemPost(zone2Sem);
				break;

			case ID_SWITCH_4:
				OSSemPend(zone3Sem, 0, &err);
				if (zone3State == ON) {
					printf("\nzone 333 \n");
				}
				OSSemPost(zone3Sem);
				break;

			default:
				break;
		}
	}
}

void buzzerTask(void* data) {
	INT8U err;
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

        //Send email
        if(pushed > -1)
        {
	 		  sendEmail(pushed);
        }

        //Reset for next button push
	     pushed = -1;


		// now leave the buzzer
		OSTaskChangePrio(TASK_LOWEST_PRIORITY, TASK_BUZZER_PRIORITY);
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
	OSTaskCreate(switchTask, NULL, 512, TASK_SWITCH_PRIORITY);
	OSTaskCreate(buzzerTask, NULL, 512, TASK_BUZZER_PRIORITY);
}

void initHttp() {
	char buffer[16];
	brdInit();

  	// configure IO channels DIO00 - DIO03 as digital outputs (sinking type outputs)


	// initialize socket
   	sock_init();

	// wait for IP address to show up
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}

	printf("My IP address is %s\n", inet_ntoa(buffer, gethostid()));


   	http_init();
   	tcp_reserveport(80);

   	// set the initial state of the LED's
   	/*strcpy(led_LED0, "ledon.gif");
   	strcpy(led_LED1, "ledon.gif");
   	strcpy(led_LED2, "ledon.gif");
   	strcpy(led_LED3, "ledon.gif");*/
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

   brdInit();
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
		zone0Sem = OSSemCreate(1);
		zone1Sem = OSSemCreate(1);
		zone2Sem = OSSemCreate(1);
		zone3Sem = OSSemCreate(1);
		OSTaskCreate(startTask, NULL, 512, TASK_START_PRIORITY);
    OSStart();
}





