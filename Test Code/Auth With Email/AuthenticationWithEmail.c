/*******************************************************************************
        Samples\TcpIp\RabbitWeb\humidity.c
        Z-World, 2004

        This is a simple RabbitWeb sample based on a humidity monitor.
        It demonstrates some simple #web variable registration along with the
        authentication features.  See the pages:

        samples\tcpip\rabbitweb\pages\humidity_monitor.zhtml

        and

        samples\tcpip\rabbitweb\pages\humidity_admin.zhtml

        for the corresponding ZHTML pages.

        When accessing the administration page, note that the username is
        "harpo" and the password is "swordfish".

*******************************************************************************/

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

/********************************
 * End of configuration section *
 ********************************/

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"

/*
 * These pages contain the ZHTML portion of the demonstration.  The first has
 * a status page, while the second has a configuration interface.
 */
#ximport "pages/humidity_monitor.zhtml"	monitor_zhtml
#ximport "pages/humidity_admin.zhtml"		admin_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", monitor_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/admin/index.zhtml", admin_zhtml)
SSPEC_RESOURCETABLE_END

// The following line defines an "admin" group, which will be used to protect
// certain variables.  This must be defined before we register the variables
// below.
#web_groups admin

// The #web lines below register C variables with the web server, such that
// they can be used with the RabbitWeb HTTP server enhancements.

// The following lines creates a "hum" variable to keep the current humidity
// value, and allows read-only access to it.
int hum;
#web hum groups=all(ro)

// This creates a humidity alarm variable, which can be modified by the admin
// group, but is only readable by everybody else.
int hum_alarm;
#web hum_alarm ((0 < $hum_alarm) && ($hum_alarm <= 100)) groups=all(ro),admin

// The following two sets of registrations are for an alarm interval variable
// and an alarm email address.
int alarm_interval;
char alarm_email[50];
#web alarm_interval ((0 < $alarm_interval) && ($alarm_interval < 30000)) \
                    groups=all(ro),admin
#web alarm_email groups=all(ro),admin

void main(void)
{
   int userid;

	// Initialize the values
	hum = 50;
	hum_alarm = 75;
	alarm_interval = 60;
	strcpy(alarm_email, "somebody@nowhere.org");

	// Initialize the TCP/IP stack and HTTP server
   sock_init();
   http_init();

   // If the browser specifies a directory (instead of a proper resource name)
   // default to using "index.zhtml" in that directory, if it exists.
   // If we don't use this function, the default is "index.html" which won't
   // work for this sample.
   http_set_path("/", "index.zhtml");

	tcp_reserveport(80);

	// The following line limits access to the "/admin" directory to the admin
	// group.  It also requires basic authentication for the "/admin"
	// directory.
   sspec_addrule("/admin", "Admin", admin, admin, SERVER_ANY,
                 SERVER_AUTH_BASIC, NULL);

	// The following two lines create an "admin" user and adds it to the admin
	// group.
   userid = sauth_adduser("rabbit", "fish", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);

   // Add our own users
   // Ario
   userid = sauth_adduser("rabbit", "fish", SERVER_ANY);
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

	// This drives the HTTP server.
   while(1) {
      http_handler();
   }
}

