# Aruino project using proteus to simulate a Smarthome and control it through mobile app!

I use Proteus to create a door with padlock, door light, fan with IR Sensor, Temperature Sensor and Vibration Sensor to detect earthquake.

I use Blynk to create a fast application that connect through the blynk cloud and control the SmartHome.

It requires some application to connect Blynk app to Proteus.

# Prepare the application

 ## 1. Proteus

 Have Proteus installed on your device. (Recommend Proteus 8 with full arduino libraries.)

 Arduino libraries can be found on TheEngineeringProjects website.
 ## 2. Blynk

 + Create an account, make a device and prepare Digital Pin Datastream to pin 13 (Light) and 11 (Fan).
 + Create Double Virtual Pin Datastream for get Temperature and Humidity on Proteus
 + Create button for both of digitalpin and two display for temperature and humidity.

Can control on mobile Blynk app.

# Connection

## 1. Download Virtual Serial Port Driver

 After download and install, create a "Pair Connection" from COM3 to COM1. Make sure this stay opened.

## 2. Run blynk cloud server

Go to this directory:

```C:\Users\{name}\Documents\Arduino\libraries\Blynk\scripts```

Run "cmd" on this folder.

type in ```blynke-ser.bat -c COM1``` to run server and connect to COM1.

If show:

```
Connect("blynk.cloud", "80") - OK
InOut() START
DSR is OFF
```

Then connection are good.

## 3. Run program

 Go to proteus and run simulation. The Virtual Serial Port Driver should show 2 COM Ports are opened. Then feels free to test.
