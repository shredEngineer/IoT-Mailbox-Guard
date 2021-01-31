#include <ESP8266WiFi.h>

const char *wifi_ssid         = "YOUR-WIFI-NAME";
const char *wifi_pass         = "YOUR-WIFI-PASSWORD";

const char *mail_server       = "mail.smtp2go.com";
const int   mail_port         = 2525;
const char *mail_user_base64  = "YOUR-BASE64-SMTP2GO-USERNAME";
const char *mail_pass_base64  = "YOUR-BASE64-SMTP2GO-PASSWORD";

const char *mail_sender       = "YOUR-MAIL-SENDER";                                   // E-mail address that's shown as sender

const char *mail_recipients[] = { "YOUR-MAIL-RECIPIENT-1", "YOUR-MAIL-RECIPIENT-2" }; // Array of one or more e-mail recipients
const char *mail_subject      = "IoT mailbox guard";
const char *mail_body         = "The mailbox was opened.";

WiFiClient espClient;

void setup() {

  Serial.end();
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.println();

  blink();
  
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  Serial.println();
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < sizeof(mail_recipients) / sizeof(const char *); i++) {
  
    if (sendEmail(mail_recipients[i])) {
      Serial.println();
      Serial.println("E-Mail sent successfully.");
    } else {
      Serial.println();
      Serial.println("Error!");
    }

  }

  Serial.println();
  Serial.println("Pulling RX LOW.");
  Serial.println("Application done.");

  blink();

  // LED ON
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  // RX LOW
  pinMode(3, FUNCTION_3);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

}

void loop() {
}

bool sendEmail(const char *recipient) {

  Serial.println();
  Serial.println("Connecting to mail server");

  if (espClient.connect(mail_server, mail_port) != 1) return false;

  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Sending EHLO");
  espClient.println("EHLO www.example.com");
  if (!emailResp()) return false;

/*
  Serial.println();
  Serial.println("Sending TTLS");
  espClient.println("STARTTLS");
  if (!emailResp()) return false;
*/

  Serial.println();
  Serial.println("Sending AUTH LOGIN");
  espClient.println("AUTH LOGIN");
  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Sending username");
  espClient.println(mail_user_base64);
  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Sending password");
  espClient.println(mail_pass_base64);
  if (!emailResp()) return false;

  Serial.println();
  Serial.println(F("Sending sender"));
  espClient.print("MAIL From: ");
  espClient.println(mail_sender);
  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Sending recipient");
  espClient.print("RCPT To: ");
  espClient.println(recipient);
  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Sending data");
  espClient.println("DATA");
  if (!emailResp()) return false;

  espClient.print("To: ");
  espClient.println(recipient);

  espClient.print("From: ");
  espClient.println(mail_sender);
  
  espClient.print("Subject: ");
  espClient.println(mail_subject);
  
  espClient.println("Content-Transfer-Encoding: 8bit");
  espClient.println("Content-Type: text/html; charset=\"UTF-8\"");

  espClient.println();
  
  espClient.println(mail_body);

  espClient.println(".");
  
  if (!emailResp()) return false;

  Serial.println();
  Serial.println("Ending session");
  espClient.println("QUIT");
  if (!emailResp()) return false;

  espClient.stop();
  return true;

}

bool emailResp() {
  
  byte responseCode;
  byte readByte;
  int loopCount = 0;

  while (!espClient.available()) {
    delay(1);
    loopCount++;
    if (loopCount > 20000) {
      espClient.stop();
      Serial.println("Timeout!");
      return false;
    }
  }

  responseCode = espClient.peek();
  while (espClient.available()) {
    readByte = espClient.read();
    Serial.write(readByte);
  }

  return responseCode < '4';

}

void blink() {

  // LED ON
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  delay(25);

  // LED OFF
  digitalWrite(2, HIGH);
  pinMode(2, INPUT);
  delay(25);
  
}
