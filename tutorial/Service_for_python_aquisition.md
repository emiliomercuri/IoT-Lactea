# Como criar um serviço Linux que roda um código Python em background

Este tutorial foi criado com base no video do YouTube:
https://www.youtube.com/watch?v=0mluq0oJk7o&t=373s&ab_channel=IoTFrontier

Configurar o Raspberry Pi 3 ou 4 com o sistema operacional Raspberry Pi OS (64bits).

## MQTT Install
Para instalar o Mosquitto, basta rodar os seguintes comandos no prompt de comando do Raspberry Pi:

```bash
sudo apt update && sudo apt upgrade
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto.service
```

Para verificar o status do Mosquitto:
```
mosquitto -v
```
Onde deve ser exibido o seguinte erro:

> Opening ipv4 listen socket on port 1883.
> 
> Error: Address already in use
> 
> Opening ipv6 listen socket on port 1883.
> 
> Error: Address already in use

Ou
```
sudo systemctl status mosquitto
```
One deve ser exibido:
> Loaded: *loaded (... ; *enabled; preset: *enabled)
> 
> Active: *active (running)

Para configurar a porta de comunicação do Mosquito, deve ser editado o arquivo "mosquitto.conf", utilizando o código:
```
sudo nano /etc/mosquitto/mosquitto.conf
```
E acrescentar ao final do arquivo de texto as seguintes linhas:
> listener 1883
> 
> allow_anonymous true

Por fim, reiniciar o Mosquito com o código:
```
sudo systemctl restart mosquitto
```


deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main
```

Atuzalizar os pacotes com:
```
sudo apt update
```

Instalar o Influx, execute:
```
sudo apt install influxdb2
```

Para habilitar e inicar o Influx, execute:
```
sudo systemctl unmask influxdb
sudo systemctl enable influxdb
sudo systemctl start influxdb
```

## Grafana Install
Para baixar o Grafana, execute os seguintes códigos:
```
curl https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /usr/share/keyrings/grafana-archive-keyrings.gpg >/dev/null
```
E
```
echo "deb [signed-by=/usr/share/keyrings/grafana-archive-keyrings.gpg] https://apt.grafana.com stable main" | sudo tee /etc/apt/sources.list.d/grafana.list
```

Atuzalizar os pacotes com:
```
sudo apt update
```

Para instalar o Grafana, execute:
```
sudo apt install grafana
```

Para habilitar e iniciar o Grafana, execute:
```
sudo systemctl enable grafana-server
sudo systemctl start grafana-server
```

## Python Install
Para instalar o Python, execute os seguintes comandos:
```
sudo apt install python3 python3-pip python3-venv
```
Será necessário criar um ambiente Python, mas antes de cria-lo, utilize os seguintes comandos:
```
sudo apt install libgpiod2
sudo apt-get install python3-rpi.gpio
```
Criando um ambiente Python:
```
mkdir ~/dht11
cd ~/dht11
python3 -m venv env
```
Acessando o ambiente Python:
```
source env/bin/activate
```
No ambiente Python, instalar as seguintes bibliotecas:
```
pip3 install adafruit-circuitpython-dht
pip3 install adafruit-circuitpython-minimqtt
pip3 install RPI.GPIO adafruit-blinka
```
Ou
```
python3 -m pip install adafruit-circuitpython-dhtpaho-mqtt
```
![teste](C:\Users\Daniel\Desktop\aa.png)

