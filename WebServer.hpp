#pragma once

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#include "static.hpp"
extern SubBoard *board;

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

const char *myHostname = "ecto-1";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[33] = "";
char password[65] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 217, 28, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

String wrap(String text){
  String Page = F(HEADER_HTML);
  Page += text;
  Page += F(FOOTER_HTML);
  return Page;
}

void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }
  DEBUG_ESP_PORT.println("Recovered credentials:");
  DEBUG_ESP_PORT.println(ssid);
  DEBUG_ESP_PORT.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}

void connectWifi() {
  DEBUG_ESP_PORT.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  DEBUG_ESP_PORT.print("connRes: ");
  DEBUG_ESP_PORT.println(connRes);
}

void wifiLoop() {
  if (connect) {
    DEBUG_ESP_PORT.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      DEBUG_ESP_PORT.print("Status: ");
      DEBUG_ESP_PORT.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        DEBUG_ESP_PORT.println("");
        DEBUG_ESP_PORT.print("Connected to ");
        DEBUG_ESP_PORT.println(ssid);
        DEBUG_ESP_PORT.print("IP address: ");
        DEBUG_ESP_PORT.println(WiFi.localIP());

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          DEBUG_ESP_PORT.println("Error setting up MDNS responder!");
        } else {
          DEBUG_ESP_PORT.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    DEBUG_ESP_PORT.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

/** Handle root or redirect to captive portal */
void handleRoot() {
  if (captivePortal()) return; // If caprive portal redirect instead of displaying the page.
  server.sendHeader("Cache-Control", "max-age=7200");
  server.send(200, "text/html", wrap(F(INDEX_HTML)));
}

void handleAPI() {
  String command = server.arg("cmd");
  String argument = server.arg("arg");
  DEBUG_ESP_PORT.printf("API call '%s' with args '%s'\n",command,argument);
  
  String Page;
  bool error;
  
  if(command == "enableMod"){
    unsigned int channel = atoi(argument.c_str());
    board->enableMod(channel);
    error = false;
    Page = "Enabled mod " + argument;
  }else if(command == "disableMod"){
    unsigned int channel = atoi(argument.c_str());
    board->disableMod(channel);
    error = false;
    Page = "Disabled mod " + argument;
  }else if(command == "pressButton"){
    unsigned int channel = atoi(argument.c_str());
    board->pressButton(channel);
    error = false;
    Page = "Pressed button " + argument;
  }else if(command == "beep"){
    board->beep();
    error = false;
    Page = "Beeping..." + argument;
  }else{
    error = true;
    Page = "Command not found";
  }
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  if(!error) server.send(200, "text/plain", Page); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  else server.send(400, "text/plain", Page);
  
  server.client().stop(); // Stop is needed because we sent no content length
}

void handleStaticCSS() {
  String CSSFile = F(CSS_FILE_STRING);
  server.sendHeader("Cache-Control", "max-age=604800");
  server.send(200, "text/css", CSSFile); // Empty content inhibits Content-length header so we have to close the socket ourselves.
}

void handleStaticJS() {
  String JSFile = F(JS_FILE_STRING);
  server.sendHeader("Cache-Control", "max-age=604800");
  server.send(200, "application/javascript", JSFile); // Empty content inhibits Content-length header so we have to close the socket ourselves.
}

/** Wifi config page handler */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  DEBUG_ESP_PORT.println("scan start");
  int n = WiFi.scanNetworks();
  DEBUG_ESP_PORT.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  DEBUG_ESP_PORT.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleAbout() {
  server.sendHeader("Cache-Control", "max-age=7200");
  server.send(200, "text/html", wrap(F(ABOUT_HTML)));
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

void createRoutes(){
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/api", handleAPI);
  server.on("/style.css", handleStaticCSS);
  server.on("/script.js", handleStaticJS);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/about", handleAbout);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html",  wrap(F(UPDATE_HTML)));
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    bool hasError = Update.hasError();
    if(hasError){
      server.send(400, "text/plain", "FAIL");
    }else{
       server.send(200, "text/plain", "Update Successful. Restarting...");
       delay(500);
       ESP.restart();
    }
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      DEBUG_ESP_PORT.setDebugOutput(true);
      WiFiUDP::stopAll();
      DEBUG_ESP_PORT.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(DEBUG_ESP_PORT);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(DEBUG_ESP_PORT);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        DEBUG_ESP_PORT.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(DEBUG_ESP_PORT);
      }
      DEBUG_ESP_PORT.setDebugOutput(false);
    }
    yield();
  });
}

void setupWifi() {
  DEBUG_ESP_PORT.println();
  DEBUG_ESP_PORT.println("Configuring access point...");
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500);
  
  DEBUG_ESP_PORT.print("AP IP address: ");
  DEBUG_ESP_PORT.println(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  createRoutes();
  server.begin(); // Web server start
  DEBUG_ESP_PORT.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}
