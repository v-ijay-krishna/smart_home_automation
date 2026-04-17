# Smart Home Automation 🏠🔐

A high-performance, **ESP8266-based** smart home automation system featuring a neon-themed "Cyber-Sec" web dashboard. This system integrates hardware sensors with a secure web interface for real-time monitoring and control.

## 🚀 Features
- **Secure Authentication:** Password-protected "Cyber-Sec" login portal.
- **Dynamic Dashboard:** Real-time UI updates for temperature, light status, and door security.
- **Automated Security:** Ultrasonic-based intruder detection with buzzer alerts.
- **Environment Control:** - PWM-controlled multi-speed fan logic.
  - Servo-controlled door locking mechanism.
  - Temperature monitoring via LM35.

## 🛠️ Hardware Stack
- **Microcontroller:** ESP8266 (NodeMCU/Wemos D1 Mini)
- **Sensors:** HC-SR04 (Ultrasonic), LM35 (Temperature)
- **Actuators:** MG90S Servo (Door), DC Fan (via Transistor/PWM), 5V Buzzer, LED/Relay (Lighting)

## 📌 Pin Mapping
| Component | Pin | Function |
| :--- | :--- | :--- |
| Light | D0 | Lighting Control |
| Temperature | A0 | LM35 Analog Input |
| Buzzer | D4 | Alert System |
| Ultrasonic | D5 (Echo), D6 (Trig) | Intruder Detection |
| Fan | D7 | PWM Speed Control |
| Servo | D8 | Door Lock Mechanism |

## 💻 Software & Libraries
- **Language:** C++ (Arduino Framework)
- **Frontend:** HTML5, CSS3 (Neon-Cyber Theme), JavaScript (Fetch API)
- **Libraries:** - `ESP8266WiFi.h` & `ESP8266WebServer.h`
  - `Servo.h`

## ⚙️ Setup Instructions
1.  **Clone the repo:** `git clone https://github.com/YOUR_USERNAME/Smart-Home-CyberSec.git`
2.  **Configuration:** Update `AP_SSID` and `AP_PASSWORD` in the `.ino` file if necessary.
3.  **Upload:** Use the Arduino IDE to upload the code to your ESP8266.
4.  **Connect:** Connect to the WiFi Access Point: **TouchlessBell_Cyber**.
5.  **Access:** Open `192.168.4.1` in your browser to enter the Cyber-Sec portal.


Below are the interface designs and hardware photos showcasing and the physical integration of the sensors.
<img width="1206" height="724" alt="image" src="https://github.com/user-attachments/assets/66d97d33-7f53-48bd-8832-6c212e819232" />


<img width="344" height="248" alt="image" src="https://github.com/user-attachments/assets/29593f05-ad4f-471b-a05a-d68eab3428ff" />

---
*Developed as part of an IoT Security & Automation exploration.*
