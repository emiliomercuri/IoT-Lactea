# Build Real-Time IoT Dashboard: Node-RED + InfluxDB + Grafana + MQTT Tutorial (MING)

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

## Node Red Install
Para baixar o Node Red, usar o seguinte código:
```
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)
```

Redefinir o espaço do Node Red com o código:
```
node-red-pi --max-old-space-size=256
```

Para iniciar, parar ou reiniciar o Node Red, usar:
```
	node-red-start
	node-red-stop
	node-red-restart
```
Ou:
```
sudo systemctl enable nodered.service
sudo systemctl disable nodered.service
```

## InfluxDB Install
Para baixar o Influx, executar os seguintes códigos:
```
curl https://repos.influxdata.com/influxdata-archive.key | gpg --dearmor | sudo tee /usr/share/keyrings/influ
xdb-archive-keyring.gpg >/dev/null
```
E
```
echo "deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main" | sudo tee /etc/apt/sources.list.d/influxdb.list
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

## Acessando os ambientes do Node Red, InfluxDB e Grafana
Para acessar os ambientes, abra um navegador e digite o número do IP da conexão do Raspberry Pi. Você pode encontrar o número do IP utilizando o código abaixo:
```
ifconfig
```

Posterior mente, copie o código do IP e cole na aba de navegação do navegador, acrescente ":" e o número das portas de acesso para cada ambiente. Segue exemplos:

> "ipNumber":1880	; para o Node Red
>
> "ipNumber":8086	; para o Influx
>
> "ipNumber":3000	; para o Grafana
>

## Python Install
