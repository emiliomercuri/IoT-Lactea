import os
import time
import sys
import adafruit_dht as dht
import paho.mqtt.client as mqtt
import json
import board

HOST = 'localhost'
INTERVAL = 2

sensor_data = {'temperature': 0, 'humidity': 0}
next_reading = time.time()

client = mqtt.Client()

dht_device = dht.DHT11(board.D4)

# Connect to MQTT broker
client.connect(HOST, 1883, 60)
client.loop_start()

try:
    while True:
        try:
            humidity = round(dht_device.humidity, 2)
            temperature = round(dht_device.temperature, 2)
            print(u"Temperature: {:g}\u00b0C, Humidity: {:g}%".format(temperature, humidity))
            sensor_data['temperature'] = temperature
            sensor_data['humidity'] = humidity

            # Publish to MQTT
            client.publish('dht11/sensordata', json.dumps(sensor_data), qos=1)

        except RuntimeError as error:
            # Erros comuns do DHT11 (sem buffer, etc)
            print(f"Erro de leitura do sensor: {error}")
        except Exception as error:
            # Erros mais sérios, encerrar o sensor corretamente
            dht_device.exit()
            print(f"Erro inesperado: {error}")
            raise error

        # Intervalo entre leituras
        next_reading += INTERVAL
        sleep_time = next_reading - time.time()
        if sleep_time > 0:
            time.sleep(sleep_time)

except KeyboardInterrupt:
    pass

client.loop_stop()
client.disconnect()
