/**************************************************************************

	digout.c
   Z-World, 2004

	This sample program is for the BL2600 series controllers.

	This program demonstrates using the digOut API function to
   control digital sinking type outputs to toggle LED's ON and
   OFF on the demo board provided with your kit.

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

   4. Connect the following wires from the controller J1 to the DEMO
	   board J1:

	   	From DIO00 to LED1
	   	From DIO01 to LED2
	   	From DIO02 to LED3
	   	From DIO03 to LED4

	Instructions:
	------------
	1. Compile and run this program.

	2. Select a output channel via the STDIO window (per the channels you
      connected to the LED's) to toggle a LED on the demo board.

	3. To check other outputs that are not connected to the demo board,
      you can use a voltmeter to see the output change states.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// Enable all digital outputs (sinking only)
#define DIGOUTCONFIG	0xFFFF

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
	while( (long) (MS_TIMER - done_time) < 0 );
}


// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

///////////////////////////////////////////////////////////////////////////

void main()
{

	auto char s[128];
	auto char display[128];
	auto char channels[8];
	auto int output_status, channel;
	auto int output_level;
   auto int reading;
   auto int configuration;

   // Initialize the controller
	brdInit();

   // Enable digital outputs 0 - 15 (sinking type outputs only)
   digOutConfig(DIGOUTCONFIG);

   // Set all outputs to be OFF
	for(channel = 4; channel < 8; channel++)
	{
   	// Set output to be OFF
		channels[channel] = 1;
		digOut(channel, 1);
   }


	//loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// update high current outputs
		//display[0] = '\0';								//initialize for strcat function
      //for(channel = 0; channel < 4; channel++)	//output to channels 0 - 7
		//{
		//	output_level = channels[channel];		//output logic level to channel
		//	digOut(channel, output_level);
		//}

		for(channel = 4; channel < 8; channel++)	//output to channels 8 - 15
		{
	      reading = digIn(channel);
         channels[channel] = reading;
			output_level = channels[channel];		//output logic level to channel
			digOut(channel - 4, reading);


		}

        //msDelay(500); // Delay to see logic level display

   }
}