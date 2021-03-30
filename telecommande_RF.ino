#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "settings.h" // paramètres wifi


int emiter = D6;
int reciever = D5;

typedef struct {
  int32_t code;
  int bits;
  int x, y;
  char* lbl;
  } btn ;
char lbl_ZAP[] = "ZAP";
char lbl_vibrate[] = "vibrate";

btn boutons[] = {
  {.code=2782728, .bits=24, .x=50, .y=33, .lbl=lbl_ZAP},
  {.code=2782724, .bits=24, .x=50, .y=67, .lbl=lbl_vibrate}
};


RCSwitch mySwitchT = RCSwitch();
RCSwitch mySwitchR = RCSwitch();
ESP8266WebServer server ( 80 );

char* homePage = NULL;
int homePageLen = 0;
int homePageAllocated = 0;
int bl_size = 1024 * 16;
void AddHomePage(String str){
  if (homePage == NULL) homePage = (char*)malloc(bl_size);
  int size = str.length();
  if (homePageLen + size + 1 >= homePageAllocated){
    homePageAllocated = bl_size * ( (homePageLen + size + 1) / bl_size + 1 );
    homePage = (char*)realloc(homePage, homePageAllocated);
  }
  strcpy(homePage + homePageLen, str.c_str());
  homePageLen += size;
}

void ServerHome() {
  server.send(200, "text/html", homePage);
}
void WaitWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  #ifdef DEBUG
  Serial.printf("[Wifi] OK\n");
  #endif
}

void ServerRequest() {
  int id = server.arg("id").toInt();
  Serial.printf("BOUTON : %d\n", id);
  mySwitchT.send(boutons[id].code, boutons[id].bits);
  int state = server.arg("state").toInt();
  server.send(200, "text/plain", "{\"status\" : \"OK\"} ");
}

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.setDebugOutput(true);
  WiFi.begin(ssid, password);
  WaitWifi();
  mySwitchR.enableReceive(reciever);
  mySwitchT.enableTransmit(emiter);

  int ScaleX = 2, ScaleY = 3;

  int MinX = 100;
  int MinY = 100;
  
  int WIDTH = 120 * ScaleX + MinX;
  int HEIGHT = 120 * ScaleY + MinY;

  AddHomePage((String)"<html><head><style type=\"text/css\">input{ padding: 10px;\n");
  AddHomePage((String)" border-radius: 10px;background-color:#234;color:#FFF;border:1px solid #000;}\n");
  AddHomePage((String)"</style><script type=\"text/javascript\">\n");
  AddHomePage((String)"function btn(i){\n");
  AddHomePage((String)"  document.getElementById(\"LOCK\").style.display=\"block\";");
  AddHomePage((String)"  var oReq = new XMLHttpRequest();\n");
  AddHomePage((String)"  oReq.onreadystatechange = function(){ if(oReq.readyState === 4 && oReq.status === 200) { document.getElementById(\"LOCK\").style.display=\"none\"; }};\n");
  AddHomePage((String)"  oReq.open(\"get\", \"/request?id=\"+i);\n");
  AddHomePage((String)"  oReq.send();\n");
  AddHomePage((String)"  }\n");
  AddHomePage((String)"</script><title>Télécommande!</title></head><body>");
  AddHomePage((String)"<div style=\"width:");
  AddHomePage((String)(WIDTH));
  AddHomePage((String)"px;height:");
  AddHomePage((String)(HEIGHT));
  AddHomePage((String)"px; left: 25px; top: 25px; position: absolute;border-radius: 10px; background-color:#000;\"></div>\n");
  int boutons_nbr = sizeof(boutons) / sizeof(boutons[0]);
  for (int i = 0; i < boutons_nbr; i ++){
    AddHomePage((String)"<input type=\"button\" style=\"transform: translate(-50%, -50%);position:absolute;left: ");
    AddHomePage((String)(MinX + boutons[i].x * ScaleX));
    AddHomePage((String)"px;top: ");
    AddHomePage((String)(MinY + boutons[i].y* ScaleY));
    AddHomePage((String)"px;\" value=\"");
    AddHomePage(boutons[i].lbl);
    AddHomePage((String)"\" onclick=\"btn(");
    AddHomePage((String)(i));
    AddHomePage((String)")\">\n");
  }
  AddHomePage((String)"<div style=\"width:340px;height:460px; left: 25px; top: 25px; position: absolute;border-radius: 10px; background-color:#AAA;display:none\" id=\"LOCK\"></div></body></html>");
  
  server.on("/", ServerHome);
  server.on("/request", ServerRequest);
  server.begin();
  String WifiString(WiFi.localIP().toString());
  Serial.printf("IP : %s\n", WifiString.c_str());
}

void loop() {
  server.handleClient();
  
  if (mySwitchR.available()) {
    Serial.print("Received ");
    Serial.print( mySwitchR.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitchR.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.println( mySwitchR.getReceivedProtocol() );

    mySwitchR.resetAvailable();
  }
  /*
  Received 2782728 / 24bit Protocol: 1
  Received 2782724 / 24bit Protocol: 1
  */


}
