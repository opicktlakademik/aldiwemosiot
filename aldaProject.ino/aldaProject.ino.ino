#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>


const char* ssid = "@wifi.id";
const char* pass = "KokAndaMiskin";
const char* apiKey = "aldiwemosapikey";
ESP8266WebServer server(80);
unsigned long long int last = 0;
int door_pos = 0;
int lock_pos = 0;
Servo door;
Servo lock;

String checkCredential(){
  String pesan = "lanjut bro";
  if(server.hasArg("apikey") == false || server.arg("apikey") != apiKey ){
    //server.send(401, "text/plain", "API key is not correct for this wemos");
    pesan = "API Key is incorrect";
  }else if(server.hasArg("action") == false){
    //server.send(401, "text/plain", "The data you sent is incorrect");
    pesan = "Action is undeclared";
  }
  return pesan;
}

void setIP(){
  startRequestToMyServer:
  String door = "";
  String lock = "";
  door += door_pos;
  lock += lock_pos;
  HTTPClient httpclient;
  httpclient.begin("http://alr.rajamitra.net/api/mc/handle"); //inisialisasi domain
  httpclient.addHeader("ApiKey", apiKey); // inisialisasi apikey on header
  httpclient.addHeader("door", door);
  httpclient.addHeader("lock", lock);
  //send request
  int httpCode = httpclient.GET();
  String payload = httpclient.getString();
  
  if(httpCode == 200 ){
    Serial.print("\n\n[RESPONSE FROM MY SERVER]\n");
    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    Serial.println(payload);
  }else{
    Serial.println("Response Failed");
    Serial.print(payload);
    Serial.println();
    Serial.println("Mengulangi pendaftaran");
    goto startRequestToMyServer;
  }
  httpclient.end();
  return;
}
void handleRoot(){
  String message = "Hi, you are here! i love you!";
  server.send(200, "text/plain", message);
}

void handleLock(){
  Serial.println("Try to Lock");
  lock.write(0);
  lock_pos = 0;
  String message = "The door is locked!";
  server.send(200, "text/plain", message);
}
void handleUnlock(){
  Serial.println("Try to unLock");
  lock.write(80);
  lock_pos = 80;
  String message = "The door is unlocked!";
  server.send(200, "text/plain", message);
}
void handleOpen(){
  Serial.println("Try to open");
  door.write(90);
  door_pos = 90;
  String message = "The door is open!";
  server.send(200, "text/plain", message);
}
void handleClose(){
  Serial.println("Try to close");
  door.write(0);
  door_pos = 0;
  String message = "The door is close!";
  server.send(200, "text/plain", message);
}

void handleCheck(){
  Serial.println("Try to get status");
  Serial.print("status door: "); Serial.println(door.read());
  Serial.print("status lock: "); Serial.println(lock.read());  
  //send response
  String res = "{door:";
  res += door_pos;
  res += ", lock:";
  res += lock_pos;
  res += "}"; 
  server.send(200, "text/json", res);
}

void handleAction(){
  String checking = checkCredential();
  String action = server.arg("action");

  if (checking == "lanjut bro"){
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
  }else{
    server.send(401, "text/plain", checking);
  }
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
  Serial.println(" ");
  Serial.println("Inisialisasi Door dan Lock di posisi 0");
  door.attach(D2);
  lock.attach(D3);
  door.write(0);
  lock.write(0);
  door_pos = 0;
  lock_pos = 0;
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
  }
  server.handleClient();
}
