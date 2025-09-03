Robotic Dog with Python GUI
Overview

This project is an open-source quadruped robot controlled through a custom Python-based GUI. It’s designed as an affordable and educational platform to explore robotics, electronics, and programming. 
The robot features 12 servo motors for movement, sensors for motion and distance detection, and an onboard display for real-time status updates.
The goal of this project is to provide a hands-on learning experience while offering a fully functional, customizable robot that can inspire beginners and enthusiasts in robotics.



Features
12 servo motors controlling all legs (4 × 20kg, 4 × 25kg, 4 × 35kg)
Sensors: MPU-9250/6500 (accelerometer + gyroscope), 2 × HC-SR04 (ultrasonic distance)
ESP32-WROOM-32U microcontroller with 2.4 GHz antenna and cooler
OLED display 128×64 for real-time feedback
MAX4466 microphone module
Python GUI built with Tkinter for real-time control and custom programming
Open-source and fully documented for learning and modification

Installation & Setup
Flash the ESP32 with the provided firmware.
Connect sensors, servos, and the OLED display.
Install Python 3.x and required libraries:
pip install pyserial pyqt6



Usage
Control the robot in real time using the GUI interface.
Program movement sequences using the simplified scripting language in the GUI.

Learning Outcomes
Understanding of microcontroller-based robotics and servo motor control
Integration of sensors with real-time feedback
Experience with GUI development in Python
Hands-on learning of electronics, networking, and system design

Contributing
This project is open-source and welcomes contributions. 
Feel free to submit bug reports, enhancements, or new features via pull requests.


License
This project is licensed under the MIT License – see the LICENSE file for details.
