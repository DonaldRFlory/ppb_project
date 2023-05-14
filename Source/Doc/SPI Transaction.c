            digitalWrite(ADC_TELL_TALE, HIGH);
            // Display current state, and ON/OFF buttons for GPIO 26
            // If the output26State is off, it displays the ON button
            //client.println("<p>GPIO 26 - State " + output26State + "</p>");
            // If the output26State is off, it displays the ON button
            if (output26State=="off")
            {
              client.println("<p>GPIO 26 - State " + output26State + "</p>\r\n<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p>GPIO 26 - State " + output26State + "</p>/r/n<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            digitalWrite(ADC_TELL_TALE, LOW);
