          
          
          #include <SoftwareSerial.h>
          /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          #include <LiquidCrystal.h>
          #include <string.h>
          #include <ctype.h>
          #include <stdlib.h>
          #include <time.h>
          
          LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
          SoftwareSerial cell(7, 8); // RX, TX
          int8_t answer;
          int rxPin = 0; // RX pin/////11111111111111111111111111111111
          int txPin = 1; // TX pin
          int byteGPS = -1;
          char cmd[7] = "$GPRMC";
          int counter1 = 0; // counts how many bytes were received (max 300)
          int counter2 = 0; // counts how many comintas were seen
          int offsets[13];
          char val[1] = "A";
          char buf[300] = "";
          char bufclr[300] = "";
          char tbuf[6] = "";
          char labuf[12] = "";
          char lobuf[12] = "";
          char dbuf[6] = "";
          char tbufclr[6] = "";
          char labufclr[12] = "";
          char lobufclr[12] = "";
          char dbufclr[6] = "";
          char thrbuf[3] = "";
          //char tmintbuf[3] = "";
          //char tsecbuf[3] = "";
          //char hrbuf[2] = "";
          //char mintbuf[2] = "";
          //char secbuf[2] = "";
          char bufferfunc[30] = "";
          char bufferfuncclr[30] = "";
          char sendbuf[45] = "";
          char Hedbuf[12] = "";
          char vbuf[4] = "";
          char vbufclr[4] = "";
          //unsigned char
          int d;
          int mon;
          int yr;
          int Time;
          int hr;
          int mint;
          int sec;
          //char timeascii[8];
          int countj = 0;
          unsigned long previousMillis = 0;        // will store last time LED was updated
          
          // constants won't change :
          const long interval = 500;
          
          ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          // software serial #1: RX = digital pin 9, TX = digital pin 10
          SoftwareSerial portOne(9, 10);
          
          // software serial #2: RX = digital pin 8, TX = digital pin 9
          // on the Mega, use other pins instead, since 8 and 9 don't work on the Mega
          //SoftwareSerial portTwo(8, 9);
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          // Represent the date as struct tm.
          // The subtractions are necessary for historical reasons.
          void GMT_TO_IST()
          {
            struct tm  t = { 0 };
            t.tm_mday = d;
            //Serial.print("T.Tm_mday");Serial.print(t.tm_mday);Serial.println("\t");
            t.tm_mon = mon - 1;
            //Serial.print("t.tm_mon");Serial.print(t.tm_mon);Serial.println("\t");
            t.tm_year = yr - 1900;
            //Serial.print("t.tm_year");Serial.print(t.tm_year);Serial.println("\t");
            t.tm_hour = hr;
            t.tm_min = mint;
            t.tm_sec = sec;
          
            // Add 'skip' days to the date.
            // t.tm_mday += skip;
          
            t.tm_min += 30;
            t.tm_hour += 5;
            mktime(&t);
          
            // Print the date in ISO-8601 format.
            //char bufferfunc[30];
            //strftime(bufferfunc, 20, "%Y-%m-%d-%H-%M-%S", &t);
            strftime(bufferfunc, 30, "D:%d-%m-%yT:%H-%M-%S", &t);
            Serial.print("-------bufferfunc-------- \n"); Serial.print(bufferfunc); Serial.println("\t");
            // puts(buffer);
            //sendbuf[0]='E';
            sendbuf[0] = 'E';
            //sendbuf[1]='1';
            //sendbuf[2]='3';
            //sendbuf[3]='1';
          
            sendbuf[19] = bufferfunc[2];
            sendbuf[20] = bufferfunc[3];
            sendbuf[21] = bufferfunc[5];
            sendbuf[22] = bufferfunc[6];
            sendbuf[23] = bufferfunc[8];
            sendbuf[24] = bufferfunc[9];
            sendbuf[25] = bufferfunc[12];
            sendbuf[26] = bufferfunc[13];
            sendbuf[27] = bufferfunc[15];
            sendbuf[28] = bufferfunc[16];
            sendbuf[29] = bufferfunc[18];
            sendbuf[30] = bufferfunc[19];
          
          
          
          }
          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          void setup()
          {
            unsigned long previousMillis = 0;
            const long interval = 500;
            //////////////////////////////////////////////////////////////////
            pinMode(rxPin, INPUT);
            pinMode(txPin, OUTPUT);
            Serial.begin(9600);
            cell.begin(9600);
            lcd.begin(16, 2);///////////////11111111111111111111111111111
            lcd.clear();
            lcd.setCursor(5, 0);
            lcd.print("ARTEZ");
            lcd.setCursor(0, 1);
            lcd.print("waiting for gps");
            delay(2500);
            offsets[0] = 0;
            reset();
            while ( sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 1000) == 0 );
     delay(3000);
     Serial.println("Connecting to the network...");
     lcd.setCursor(0,0);
     lcd.print("Connecting to network...");
          
      Setup_TCPIP();
            ///////////////////////////////////////////////////////////////////////////
            // Open serial comintunications and wait for port to open:
            //Serial.begin(9600);
            while (!Serial)
            {
              ; // wait for serial port to connect. Needed for native USB port only
            }
            // Serial.println("starting....");
          
            // Start each software serial port
            portOne.begin(9600);
            //portTwo.begin(9600);
          }
          ///////////////////////////////////////////////////////////////////
          void reset()
          {
            counter1 = 0;//////////////111111111111111111111111111111111
            counter2 = 0;
          }
          ///////////////////////////////////////////////////////////////////////
          int get_size(int offset)
          {
            return offsets[offset + 1] - offsets[offset] - 1;
          }
          
          int handle_byte(int byteGPS)
          {
            buf[counter1] = byteGPS;
            //Serial.print("byteGPS");
            // Serial.print((char)byteGPS);//nidhin
            counter1++;
            if (counter1 == 300)
            {
              return 0;
            }
          
            if (byteGPS == ',')
            {
              counter2++;
              offsets[counter2] = counter1;
          
              if (counter2 == 13)
              {
                return 0;
              }
            }
            if (byteGPS == '*')
            {
              offsets[12] = counter1;
            }
          
            // Check if we got a <LF>, which indicates the end of line
            if (byteGPS == 10)
            {
              // Check that we got 12 pieces, and that the first piece is 6 characters
              if (counter2 != 12 || (get_size(0) != 6))
              {
                return 0;
              }
          
              // Check that we received $GPRMC
              for (int j = 0; j < 6; j++)
              {
                //Serial.print("GPRMC CHECKING ");
          
                if (buf[j] = cmd[j])
                {
                  // lcd.setCursor(5, 0);
                  //lcd.print(" ARTEZ ");
                  countj = countj + 1;
                }
              }
              if (countj == 6)
              {
                countj = 0;
                if (buf[18] == val[0])
                {
              //...........................................................................
                 
          
                  thrbuf[0] = buf[7];
                  thrbuf[1] = buf[8];
                  hr = atoi(thrbuf);
                  //Serial.print("hr====");  Serial.print(hr);Serial.print(" : ");
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
                  thrbuf[0] = buf[9];////////////////////////////////////time minut extraction to a temporary buffer
                  thrbuf[1] = buf[10];
          
                  mint = atoi(thrbuf);
                  //Serial.print("mint====");  Serial.println(mint); Serial.print(" : ");
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
                  thrbuf[0] = buf[11];////////////////////////////////////time second extraction to a temporary buffer
                  thrbuf[1] = buf[12];
                  sec = atoi(thrbuf);
                  // Serial.print("sec====");  Serial.println(sec);Serial.println("    ");
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
          
          
          
                  for (int a = 0; a < 30; a++) ////////////////////////////////////clearing temporary buffer of time
                  {
                    bufferfunc[a] = bufferfuncclr[a];
          
                  }
                  //Serial.print("bufferfuncclr");Serial.print(bufferfunc);Serial.println("\t");
          
                  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          
                  for (int b = 0; b < 11; b++)
                  {
                    labuf[b] = buf[b + 20];//////////////////////////////////lattitude extraction to atemporary buffer
          
                  }
                  //                lcd.setCursor(0, 1);
                  //                lcd.print("latt:");
                  //                lcd.setCursor(5, 1);
                  //                lcd.print(labuf);
                  Serial.print("lat== ");
                  Serial.print(labuf);
                  Serial.println("\t");
                  lcd.setCursor(0, 0);
                  lcd.print("lat:");
                  lcd.setCursor(4, 0);
                  lcd.print(labuf[0]);
                  lcd.setCursor(5, 0);
                  lcd.print(labuf[1]);
                  lcd.setCursor(6, 0);
                  lcd.print(labuf[2]);
                  lcd.setCursor(7, 0);
                  lcd.print(labuf[3]);
                  lcd.setCursor(8, 0);
                  lcd.print(labuf[4]);
                  lcd.setCursor(9, 0);                // problem when printing the the lobuf
                  lcd.print(labuf[5]);
                  lcd.setCursor(10, 0);                // each array element is printed one by one
                  lcd.print(labuf[6]);
                  lcd.setCursor(11, 0);
                  lcd.print(labuf[7]);
                  lcd.setCursor(12, 0);
                  lcd.print(labuf[8]);
                  lcd.setCursor(13, 0);
                  lcd.print(labuf[9]);
                  lcd.setCursor(14, 0);
                  lcd.print(labuf[10]);
                  // lcd.setCursor(15, 0);
                  //lcd.print(labuf[11]);
          
          
          
          
                  // Serial.println(labuf);//.............................................................................................................................
          
                  for (int k = 0; k <= 3; k++) {
                    sendbuf[k + 2] = labuf[k];
                    //                sendbuf[2]=labuf[0];
                    //                sendbuf[3]=labuf[1];
                    //                sendbuf[4]=labuf[2];
                    //                sendbuf[5]=labuf[3];
                  }
                  for (int k = 5; k <= 8; k++) {
                    sendbuf[k + 1] = labuf[k];
                  }
                  //                sendbuf[6]=labuf[5];
                  //                sendbuf[7]=labuf[6];
                  //                sendbuf[8]=labuf[7];
                  //                sendbuf[9]=labuf[8];
          
                  for (int b = 0; b < 11; b++)/////////////////////////////////clearing temporary buffer of lattitude
                  {
                    labuf[b] = labufclr[b];
          
                  }
          
          
                  for (int b = 0; b < 10; b++)
                  {
                    Hedbuf[b] = buf[b + 30];//////////////////////////////////lattitude extraction to atemporary buffer
          
                  }
                  sendbuf[31] = Hedbuf[0];
          
          
                  //              Serial.print("lat== ");
                  //              Serial.println(labuf);
                  //              Serial.print("\t");
                  //
                  for (int b = 0; b < 11; b++)/////////////////////////////////clearing temporary buffer of lattitude
                  {
                    Hedbuf[b] = labufclr[b];
          
                  }
          
                  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                  for (int c = 0; c < 12; c++)
                  {
                    lobuf[c] = buf[c + 32];/////////////////////////////////longitude extraction to a temporarry buffer
          
                  }
                  Serial.print("lon== ");
                  Serial.print(lobuf);
                  Serial.println("\t");
                  lcd.setCursor(0, 2);
                  lcd.print("lon:");
                  lcd.setCursor(4, 2);
                  lcd.print(lobuf[0]);
                  lcd.setCursor(5, 2);
                  lcd.print(lobuf[1]);
                  lcd.setCursor(6, 2);
                  lcd.print(lobuf[2]);
                  lcd.setCursor(7, 2);
                  lcd.print(lobuf[3]);
                  lcd.setCursor(8, 2);
                  lcd.print(lobuf[4]);
                  lcd.setCursor(9, 2);                // problem when printing the the lobuf
                  lcd.print(lobuf[5]);
                  lcd.setCursor(10, 2);                // each array element is printed one by one
                  lcd.print(lobuf[6]);
                  lcd.setCursor(11, 2);
                  lcd.print(lobuf[7]);
                  lcd.setCursor(12, 2);
                  lcd.print(lobuf[8]);
                  lcd.setCursor(13, 2);
                  lcd.print(lobuf[9]);
                  lcd.setCursor(14, 2);
                  lcd.print(lobuf[10]);
                  lcd.setCursor(15, 2);
                  lcd.print(lobuf[11]);//.................................................................................................................
          
                  delay(1500);
          
                  sendbuf[10] = lobuf[0];
                  sendbuf[11] = lobuf[1];
                  sendbuf[12] = lobuf[2];
                  sendbuf[13] = lobuf[3];
                  sendbuf[14] = lobuf[4];
          
                  sendbuf[15] = lobuf[6];
                  sendbuf[16] = lobuf[7];
                  sendbuf[17] = lobuf[8];
                  sendbuf[18] = lobuf[9];
          
          
          
          
                  //                Serial.print("lon== ");
                  //                Serial.print(lobuf[0]);
                  //                Serial.print(lobuf[1]);
                  //                Serial.print(lobuf[2]);
                  //                Serial.print(lobuf[3]);
                  //                Serial.print(lobuf[4]);
                  //                Serial.print(lobuf[5]);
                  //                Serial.print(lobuf[6]);
                  //                Serial.print(lobuf[7]);
                  //                Serial.print(lobuf[8]);
                  //                Serial.print(lobuf[9]);
                  //                Serial.print(lobuf[10]);
                  //                Serial.print(lobuf[11]);
          
                  for (int c = 0; c < 12; c++)//////////////////////////////clearing the longitude buffer
                  {
                    lobuf[c] = lobufclr[c];
          
                  }
          
                  for (int c = 0; c < 4; c++)
                  {
                    vbuf[c] = buf[c + 45];/////////////////////////////////longitude extraction to a temporarry buffer
          
                  }
          
                  Serial.print("velo==");
                  Serial.println(vbuf);
                  lcd.setCursor(0, 2);
                  lcd.print("Vel:");
                  lcd.setCursor(5, 2);
                  lcd.print(vbuf[0]);
                  lcd.setCursor(6, 2);
                  lcd.print(vbuf[1]);
                  lcd.setCursor(7, 2);
                  lcd.print(vbuf[2]);
                  lcd.setCursor(8, 2);
                  lcd.print(vbuf[3]);
                  lcd.setCursor(9, 2);
                  lcd.print(" ");
                  lcd.setCursor(9, 2);                // problem when printing the the lobuf
                  lcd.print(" ");
                  lcd.setCursor(10, 2);                // each array element is printed one by one
                  lcd.print(" ");
                  lcd.setCursor(11, 2);
                  lcd.print(" ");
                  lcd.setCursor(12, 2);
                  lcd.print(" ");
                  lcd.setCursor(13, 2);
                  lcd.print(" ");
                  lcd.setCursor(14, 2);
                  lcd.print(" ");
                  lcd.setCursor(15, 2);
                  lcd.print(" ");


                  
                  sendbuf[32] = vbuf[0];
                  sendbuf[33] = vbuf[1];
                  sendbuf[34] = vbuf[2];
                  sendbuf[35] = vbuf[4];



          
                  for (int c = 0; c < 4; c++)
                  {
                    vbuf[c] = vbufclr[c];/////////////////////////////////longitude extraction to a temporarry buffer
          
                  }
          
                  thrbuf[0] = buf[57];
                  thrbuf[1] = buf[58];
                  d = atoi(thrbuf);
                  //Serial.print("d====");  Serial.println(d);
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
                  thrbuf[0] = buf[59];////////////////////////////////////time minut extraction to a temporary buffer
                  thrbuf[1] = buf[60];
          
                  mon = atoi(thrbuf);
                  //  Serial.print("mon====");  Serial.println(mon);
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
                  thrbuf[0] = buf[61];////////////////////////////////////time second extraction to a temporary buffer
                  thrbuf[1] = buf[62];
                  yr = atoi(thrbuf);
                  //  Serial.print("yr====");  Serial.println(yr);
                  yr = yr + 2000;
                  //Serial.print("yr====");  Serial.println(yr);
                  for (int a = 0; a < 2; a++)////////////////////////////////////clearing temporary buffer of time
                  {
                    thrbuf[a] = tbufclr[a];
          
                  }
          
                  GMT_TO_IST();
                  // lcd.setCursor(0, 1); //commented on testing the lcd
                  // lcd.print(bufferfunc);//............................................................................................................
          
                  Serial.println("");
                  Serial.println("##############################################################");
                  Serial.println("");
                  // Serial.print(bufferfunc);Serial.println("\t");
                  //Serial.print("checking");
          
                  
                  sendDATA(sendbuf);
                    Serial.println(sendbuf);
            
                }
          
                else
                {
          
                }
              }
          
             
          
  
          
          
          
              ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          
          
          
              //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          
              // Check that time is well formed
              if (get_size(1) != 10)
              {
                return 0;
              }
          
              
              if (get_size(9) != 6)
              {
                return 0;
              }
             
          
              return 0;
          
            }
          
            else
            {
              
            }
            return 1;
          }
          ////////////////////////////////////////////////////////////////////////////////
          
          void sendDATA(String data) 
          {
     //Serial.println(data);
         int data_len = data.length() + 1;
       char char_array[data_len];
           data.toCharArray(char_array, data_len);
          
        cell.println("AT+CIPSEND");
         delay(100);
         cell.println(char_array);
         delay(300);
         cell.write(0x1A);
          if(cell.read()=="ERROR")
          {
            Setup_TCPIP();
          }
          
          
           // sprintf(char_array,"AT+CIPSEND=%d", strlen(data));
//         if (sendATcommand2("AT+CIPSEND", ">", "ERROR", 10000) == 1)
//          {
//          
//              sendATcommand2(char_array + (char)26, "SEND OK", "ERROR", 10000);
//          //sendATcommand2((char)26, "SEND OK", "ERROR", 1000);
//            
//            }
//     else
//             {
//             Serial.println("Error sending");
//            sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
//             Setup_TCPIP();
              
      //        }
            }
          /////////////////////////////////////////////////////////////////////////////
          
          void Setup_TCPIP() {
        // Selects Single-connection mode
  if (sendATcommand2("AT+CIPMUX=0", "OK", "ERROR", 1000) == 1)
    {
      // Waits for status IP INITIAL
              while (sendATcommand2("AT+CIPSTATUS", "INITIAL", "", 500)==0 );
    delay(5000);
// Sets the APN, user name and password
             if (sendATcommand2("AT+CSTT=\"TATA.DOCOMO.INTERNET\",\"\",\"\"", "OK", "ERROR", 30000) == 1)
               {
// Waits for status IP START
   while (sendATcommand2("AT+CIPSTATUS", "START", "", 500)==0 );
                delay(5000);
          
                // Brings Up Wireless Connection
                if (sendATcommand2("AT+CIICR", "OK", "ERROR", 30000) == 1)
                  {
                  // Waits for status IP GPRSACT
       while (sendATcommand2("AT+CIPSTATUS", "GPRSACT", "", 500) == 0 );
         delay(5000);
          
                  // Gets Local IP Address
          if (sendATcommand2("AT+CIFSR", ".", "ERROR", 10000) == 1)
                  {
                    // Waits for status IP STATUS
         while (sendATcommand2("AT+CIPSTATUS", "IP STATUS", "", 500)== 0 );
            delay(5000);
            Serial.println("Openning TCP");
                                                                
           // Opens a TCP socket
             if (sendATcommand2("AT+CIPSTART=\"tcp\",\"117.199.11.2\",\"400\"", "CONNECT OK", "CONNECT FAIL", 30000) == 1)
                      {
                      while (sendATcommand2("AT+CIPSTATUS", "IP STATUS", "", 500) == 0 );
                                  Serial.println("Connected");
                    }
                                    else
                                    {
                                Serial.println("Error openning the connection");
                                   sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
            }
               }
                     else
                  {
                  Serial.println("Error getting the IP address");
               sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
             }

              }
              else
           {
                              Serial.println("Error bring up wireless connection");
                            Setup_TCPIP();
                   
          }

         }
              else
             {
                Serial.println("Error setting the APN");
             sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
           Setup_TCPIP();
            
                 
          }

               }
                 else
                     {
                Serial.println("Error setting the single connection");
                        sendATcommand2("AT+CIPSHUT", "OK", "ERROR", 10000);
                        Setup_TCPIP();
                 
          
              }
          
            }
          
          ///////////////////////////////////////////////////////////////
          
          int8_t sendATcommand2(char* ATcommand, char* expected_answer1, char* expected_answer2, unsigned int timeout) {
                //show();
          
             uint8_t x = 0, answer = 0;
             char response[100];
             unsigned long previous;
          
             memset(response, "" , 100);// Initialize the string
          
             delay(100);
          while ( cell.available() > 0) cell.read();// Clean the input buffer
             //cell.flush();
          
             cell.println(ATcommand);
                    Serial.println(ATcommand); // Send the AT command
          
          
             x = 0;
               previous = millis();
          
                // this loop waits for the answer
                 do {
                  // if there are data in the UART input buffer, reads it and checks for the asnwer
                    if (cell.available() != 0) {
                       response[x] = cell.read();
                        x++;
          
                        if (strstr(response, expected_answer1) != NULL)
                            {
                      answer = 1;
                }
                // check if the desired answer 2 is in the response of the module
                else if (strstr(response, expected_answer2) != NULL)
                {
                  answer = 2;
                }
              }
            }
          
            // Waits for the asnwer with time out
            while ((answer == 0) && ((millis() - previous) < timeout));
            //Serial.println(response);
            if(answer ==1 || answer ==2){
              Serial.println("OK");
            }
            else
            {
              Serial.println("Error");
            }
          
            return answer;
          }
          
          /////////////////////////////////////////////////////////////
          
          
          void loop()
          {
            unsigned long currentMillis = millis();
            //portOne.listen();
            byteGPS = portOne.read();        // Read a byte of the serial port
            //byteGPS = Serial.read();
            //Serial.println(byteGPS);
            if (byteGPS == -1)
            { // See if the port is empty yet
              delay(100);
            }
            else if (!handle_byte(byteGPS))
            {
              reset();
          
            }
          
            // By default, the last intialized port is listening.
            // when you want to listen on a port, explicitly select it:
            // Serial.println("Data from port one:");
            // while there is data comintg in, read it
            // and send to the hardware serial port:
            //while (portOne.available() > 0) {
            // char inByte = portOne.read();
            //Serial.write(inByte);
            //
            //}
            if (currentMillis - previousMillis >= interval)
            {
              sendbuf[1] = '2';
              // save the last time you blinked the LED
              previousMillis = currentMillis;
              //Serial.println("sendbuf==");
              // Serial.println(sendbuf);
            }
            else
            {
              sendbuf[1] = '1';
              // Serial.println("sendbuf");
            }
          }










