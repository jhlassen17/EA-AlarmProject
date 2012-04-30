/*******************************************************************************
   telnet.c
   Z-World Inc 2004

   This program takes anything that comes in on a PORT and sends it out
   serial port C. It uses a digital input to indicate that the TCP/IP
   connection should be closed and a digital output to toggle a LED to
   indicate that there's an active connection.

	Connections:
	------------
	1. DEMO board jumper settings:
			- H1 remove all jumpers
			- H2 jumper pins 3-5
              jumper pins 4-6

	2. Connect a wire from the controller J12 GND, to the DEMO board
	   J1 GND.

	3. Connect a wire from the controller J12 +PWR to the DEMO board
	   J1 +K.

   4. Connect SW1 from the demo board to controller connector J1 signal
      pin DIO00.

   5. Connect LED1 from the demo board to controller connector J1 signal
      pin DIO01 on the controller.

	Test Instructions:
	------------------
	1. Compile and run this program.
   2. Start a Telnet session to this board.
   3. View the LED, when toggling ir indicates that there's an active telnet
      connection.
   4. Press SW1 to close the telent connection, LED should stop toggling to
      indicate that the telnet connection has been closed.


*******************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto



/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * Pick the predefined TCP/IP configuration for this sample.  See
 * LIB\TCPIP\TCP_CONFIG.LIB for instructions on how to set the
 * configuration.
 */
#define TCPCONFIG 1

// User macro for TCP port to use for processing
//  23 = telnet.
#define PORT 23

#use "dcrtcp.lib"

/*
 *		MACRO used to indicate when a key has been pressed.
 */

#define KEY_PRESSED	1

/*
 *		TIMEOUT is the number of seconds of no activity before closing
 *		the connection.
 *
 */

#define TIMEOUT		60


/*
 *		Led control MACRO's
 *
 */

#define TOGGLE_LED		0
#define RESET_LED   		1
#define LED_DELAY			100


/*
 *		Serial port Settings
 *		Serial PORT 1=A, 2=B, 3=C, 4=D
 */

#define SERIAL_PORT	3
#define BAUD_RATE		19200L
#define INBUFSIZE		127
#define OUTBUFSIZE 	127

#if (SERIAL_PORT==1)

	#define serXopen  	serAopen
	#define serXread  	serAread
	#define serXwrite 	serAwrite
	#define serXclose 	serAclose
	#define serXwrFlush	serAwrFlush
	#define serXrdFlush	serArdFlush
	#define serXwrFree	serAwrFree

	#define AINBUFSIZE	INBUFSIZE
	#define AOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==2)

	#define serXopen  	serBopen
	#define serXread  	serBread
	#define serXwrite 	serBwrite
	#define serXclose 	serBclose
	#define serXwrFlush	serBwrFlush
	#define serXrdFlush	serBrdFlush
	#define serXwrFree	serBwrFree

	#define BINBUFSIZE	INBUFSIZE
	#define BOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==3)

	#define serXopen  	serCopen
	#define serXread  	serCread
	#define serXwrite 	serCwrite
	#define serXclose 	serCclose
	#define serXwrFlush	serCwrFlush
	#define serXrdFlush	serCrdFlush
	#define serXwrFree	serCwrFree

	#define CINBUFSIZE	INBUFSIZE
	#define COUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==4)

	#define serXopen  	serDopen
	#define serXread  	serDread
	#define serXwrite 	serDwrite
	#define serXclose 	serDclose
	#define serXwrFlush	serDwrFlush
	#define serXrdFlush	serDrdFlush
	#define serXwrFree	serDwrFree

	#define DINBUFSIZE	INBUFSIZE
	#define DOUTBUFSIZE	OUTBUFSIZE

#endif

///////////////////////////////////////////////////////////////////////

/*
 *
 * Function to strip out telnet commands
 * Losses if there are NUL bytes inside of telnet string.
 */

int strip_telnet_cmds(char *ptr, int bytes_read)
{
	int index;

	// write the NUL terminator at the end of the data string
	ptr[bytes_read] = '\0';

	index = 0;
	// Check for telnet command header, if none exit without altering the buffer
	if(ptr[index] == 0xff) {

		// Scan data looking for telnet commands
		while(ptr[index] == 0xff && index < bytes_read) {

			switch(ptr[index+1])
			{
				case 251:
				case 252:
				case 253:
				case 254:
					index += 3;
					break;

				case 255:
					break;

				default:
					index += 2;
					break;
			}
		}
		if(index >= bytes_read)
		{	//return 0 for no valid ascii data recieved in this packet
			return(0);
		}
		//overwrite the leading telnet commands with the valid ascii message
		strcpy(ptr, ptr+index);
	}

	//return with length of buffer
	return(strlen(ptr));
}

/*
 *		led(void)
 *		This function displays a binary count on the LED's.
 *
 */

void led(int mode)
{
	static int toggle;
	static unsigned long led_delay;

	#GLOBAL_INIT{toggle = 0;}

	if(mode == RESET_LED)
	{
		led_delay = MS_TIMER+LED_DELAY;
	  	digOut(1, 1);
	}
	else if((long) (MS_TIMER - led_delay) >= 0 )
	{
		//update led
		if(toggle)
		{	// turn led OFF
			digOut(1, 1);
			toggle = 0;
		}
		else
		{	// turn led ON
			digOut(1, 0);
			toggle = 1;
		}
		// set the interval for the next led update
		led_delay = MS_TIMER+LED_DELAY;
	}
}

/*
 *		debounce_keys(void)
 *		This function checks for a valid keypress
 *
 */

int debounce_key(void)
{
	auto int status;
	static int state;
	static unsigned long debounce_period;

	// initialize state machine during system initialization
	#GLOBAL_INIT {state=0;}

	status = !KEY_PRESSED;
	switch(state)
	{
		case 0:
		   // wait for the switch to be pressed
			if(!digIn(0))
			{  //set debounce period
				state++;
				debounce_period = MS_TIMER + 50; // debounce period = 50ms
			}
			break;

		case 1:
			// check if switch is still being pressed, if not reset state machine
			if((long) (MS_TIMER-debounce_period) >= 0 )
			{
				if(!digIn(0))
				{
					state++;
					status = KEY_PRESSED;
				}
				else
				{
					state = 0;
				}
			}
			break;

		case 2:
		   // wait for the switch to be released
			if(digIn(0))
			{
				state++;
				debounce_period = MS_TIMER + 100;	// debounce period = 100ms
			}
			break;

		case 3:
			// verify that the switch is still released after debounce period
			if((long)(MS_TIMER-debounce_period) >= 0 )
			{
				if(digIn(0))
				{
					state = 0;
				}
				else
				{
					debounce_period = MS_TIMER + 20;	// debounce period = 20ms
				}
			}
			break;

		default: // should never get here!
			state = 0;
			break;
	}
	return(status);
}

/*
 *		main()
 *
 *		This function drives the state machine that manages
 *		the TCP->serial connection.
 *
 *		The INIT state sets a socket up for listening.
 *		The LISTEN state checks for a connection and sets
 *			up the data structures for an open socket.
 *		The OPEN state transfers as much bytes as we have
 *			buffer space from serial to TCP or vice-versa.
 *		The WAITCLOSED state waits for the socket to be
 *			available again.
 *
 */

#define CONNECTION_INIT			0
#define CONNECTION_LISTEN		1
#define CONNECTION_OPEN			2
#define CONNECTION_CLOSED		3

void main()
{
	int state;
	long timeout;
	int bytes_read;

	static char buffer[64];
	static tcp_Socket socket;

	brdInit();
	digOutConfig(0x0002); 			// Configure DIO01 as sinking type output
	serXopen(BAUD_RATE);				// set up serial port
   serMode(0);

	sock_init();						// initialize DCRTCP
	tcp_reserveport(PORT);			// set up PORT for SYN Queueing
											// which will hold a pending connection
											// until we are ready to process it.
	state=CONNECTION_INIT;

	while(1) {
		if(state==CONNECTION_OPEN) {
			if(debounce_key()) {
				sock_close(&socket);
				state=CONNECTION_CLOSED;
			}
		}
		/*
		 *		Make sure that the connection hasn't closed on us.
		 *
		 */

		if(tcp_tick(&socket)==0 && state!=CONNECTION_INIT) {
				sock_close(&socket);
				state=CONNECTION_CLOSED;
		}

		switch(state) {
			case CONNECTION_INIT:
				tcp_listen(&socket,PORT,0,0,NULL,0);
				state=CONNECTION_LISTEN;
				break;

			case CONNECTION_LISTEN:
				if(sock_established(&socket)) {
					state=CONNECTION_OPEN;
					timeout=SEC_TIMER+TIMEOUT;
					serXrdFlush();
					serXwrFlush();
					led(RESET_LED);
 			}
				break;

			case CONNECTION_OPEN:

				/*
				 *		close the socket on a timeout
				 *
				 */
				if((long) (SEC_TIMER-timeout) >= 0) {
					sock_close(&socket);
					state=CONNECTION_CLOSED;
					break;
				}

				/*
				 *		read as many bytes from the socket as we have
				 *		room in the serial buffer. Also strip out the
				 *    telnet commands.
				 */
				bytes_read = 0;
				if(sock_bytesready(&socket) != -1)
				{
					bytes_read=sock_fastread(&socket, buffer, min(sizeof(buffer), serXwrFree()));
					bytes_read=strip_telnet_cmds(buffer, bytes_read);
				}

				/*
				 *		close the socket on an error
				 *
				 */

				if(bytes_read<0) {
					sock_close(&socket);
					state=CONNECTION_CLOSED;
					break;
				}

				/*
				 *		copy any bytes that we read
				 *
				 */

				if(bytes_read > 0) {
					timeout=SEC_TIMER+TIMEOUT;
					serXwrite(buffer, bytes_read);
				}

				/*
				 *		read as many bytes from the serial port as we
				 *		have room in the socket buffer.
				 *
				 */

				bytes_read=serXread(buffer,min(sizeof(buffer),sock_tbleft(&socket)),100);
				if(bytes_read>0) {
					timeout=SEC_TIMER+TIMEOUT;
					if(sock_fastwrite(&socket,buffer,bytes_read)<0) {
						sock_close(&socket);
						state=CONNECTION_CLOSED;
					}
				}
				led(TOGGLE_LED);
				break;

			case CONNECTION_CLOSED:
				serXrdFlush();
				serXwrFlush();
				sprintf(buffer, "\n\rConnection closed\n\r");
				serXwrite(buffer, strlen(buffer));
				led(RESET_LED);
				state=CONNECTION_INIT;
				break;
		}
	}
}