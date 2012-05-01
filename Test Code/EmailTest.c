/*******************************************************************************
        smtp.c
        Z-World, 2000

        A small program that uses the SMTP library
        to send an e-mail.
*******************************************************************************/
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
#define TCPCONFIG 3

/*
 *   These macros need to be changed to the appropriate values or
 *   the smtp_sendmail(...) call in main() needs to be changed to
 *   reference your values.
 */

#define FROM     "alarm@yalost.me"
#define TO       "tobymessinger@hotmail.com"
#define SUBJECT  "You've got mail!"
#define BODY     "Visit the Rabbit Semiconductor web site.\r\n" \
	"There you'll find the latest news about Dynamic C."

/*
 *   The SMTP_SERVER macro tells DCRTCP where your mail server is.  This
 *   mail server MUST be configured to relay mail for your controller.
 *
 *   This value can be the name or the IP address.
 */

/*
 *   The SMTP_SERVER macro tells DCRTCP where your mail server is.  This
 *   mail server MUST be configured to relay mail for your controller.
 *
 *   This value can be the name or the IP address.
 */

#define SMTP_SERVER "mail.yalost.me"
//#define SMTP_SERVER "100.42.54.124"

#define SMTP_PORT		26


/*
 *   The SMTP_DOMAIN should be the name of your controller.  i.e.
 *   "somecontroller.somewhere.com"  Many SMTP servers ignore this
 *   value, but some SMTP servers use this field.  If you have
 *   problems, turn on the SMTP_DEBUG macro and see were it is
 *   bombing out.  If it is in the HELO command consult the
 *   person in charge of the mail server for the appropriate value
 *   for SMTP_DOMAIN. If you do not define this macro it defaults
 *   to the value in MY_IP_ADDRESS.
 *
 */

//#define SMTP_DOMAIN "mycontroller.mydomain.com"

/*
 *   The SMTP_VERBOSE macro logs the communications between the mail
 *   server and your controller.  Uncomment this define to begin
 *   logging
 */

#define SMTP_VERBOSE

/*
 *   The USE_SMTP_AUTH macro enables SMTP Authentication, a method
 *   where the client authenticates with the server before sending
 *   a message.  Call smtp_setauth() before smtp_sendmail() to set
 *   the username and password to use for authentication.
 */
#define USE_SMTP_AUTH

/*
 *   If the following macro is defined, then if SMTP authentication
 *   fails, the library will NOT attempt non-authenticated SMTP.
 */
#define SMTP_AUTH_FAIL_IF_NO_AUTH

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use dcrtcp.lib
#use smtp.lib

void main()
{
	sock_init();
	// Wait for the interface to come up
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}

//   #if _USER
   	smtp_setserver(SMTP_SERVER);
//   #endif

//#ifdef USE_SMTP_AUTH
	smtp_setauth ("alarm+yalost.me", "Al@rm");
//#endif

//	smtp_sendmail(TO, FROM, SUBJECT, BODY);

  	smtp_sendmail("i242714@rppkn.com", "alarm@yalost.me",
   	"You've got rabbit mail!", "Visit the Rabbit Semiconductor web site.\r\n" \
	"There you'll find the latest news about Dynamic C.");

	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	if(smtp_status()==SMTP_SUCCESS)
		printf("Message sent\n");
	else
		printf("Error sending message\n");
}