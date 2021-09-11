#define ARDUINOJSON_USE_LONG_LONG 1
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "access_sectets.h"
#include "Display_Handler.h"
#include "time.h"

const int ButtonNext = 18;
const int ButtonPrev = 2;
const int ButtonAction = 15;
const int SpareButton = 4;
const int Buzzer = 5;

unsigned long ButtonNextPressTime =  0l;
unsigned long ButtonPrevPressTime =  0l;
unsigned long ButtonActionPressTime =  0l;

bool lastStateButtonNext = false ;
bool lastStateButtonPrev = false ;
bool lastStateButtonAction = false ;

byte ButtonNextRqst = 0 ; // 0 - NULL, 1 - pressed, 2- Held 3- Stuck
byte ButtonPrevRqst = 0 ;
byte ButtonActionRqst = 0 ;

unsigned long ButtonPresstime = 30;
unsigned long ButtonHeldtime = 1000;
unsigned long ButtonStucktime = 30000;
unsigned int NumberOfProjects = 0;
unsigned int NumberOfTasks = 0;

String selectedProjectID = "";

unsigned int SCREEN_NUMBER = 1; // 1 - Project select screen, 2 - Task Display Screen


DynamicJsonDocument doc(8192);




const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const String GET_PROJECTS_URL = "https://api.todoist.com/rest/v1/projects";
const String GET_TASK_URL = "https://api.todoist.com/rest/v1/tasks";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;
const int   daylightOffset_sec = 0;

unsigned long timer_updated_at;
int ProjectIndex = 0;
int TaskIndex = 0;

void updateTime()
{
  struct tm timeinfo;


  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  DisplayArea1(String(timeinfo.tm_hour) + ":" +String(timeinfo.tm_min));
  //Serial.println(String(timeinfo.tm_hour) + ":" +String(timeinfo.tm_min));
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  timer_updated_at = millis();
}


void monitorNextButton(){
  bool buttonNextState = digitalRead(ButtonNext);
  if( buttonNextState != lastStateButtonNext ){
    if(buttonNextState){
      ButtonNextPressTime = millis();
    }
    else{

      if(millis() < ButtonNextPressTime){
        ButtonNextRqst = 0;

      }
      else if((millis() - ButtonNextPressTime) > ButtonStucktime){
        ButtonNextRqst = 3;
      } else if((millis() - ButtonNextPressTime) > ButtonHeldtime){
        ButtonNextRqst = 2;
        Serial.println("Next Button Held");
      } else if((millis() - ButtonNextPressTime) > ButtonPresstime){
        ButtonNextRqst = 1;
        Serial.println("Next Button Pressed");
      } else {
        ButtonNextRqst = 0;
      }
    }
  }
  lastStateButtonNext = buttonNextState;
}
void monitorPrevButton(){
  bool buttonPrevState = digitalRead(ButtonPrev);

  if( buttonPrevState != lastStateButtonPrev ){
    if(buttonPrevState){
      ButtonPrevPressTime = millis();
    }
    else{

      if(millis() < ButtonPrevPressTime){
        ButtonPrevRqst = 0;
      }
      else if((millis() - ButtonPrevPressTime) > ButtonStucktime){
        ButtonPrevRqst = 3;
      } else if((millis() - ButtonPrevPressTime) > ButtonHeldtime){
        ButtonPrevRqst = 2;
        Serial.println("Previous Button Held");
      } else if((millis() - ButtonPrevPressTime) > ButtonPresstime){
        ButtonPrevRqst = 1;
        Serial.println("Previous Button Pressed");
      } else {
        ButtonPrevRqst = 0;
      }
    }
  }
  lastStateButtonPrev = buttonPrevState;
}

void monitorActionButton(){
  bool buttonActionState = digitalRead(ButtonAction);
  if( buttonActionState != lastStateButtonAction ){
    if(buttonActionState){
      ButtonActionPressTime = millis();
    }
    else{

      if(millis() < ButtonActionPressTime){
        ButtonActionRqst = 0;
      }
      else if((millis() - ButtonActionPressTime) > ButtonStucktime){
        ButtonActionRqst = 3;
      } else if((millis() - ButtonActionPressTime) > ButtonHeldtime){
        ButtonActionRqst = 2;
        Serial.println("Action Button Held");
      } else if((millis() - ButtonActionPressTime) > ButtonPresstime){
        ButtonActionRqst = 1;
        Serial.println("Action Button Pressed");
      } else {
        ButtonActionRqst = 0;
      }
    }
  }
  lastStateButtonAction = buttonActionState;
}


void Get_Projects(){

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(GET_PROJECTS_URL);

    http.addHeader("Authorization","Bearer " + String(SECRET_API_KEY));
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
      StaticJsonDocument<256> filter;

      JsonObject filter_0 = filter.createNestedObject();
      filter_0["id"] = true;
      filter_0["name"] = true;
      doc.clear();
      DeserializationError error = deserializeJson(doc, http.getString(),DeserializationOption::Filter(filter));
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());

      }
      NumberOfProjects = doc.size();
      //String payload = http.getString();
      Serial.println(httpCode);
      //Serial.println(payload);
    }

    else {
      Serial.println("Error on HTTP request");
    }

    http.end();


    //Free the resources
  }
}

void Get_Tasks(){

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(GET_TASK_URL + "?project_id="+ selectedProjectID);
    Serial.println("hitting URL : " +GET_TASK_URL + "?project_id="+ selectedProjectID);
    http.addHeader("Authorization","Bearer " + String(SECRET_API_KEY));
    int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
      String payload = http.getString();
      Serial.println("Payload =>");
      Serial.println(payload);
      StaticJsonDocument<256> filter;

      JsonObject filter_0 = filter.createNestedObject();
      filter_0["id"] = true;
      filter_0["project_id"] = true;
      filter_0["content"] = true;
      //filter_0["due"] = true;
      filter_0["priority"] = true;

      //doc.clear();
      DeserializationError error = deserializeJson(doc, payload,DeserializationOption::Filter(filter));
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());

      }
      NumberOfTasks = doc.size();
      //String payload = http.getString();
      Serial.println(httpCode);
      //Serial.println("Payload =>");
      //Serial.println(payload);
      Serial.println("Number of tasks ");
      Serial.println(NumberOfTasks);
      serializeJson(doc, Serial);

    }

    else {
      Serial.println("Error on HTTP request");
    }

    http.end();
    JsonObject root_0 = doc[0];
    Serial.println(root_0);
const char* root_0_content = root_0["content"]; // "add today's goals"
Serial.println(root_0_content);
int root_0_priority = root_0["priority"]; // 1
Serial.println(root_0_priority);

JsonObject root_1 = doc[1];
Serial.println(root_1);



//JsonObject root_2 = doc[2];
Serial.println(root_1);


    //Free the resources
  }

}
bool closeTask(String taskId){
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(GET_TASK_URL + "/"+ String(taskId) + "/close");
    Serial.println(GET_TASK_URL + "/"+ String(taskId) + "/close");
    http.addHeader("Authorization","Bearer " + String(SECRET_API_KEY));
    int httpCode = http.POST("");
    if (httpCode > 0) { //Check for the returning code
      http.end();
      return true;
    }

    else {
      Serial.println("Error on HTTP request");
      http.end();
      return false;
    }




    //Free the resources
  }
  return false;

}

void UpdateScreen(unsigned int x){
  switch (x) {
    case 1:
    //do something when var equals 1
    SCREEN_NUMBER = 1;
    TaskIndex = 0;
    Serial.println("Get project");
    Get_Projects();
    {
      ProjectIndex=0;
      JsonObject root_0 = doc[ProjectIndex];
      const char* root_0_content = root_0["name"];
      DisplayArea3("Select a Project >> " +String(root_0_content));
      Serial.println("Select a Project >> " +String(root_0_content));
    }
    break;
    case 2:
    SCREEN_NUMBER = 2;
    ProjectIndex = 0;
    Serial.println("Load Task");
    Get_Tasks();

      if(NumberOfTasks !=0){
      JsonObject root_0 = doc[0];
      const char* root_0_content = root_0["content"];
      DisplayArea3(String(root_0_content));
      Serial.print(" Displaying first task >> ");
      Serial.println(root_0_content);
    } else{
      DisplayArea3("No Tasks present long press Prev button to go back");
    }



    //do something when var equals 2
    break;
    default:
    // if nothing else matches, do the default
    // default is optional
    break;
  }



}


void taskDisplayPanelHandler(){
  //Check Exit Condition
    if(NumberOfTasks !=0){
  if(ButtonNextRqst == 1){
    TaskIndex ++;
    if(TaskIndex + 1 > NumberOfTasks ) TaskIndex = 0;
    JsonObject root_0 = doc[TaskIndex];
    //long root_0_id = root_0["id"];
    const String root_0_name = root_0["content"];

    DisplayArea3(root_0_name);
    Serial.println(" Task >> ");
    Serial.println(TaskIndex);
    Serial.println(root_0_name);
    serializeJson(root_0, Serial);


  }
  if(ButtonPrevRqst == 1){
    TaskIndex --;
    if(TaskIndex < 0 ) TaskIndex = NumberOfTasks - 1 ;
    JsonObject root_0 = doc[TaskIndex];
    //long root_0_id = root_0["id"];
    const String root_0_name = root_0["content"];

    DisplayArea3(root_0_name);
    Serial.println(" Task >> ");
    Serial.println(TaskIndex);
    Serial.println(root_0_name);
    serializeJson(root_0, Serial);

  }
  if(ButtonActionRqst == 2){
    JsonObject root_0 = doc[TaskIndex];
    JsonVariant Tid = root_0["id"];
    //DisplayArea3("Are you done with this task ?           Press > to confirm");
    closeTask(Tid.as<String>());
    TaskIndex = 0;
    UpdateScreen(2);

  }}
  if(ButtonPrevRqst == 2){
    ProjectIndex = 0;
    UpdateScreen(1);
  }

  //perform state tasks



}

void  projectSelectPanelHandler(){
  if(ButtonNextRqst == 1){
    ProjectIndex ++;
    if(ProjectIndex + 1 > NumberOfProjects ) ProjectIndex = 0;
    JsonObject root_0 = doc[ProjectIndex];
    //long root_0_id = root_0["id"];
    const String root_0_name = root_0["name"];

    DisplayArea3("Select a Project >> " + root_0_name);
    Serial.println("Load Next Project");
  }
  if(ButtonPrevRqst == 1){
    ProjectIndex --;
    if(ProjectIndex < 0 ) ProjectIndex = NumberOfProjects - 1 ;
    JsonObject root_0 = doc[ProjectIndex];
    //long root_0_id = root_0["id"];
    const String root_0_name = root_0["name"];

    DisplayArea3("Select a Project >> " + String(root_0_name));
    Serial.println(root_0_name);
    Serial.println("Load Prev Project");

  }
  if(ButtonActionRqst == 1){
    Serial.println("extract project ");
    Serial.println(ProjectIndex);
    JsonObject root_0 = doc[ProjectIndex];
    Serial.println("get project ID");
    JsonVariant  sltProjectID = root_0["id"];
    selectedProjectID = sltProjectID.as<String>();
    Serial.println("Load Task for Project Id :" + selectedProjectID);
    UpdateScreen(2);

  }//Check Exit Condition


  //perform state tasks

}



void setup() {
  Serial.begin(115200);
  Display_Handler();
  pinMode(Buzzer, OUTPUT);
  pinMode(ButtonNext, INPUT);
  pinMode(ButtonPrev, INPUT);
  pinMode(ButtonAction, INPUT);



  DisplayArea3("Hello!");
  delay(3000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DisplayArea3("Connecting to WiFi..");
  }
  DisplayArea3("Connected to the WiFi network");
  delay(500);
  clearArea3();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  updateTime();
  Get_Projects();

  JsonObject root_0 = doc[ProjectIndex];
  //long root_0_id = root_0["id"];
  const String root_0_name = root_0["name"];
  DisplayArea3("Select a Project >> " +root_0_name);
  Serial.println(root_0_name);
}

void loop() {

  if((millis()-timer_updated_at)<0 || (millis()-timer_updated_at) > 1000){
    updateTime();
  }

  monitorNextButton();
  monitorPrevButton();
  monitorActionButton();


  if(SCREEN_NUMBER == 1){
    projectSelectPanelHandler();
  }else if(SCREEN_NUMBER == 2 ){
    taskDisplayPanelHandler();
  }


  ButtonNextRqst = 0 ; // 0 - NULL, 1 - pressed, 2- Held 3- Stuck
  ButtonPrevRqst = 0 ;
  ButtonActionRqst = 0 ;


}
