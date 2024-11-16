import machine
import dht
import urequests
import utime
from twilio.rest import Client
import ADC

# WiFi credentials
SSID = 'Chivi'
PASSWORD = 'Pass4123'

# Twilio credentials


# Nominatim API URL
NOMINATIM_API_URL = "https://nominatim.openstreetmap.org/reverse?format=json&lat="

# IP Info API URL
IP_INFO_URL = 'http://ipinfo.io/json'

# Sensor pins
DHT_PIN = 4
MOISTURE_SENSOR_PIN = 35
PIR_PIN = 12
MQ7_PIN = 32
LED_PIN = 13

# Initialize sensors
dht_sensor = dht.DHT11(machine.Pin(DHT_PIN))
moisture_sensor = ADC(machine.Pin(MOISTURE_SENSOR_PIN))
pir_sensor = machine.Pin(PIR_PIN, machine.Pin.IN)
mq7_sensor = ADC(machine.Pin(MQ7_PIN))
led = machine.Pin(LED_PIN, machine.Pin.OUT)

def connect_wifi():
    wlan = machine.WLAN(machine.WLAN_STA)
    wlan.active(True)
    wlan.connect(SSID, PASSWORD)
    while not wlan.isconnected():
        utime.sleep(1)
    print('Connected to WiFi')

def send_twilio_notification(message):
    client = Client(ACCOUNT_SID, AUTH_TOKEN)
    message = client.messages.create(
        body=message,
        from_=FROM_NUMBER,
        to=TO_NUMBER
    )
    print('Notification sent')

def get_location():
    response = urequests.get(IP_INFO_URL)
    data = response.json()
    latitude = data['loc'].split(',')[0]
    longitude = data['loc'].split(',')[1]
    location_url = f"{NOMINATIM_API_URL}{latitude}&lon={longitude}"
    response = urequests.get(location_url)
    location_data = response.json()
    address = location_data['display_name']
    return address

def read_dht():
    temperature = dht_sensor.temperature()
    humidity = dht_sensor.humidity()
    return temperature, humidity

def read_moisture():
    return moisture_sensor.read()

def read_pir():
    return pir_sensor.value()

def read_mq7():
    return mq7_sensor.read()

def read_sensors():
    temperature, humidity = read_dht()
    moisture_level = read_moisture()
    pir_state = read_pir()
    mq7_value = read_mq7()
    return moisture_level, temperature, humidity, mq7_value, pir_state

connect_wifi()

while True:
    moisture_level, temperature, humidity, mq7_value, pir_state = read_sensors()
    print(f"Moisture: {moisture_level} Temperature: {temperature}C Humidity: {humidity}% MQ7: {mq7_value} PIR: {pir_state}")
    
    if pir_state == 1:
        led.value(1)
        location = get_location()
        message = f"Person detected at {location}!"
        send_twilio_notification(message)
    else:
        led.value(0)
    
    utime.sleep(15)