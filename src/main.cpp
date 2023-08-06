#include <WiFi.h>

int freq = 50;      // 频率(20ms周期)
int channel = 8;    // 通道(高速通道（0 ~ 7）由80MHz时钟驱动，低速通道（8 ~ 15）由 1MHz 时钟驱动。)
int resolution = 8; // 分辨率
const int led = 14;

int calculatePWM(int degree)
{ // 0-180度
  // 20ms周期，高电平0.5-2.5ms，对应0-180度角度
  const float deadZone = 6.4; // 对应0.5ms（0.5ms/(20ms/256）) 舵机转动角度与占空比的关系：(角度/90+0.5)*1023/20
  const float max = 32;       // 对应2.5ms
  if (degree < 0)
    degree = 0;
  if (degree > 180)
    degree = 180;
  return (int)(((max - deadZone) / 180) * degree + deadZone);
}

// Replace with your network credentials
const char *ssid = "CMCC-DCMR";
const char *password = "3gjjt6ep";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup()
{
  Serial.begin(9600);

  ledcSetup(channel, freq, resolution); // 设置通道
  ledcAttachPin(led, channel);
  // Initialize the output variables as outputs
  // Set outputs to LOW

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  { // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())   // 客户端有数据传送
      {                         // if there's bytes to read from the client,
        char c = client.read(); // 读数据
        Serial.write(c);        // 在串口中打印
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /60") >= 0) // 如果是GET请求，且路径为/26/on
            {

              ledcWrite(channel, calculatePWM(60)); // 输出PWM
              delay(200);
              ledcWrite(channel, calculatePWM(0));
            }
            else if (header.indexOf("GET /90") >= 0)
            {
              ledcWrite(channel, calculatePWM(90)); // 输出PWM
              delay(200);
              ledcWrite(channel, calculatePWM(0));
            }
             else if (header.indexOf("GET /180") >= 0)
            {
                ledcWrite(channel, calculatePWM(180)); // 输出PWM
              delay(200);
              ledcWrite(channel, calculatePWM(0));
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"utf-8\">");
            client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>117</h1>");
            client.println("<p><a href=\"/60\"><button class=\"button\">60度</button></a></p>");
            client.println("<p><a href=\"/90\"><button class=\"button \">90度</button></a></p>");
             client.println("<p><a href=\"/180\"><button class=\"button \">180度</button></a></p>");
            // Display current state, and ON/OFF buttons for GPIO 26

            // If the output26State is off, it displays the ON button
            // if (output26State == "off")
            // {

            //   client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            // }
            // else
            // {
            //   client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            // }

            // Display current state, and ON/OFF buttons for GPIO 27

            // If the output27State is off, it displays the ON button
            // if (output27State == "off")
            // {
            //   client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            // }
            // else
            // {
            //   client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            // }
            // client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}