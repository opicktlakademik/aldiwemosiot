#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>


const char* ssid = "ALPRO";
const char* pass = "opickganteng123";
const char* apiKey = "aldiWemosApiKey";
ESP8266WebServer server(80);
unsigned long long int last = 0;
int door_pos;
int lock_pos;
Servo door;
Servo lock;

void checkCredential(){
  if(server.hasArg("apikey") == false || server.arg("apikey") != apiKey )
  {
    server.send(401, "text/plain", "API key is not correct for this wemos");
    return;
  }else if(server.hasArg("action")){
    server.send(401, "text/plain", "The data you sent is incorrect");
    return;
  }
}

void setIP(){
  startRequestToMyServer:
  HTTPClient httpclient;
  httpclient.begin("http://domainname.com/igoapi/setip"); //inisialisasi domain
  httpclient.addHeader("MyAPI", apiKey); // inisialisasi apikey on header
  //send request
  int httpCode = httpclient.GET();
  String payload = httpclient.getString();
  
  if(httpCode == 200 ){
    Serial.print("\n\n[RESPONSE FROM MY SERVER]\n");
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    Serial.println(payload);
  }else{
    goto startRequestToMyServer;
  }
  httpclient.end();
  return;
}

void inisialisasiDoorAndLock(){
  door_pos = 0;
  lock_pos = 0;
  door.write(door_pos);
  lock.write(lock_pos);
}
void handleRoot(){
  String message = "Hi, you are here! i fuck you!";
  server.send(200, "text/plain", message);
}

void handleLock(){
  Serial.println("Try to Lock");
  lock.write(180);
  String message = "Hi, you are here! The door is locked!";
  server.send(200, "text/plain", message);
}
void handleUnlock(){
  Serial.println("Try to unLock");
  lock.write(0);
  String message = "Hi, you are here! The door is unlocked!";
  server.send(200, "text/plain", message);
}
void handleOpen(){
  Serial.println("Try to open");
  door.write(0);
  String message = "Hi, you are here! The door is open!";
  server.send(200, "text/plain", message);
}
void handleClose(){
  Serial.println("Try to close");
  door.write(180);
  String message = "Hi, you are here! The door is close!";
  server.send(200, "text/plain", message);
}

void handleCheck(){
  Serial.println("Try to get status");
  Serial.print("status door: "); Serial.println(door.read());
  Serial.print("status lock: "); Serial.println(lock.read());  
  //send response
  String res = "{door:";
  res += door.read();
  res += ", lock:";
  res += lock.read();
  res += "}";
  server.send(200, "text/json", res);
}

void handleAction(){
  checkCredential();
  String action = server.arg("action");

  if(action == "open"){
    handleOpen();
  }else if(action == "close"){
    handleClose();
  }else if(action == "lock"){
    handleLock();
  }else if(action == "unlock"){
    handleUnlock();
  }else if(action == "check"){
    handleCheck();
  }else{
    server.send(401, "text/plain", "Tidak ada aksi yang bisa dijalankan");
  }
  return;
}

void handleNotFound(){
  String message = "File Not Found \n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";  
  }
  server.send(404, "text/plain", message);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Inisialisasi Door dan Lock di posisi 0");
  door.attach(D2);
  lock.attach(D3);
  door.write(0);
  lock.write(0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println(" ");
  Serial.printf("Connect to %s", ssid);
  Serial.println(" ");

  //wait for wifi connected
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println(" ");
  Serial.printf("\n [Connected] \n");
  Serial.printf("SSID: %s", ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" ");
  
  if(MDNS.begin("esp8266")){
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/handleAction", handleAction);
  server.onNotFound(handleNotFound);
  server.begin();
  setIP();
}

void loop() {
  if(millis() > last + 300000){
    setIP();
    last = millis();
  }else{
    server.handleClient();
  }
}
