#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include "driver/uart.h"
#include <espconn.h>
#include <mem.h>

// Contains ssid and password
#include "user_config.h"

bool readyToDisconnect = 0;

// Data structure for the configuration of your wireless network.
// Will contain ssid and password for your network.
struct station_config stationConf;

// This function is setup to be called when data is received.
void ICACHE_FLASH_ATTR ssRecvCb(void *arg, char *data, unsigned short len) {
  char buf[32];
  int l;
  struct espconn *pespconn = (struct espconn *)arg;
  os_printf("Receive (%d): %s", len , data);
  len = os_sprintf(buf, "Goodbye %s\n", data);
  
  // Send data to the client.  For anything other than a very simple client
  // you will need logic to make sure that the data to be sent from the last espconn_sent 
  // has been sent before you call espconn_sent again.
  espconn_sent(pespconn, buf, len);
  
  // Set a flag so that once the data has been sent the client is
  // disconnected.
  readyToDisconnect = 1;
}

// This function is set up to be called when there is an error.
// For example if sending data to the client fails this function.
// will be called.
static void ICACHE_FLASH_ATTR ssRecoCb(void *arg, sint8 err) {
  struct espconn *pespconn = (struct espconn *)arg;
  os_printf("Reconnect\n");
}

// This function is set up to be called after data has been successfully
// sent to the client.
static void ICACHE_FLASH_ATTR ssSentCb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  os_printf("Sent\n");
  if (readyToDisconnect) espconn_disconnect(pespconn);
}

// This function is set up to be called when the client disconnects.
static void ICACHE_FLASH_ATTR ssDiscCb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  os_printf("Disconnect\n");
  
  // Reset exit flag for the next connection.
  readyToDisconnect = 0;
}

// This function is set up to be called when a client connects.
void ICACHE_FLASH_ATTR ssConnCb(void *arg) {
  struct espconn *pespconn = (struct espconn *)arg;
  
  // This simple server is designed for only one client / connection at a time
  // If we get a second client / connection disconnect it immediately
  if (pespconn->link_cnt > 0) {
    espconn_disconnect(pespconn);
    os_printf("Max link count of 1 exceeded\n");
    return;
  }
  
  // Print the remote ip address and port to UART1 (debug)
  os_printf("Connection from: %d.%d.%d.%d:%d\n",
    pespconn->proto.tcp->remote_ip[0],
    pespconn->proto.tcp->remote_ip[1],
    pespconn->proto.tcp->remote_ip[2],
    pespconn->proto.tcp->remote_ip[3],
    pespconn->proto.tcp->remote_port);

	// Set up the rest of the callbacks
  //  recvcb - called when data is received by the service.
  //  reconcb - called when when an error occurs
  //  sendcb - called when data has been sent
  //  disconcb - called when the client disconnects
  espconn_regist_recvcb(pespconn, ssRecvCb);
	espconn_regist_reconcb(pespconn, ssRecoCb);
	espconn_regist_sentcb(pespconn, ssSentCb);
  espconn_regist_disconcb(pespconn, ssDiscCb);
  
  // Send data to the client.  For anything other than a very simple client
  // you will need logic to make sure that the data to be sent from the last espconn_sent 
  // has been sent before you call espconn_sent again.
  espconn_sent(pespconn, "Name? ", 6);
}

void ICACHE_FLASH_ATTR ssServerInit() {
  struct espconn * pSimpleServer;
  
  // Allocate an "espconn" for your service
  pSimpleServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
  ets_memset( pSimpleServer, 0, sizeof( struct espconn ) );

  // Create and initialize the service
  espconn_create( pSimpleServer );
  pSimpleServer->type = ESPCONN_TCP;
  pSimpleServer->state = ESPCONN_NONE;
  pSimpleServer->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  pSimpleServer->proto.tcp->local_port = 1234;

  // Set a function (arg2) to get called every time you get an incoming 
  // connection on the service (arg1).
  espconn_regist_connectcb(pSimpleServer, ssConnCb);

  // Start listening for connections for the service (arg1)
  espconn_accept(pSimpleServer);

  // Session time out in for server (arg1) in seconds (arg2).
  // For all connections (arg2) 0 or single connection 1.
  // If not set defaults to 10 seconds.
  espconn_regist_time(pSimpleServer, 60, 0);
}

void ICACHE_FLASH_ATTR wifiInit() {
  // Copy the ssid and the password into the data structure for later call.
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 32);

  // Set the wifi mode.  Can be STATION_MODE, SOFTAP_MODE or STATIONAP_MODE.
  // In this caswe we just want to connect as a client on the wireless network
  // so we use station  mode.
  wifi_set_opmode( STATION_MODE );
  
  // Set the ssid and password of the network you want to connect to.
  wifi_station_set_config(&stationConf);
}

void user_init(void) {
  // Configure the UART0 and UART1 (TX only) to 115200
  uart_init(BIT_RATE_115200,BIT_RATE_115200);
  
  // Print a message that we are starting user_init on debug uart
  os_printf("\nStarting...\n");
  
  // Setup the wifi
  wifiInit();
  ssServerInit();
  
  // Print a message that we have completed user_init on debug uart
  os_printf("Ready...\n");
}
