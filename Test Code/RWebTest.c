#define TCPCONFIG 1
#define USE_RABBITWEB 1
#use "dcrtcp.lib"
#use "http.lib"
#ximport "my_app.zhtml" my_app_zhtml
SSPEC_MIMETABLE_START
SSPEC_MIME_FUNC(".html", "text/html", zhtml_handler),
SSPEC_MIMETABLE_END
SSPEC_RESOURCETABLE_START
SSPEC_RESOURCE_XMEMFILE("/index.html", my_app_zhtml)
SSPEC_RESOURCETABLE_END
int io_state;
#web io_state
void my_io_polling(void);
void main()
{
sock_init();
http_init();
for (;;) {
my_io_polling();
http_handler();
}
}
void my_io_polling()
{
io_state = read_that_io_device();
}