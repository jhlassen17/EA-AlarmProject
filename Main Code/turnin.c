// Change default storage class for local variables: on the stack
#class auto

// MACROS for the LED's
#define ON     1
#define OFF    0

#define LED0   0
#define LED1   1
#define LED2   2
#define LED3   3

#define LED_0_ID	4
#define LED_1_ID 	5
#define LED_2_ID  	6
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
#define REDIRECTTO  "http://" REDIRECTHOST "/index.shtml"

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
#define TO       	"ngocchan.nguyen.61@my.csun.edu"
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
void 	httpTask			(void *data);
void 	switchTask			(void *data);
void 	startTask   		(void *data);

void 	createOtherTasks	();
void  	checkSwitch         ();

int 	ledToggle0			(HttpState* state);
int 	ledToggle1			(HttpState* state);
int 	ledToggle2			(HttpState* state);
int 	ledToggle3			(HttpState* state);

void 	initHttp			();
void 	initEmails			();
void 	initStmpMethod		();

int     sendEmail			(int email);

/*
 * Zone state
 * 		1 -> disarm
 * 		0 -> arm
 */
int zone0State;
int zone1State;
int zone2State;
int zone3State;

char led_LED0[15];
char led_LED1[15];
char led_LED2[15];
char led_LED3[15];


#ximport "samples/BL2600/tcpip/pages/ssi.shtml"       index_html
#ximport "samples/BL2600/tcpip/pages/rabbit1.gif"     rabbit1_gif
#ximport "samples/BL2600/tcpip/pages/ledon.gif"       ledon_gif
#ximport "samples/BL2600/tcpip/pages/ledoff.gif"      ledoff_gif
#ximport "samples/BL2600/tcpip/pages/button.gif"      button_gif
#ximport "samples/BL2600/tcpip/pages/showsrc.shtml"   showsrc_shtml
#ximport "samples/BL2600/tcpip/ssi.c"                 ssi_c

SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", "text/html", shtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/index.shtml", index_html),
	SSPEC_RESOURCE_XMEMFILE("/showsrc.shtml", showsrc_shtml),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
	SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif),
	SSPEC_RESOURCE_XMEMFILE("/ssi.c", ssi_c),
	SSPEC_RESOURCE_ROOTVAR("led_LED0", led_LED0, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED1", led_LED1, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED2", led_LED2, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED3", led_LED3, PTR16, "%s"),
	SSPEC_RESOURCE_FUNCTION("/led_LED0.cgi", ledToggle0),
	SSPEC_RESOURCE_FUNCTION("/led_LED1.cgi", ledToggle1),
	SSPEC_RESOURCE_FUNCTION("/led_LED2.cgi", ledToggle2),
	SSPEC_RESOURCE_FUNCTION("/led_LED3.cgi", ledToggle3)
SSPEC_RESOURCETABLE_END

void initEmails() {
	emailArray[0].from = "viethoang1@juno.com";
	emailArray[0].to = TO;
	emailArray[0].subject = "Self-Test status OK!!!";
	emailArray[0].body = "Your house just exploded";

	emailArray[1].from = "viethoang1@juno.com";
	emailArray[1].to = TO;
	emailArray[1].subject = "Self-Test failed.";
	emailArray[1].body = "Your car is being broken into";

	emailArray[2].from = "viethoang1@juno.com";
	emailArray[2].to = TO;
	emailArray[2].subject = "System shut down";
	emailArray[2].body = "Missle heading toward you!!!";

	emailArray[3].from = "viethoang1@juno.com";
	emailArray[3].to = TO;
	emailArray[3].subject = "System malfunction";
	emailArray[3].body = "Time to sleep";
}

int sendEmail(int email) {
	int result;
   	result = -1;

	smtp_sendmail(
		emailArray[email].to,
		emailArray[email].from,
   	    emailArray[email].subject,
		emailArray[email].body);

	// Wait until the message has been sent
	while (smtp_mailtick() == SMTP_PENDING) {
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

	return result;
}



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
			zone3State = 0;
			printf("led_LED3: ON");
        OSSemPost(zone3Sem);
   }

   cgi_redirectto(state,REDIRECTTO);
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

	int localZone0;
	int localZone1;
	int localZone2;
	int localZone3;

	localZone0 = 1;
	localZone1 = 1;
	localZone2 = 1;
	localZone3 = 1;

	while (1) {
		printf("..................\n");

		// interact with the web
		http_handler();

		if (!digIn(ID_SWITCH_1)) {
			result = 16;

			// take the fucking semaphore
 			OSSemPend(zone0Sem, 0, &err);
				localZone0 = zone0State;
        	OSSemPost(zone0Sem);

			// send email
			if (localZone0 == 0)
				sendEmail(0);

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
			if (localZone1 == 0)
				sendEmail(1);

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
			if (localZone2 == 0)
				sendEmail(2);

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
			if (localZone3 == 0)
				sendEmail(3);

			// post the result to queue
			OSQPost(msgQueuePtr, (void *)&result);

			// delay for 1 second
			OSTimeDly(OS_TICKS_PER_SEC * 1);

		}
		else {

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
				if (zone0State == 0) {
					printf("\nzone 000 \n");
				}
				OSSemPost(zone0Sem);
				break;

			case ID_SWITCH_2:
				OSSemPend(zone1Sem, 0, &err);
				if (zone1State == 0) {
					printf("\nzone 111 \n");
				}
				OSSemPost(zone1Sem);
				break;

			case ID_SWITCH_3:
				OSSemPend(zone2Sem, 0, &err);
				if (zone2State == 0) {
					printf("\nzone 222 \n");
				}
				OSSemPost(zone2Sem);
				break;

			case ID_SWITCH_4:
				OSSemPend(zone3Sem, 0, &err);
				if (zone3State == 0) {
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
		if (zone0State == 0) {
			printf("\nzone 00000000000000000\n");
		}
        OSSemPost(zone0Sem);

		/*
		 * Zone 2 check
		 */
 		OSSemPend(zone1Sem, 0, &err);
		if (zone1State == 0) {
			printf("\nzone 1111111111111111\n");
		}
        OSSemPost(zone1Sem);

		/*
		 * Zone 3 check
		 */
 		OSSemPend(zone2Sem, 0, &err);
		if (zone2State == 0) {
			printf("\nzone 22222222222222222\n");
		}
        OSSemPost(zone2Sem);

		/*
		 * Zone 4 check
		 */
 		OSSemPend(zone3Sem, 0, &err);
		if (zone3State == 0) {
			printf("\nzone 33333333333333333\n");
		}
        OSSemPost(zone3Sem);

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
	// OSTaskCreate(buzzerTask, NULL, 512, TASK_BUZZER_PRIORITY);
}

nodebug void ClearScreen() {
   	printf( "\x1Bt" );            	// Space Opens Window
}

nodebug void DispStr(int x, int y, char *s) {
	x += 0x20;
   	y += 0x20;
   	printf("\x1B=%c%c%s", x, y, s);
}

void initHttp() {
	char buffer[16];
	brdInit();

  	// configure IO channels DIO00 - DIO03 as digital outputs (sinking type outputs)
   	digOutConfig(0x000F);

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
   	strcpy(led_LED0, "ledon.gif");
   	strcpy(led_LED1, "ledon.gif");
   	strcpy(led_LED2, "ledon.gif");
   	strcpy(led_LED3, "ledon.gif");
}

void initStmpMethod() {
	initEmails();

}

void main (void) {
	// initialize uC/OS-II
	zone0State = 0;
	zone1State = 0;
	zone2State = 0;
	zone3State = 0;

	initHttp();

	// verification for email server
	#ifdef USE_SMTP_AUTH
		smtp_setauth ("alarm+yalost.me", "Al@rm");
	#endif

	initStmpMethod();

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






