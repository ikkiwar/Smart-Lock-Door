#include <SPI.h>
#include <Ethernet.h>
#include "FPS_GT511C3.h"
#include "SoftwareSerial.h"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 10);
EthernetServer server(80);
FPS_GT511C3 fps(4, 5);

int rojo = 7;
int amarillo = 8;
int verde = 9;
int solenoide = 6;

void setup() {

  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at");
  Serial.println(Ethernet.localIP());

  fps.UseSerialDebug = true;
  fps.Open();

  pinMode(rojo, OUTPUT);
  pinMode(amarillo, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(solenoide, OUTPUT);
  digitalWrite(solenoide, LOW);
}

void Blink() {
  fps.SetLED(true);
  delay(100);
  fps.SetLED(false);
  delay(100);
}

void huellaNoReconocida() {
  digitalWrite(rojo, HIGH);
  delay(3000);
  digitalWrite(rojo, LOW);
}

void Enroll() {
  fps.SetLED(true);
  int enrollid = 0;
  bool usedid = true;
  while (usedid == true) {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid == true) enrollid++;
  }
  fps.EnrollStart(enrollid);
  Serial.print("Press finger to Enroll #");
  Serial.println(enrollid);

  digitalWrite(amarillo, HIGH);
  Serial.println(enrollid);
  while (fps.IsPressFinger() == false) delay(1000);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;

  if (bret != false) {
    digitalWrite(amarillo, LOW);
    fps.Enroll1();
    while (fps.IsPressFinger() == true) delay(3000);
    digitalWrite(amarillo, HIGH);
    while (fps.IsPressFinger() == false) delay(3000);
    bret = fps.CaptureFinger(true);

    if (bret != false) {
      digitalWrite(amarillo, LOW);
      fps.Enroll2();
      while (fps.IsPressFinger() == true) delay(3000);
      digitalWrite(amarillo, HIGH);
      while (fps.IsPressFinger() == false) delay(3000);
      bret = fps.CaptureFinger(true);
 
      if (bret != false) {
        digitalWrite(amarillo, LOW);
        iret = fps.Enroll3();
        if (iret == 0) {
          digitalWrite(verde, HIGH);
          delay(3000);
          digitalWrite(verde, LOW);
         
        }
        else {
          huellaNoReconocida();

        }
      }
      else {
        huellaNoReconocida();

      }
    }
    else {
      huellaNoReconocida();

    }
  }
  else {
    huellaNoReconocida();

  }
}


void Abrir() {
  fps.SetLED(true);
  if (fps.IsPressFinger())  {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();

    if (id < 200) {

      Serial.print(id);
      digitalWrite(verde, HIGH);
      digitalWrite(solenoide, HIGH);
      delay(5000);
      digitalWrite(verde, LOW);
      digitalWrite(solenoide, LOW);

    }
    else {
      huellaNoReconocida();
    }
  }

  else {
    Blink();
  }

  delay(1000);

}

void borrarPorId() {
  digitalWrite(rojo, HIGH);
  delay(3000);
  digitalWrite(rojo, LOW);
  fps.SetLED(true);
  if (fps.IsPressFinger())  {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();

    if (id < 200) {
      Serial.print(id);
      fps.DeleteID(id);

      digitalWrite(verde, HIGH);
      delay(100);
      digitalWrite(amarillo, HIGH);
      delay(100);
      digitalWrite(rojo, HIGH);
      delay(1000);
      digitalWrite(verde, LOW);
      digitalWrite(amarillo, LOW);
      digitalWrite(rojo, LOW);

    }
    else {
      huellaNoReconocida();

    }
  }

  else {
    Blink();
  }

  delay(3000);

}

void loop() {
  Abrir();
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    String cadena = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (cadena.length() < 50) {
          cadena.concat(c);

          int posicion = cadena.indexOf("Data=");

          if (cadena.substring(posicion) == "Data=1") {
            Enroll();

          }

          else if (cadena.substring(posicion) == "Data=2") {
            borrarPorId();
          }

          else if (cadena.substring(posicion) == "Data=3") {
            fps.DeleteAll();
            digitalWrite(verde, HIGH);
            delay(100);
            digitalWrite(amarillo, HIGH);
            delay(100);
            digitalWrite(rojo, HIGH);
            delay(1000);
            digitalWrite(rojo, LOW);
            delay(100);
            digitalWrite(amarillo, LOW);
            delay(100);
            digitalWrite(verde, LOW);

          }
        }
      

      if (c == 'n' && currentLineIsBlank) {

        // Enviamos al cliente una respuesta HTTP
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();

        //Página web en formato HTML
        client.println("<html>");
        client.println("<head><title>smart Look Door</title>");
        client.println("</head>");
        client.println("<body>");
        client.println("<div style='text-align:center;'>");
        client.println("<h1>SMART LOOK DOOR</h1>");
        client.println("</b></body>");
        client.println("</html>");
        break;
      }
    
      if (c == 'n') {
        currentLineIsBlank = true;
      }
      else if (c != 'r') {
        currentLineIsBlank = false;
      }
    }
  
}
delay(1);
client.stop();// Cierra la conexión


}
}

