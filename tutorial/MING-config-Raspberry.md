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
Durante a instalação do Node Red será exigido o cadastro de usuário, o qual será utilizado para acessar o ambiente no navegador. Neste caso foi utilizado as seguintes credenciais:
> Usuário: Pi
> 
> Senha: iotempire

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
curl https://repos.influxdata.com/influxdata-archive.key | gpg --dearmor | sudo tee /usr/share/keyrings/influxdb-archive-keyring.gpg >/dev/null
```
E
```
echo "deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main" | sudo tee /etc/apt/sources.list.d/influxdb.list
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

## Programa Python para coleta dos dados do sensor
Para criar o programa, dentro do ambiente Python, utilize o seguinte código para cria-lo através de um editor de texto:
```
nano mqtt.py
```
E insira o código contido na pasta: [codes/mqtt-dht11.py](https://github.com/emiliomercuri/MING-Lactea/blob/main/codes/mqtt-dht11.py)
Após inserir o código, salvar (CTRL+O para o nano) e fechar (CTRL+X para o nano).
Para executar o programa, digite o seguinte código dentro do ambiente Python:
```
python3 mqtt.py
```
Para encerrar o programa, aperte CRTL+C.

## Acessando os ambientes do Node Red, InfluxDB e Grafana
Para acessar os ambientes, abra um navegador e digite o número do IP da conexão do Raspberry Pi. Você pode encontrar o número do IP utilizando o código abaixo:
```
ifconfig
```

Posterior mente, copie o código do IP e cole na aba de navegação do navegador, acrescente ":" e o número das portas de acesso para cada ambiente. Segue exemplos:

> "ipNumber":1880	; para o Node Red (para fazer o login, utilizar as credenciais cadastradas durante a instalação do Node Red)
>
> "ipNumber":8086	; para o Influx
>
> "ipNumber":3000	; para o Grafana
>


### Configuração do ambiente Node Red
Após acessar o ambiente Node Red, você notará uma barra na lateral esquerda, com um buscador no topo e uma série de paletas para escolher. Encontre a paleta "mqtt in" e arraste-a para o espaço de trabalho.
Feito isso, clique duas vezes sobre a paleta para acessar as configurações. No local acrescente as seguintes informações:
> Servidor: localhost
>
> Tópico: dht11/sensordata
>
Após configurar, clicar em "Feito". Depois: encontrar a paleta "Debug", arrasta-la no ambiente de trabalho e conecta-los arrastando um fio de uma paleta até a outra. Feito isso, clicar em "Implementar"
Caso a configuração ocorra corretamente, no canto superior direito, há o ícone para "mensagens de depuração", clique neste ícone e, enquanto o programa Python estiver em execução, os dados coletados pelo sensor aparecerão no campo abaixo deste ícone.

Para instalar o Influx no ambiente Node Red, clique nas 3 barras do canto superior direito da tela e acesse o menu "Gerenciar paleta". Posteriormente, acesse o menu "Instalar" e digite o nome "influxdb". Encontrar a biblioteca vinculada ao Node Red (geralmente é a segunda na lista de exibição) e selecionar ela para instalar.

Após instalada, encontre a paleta do Influx na barra de paletas da lateral esquerda e selecione a paleta "influxdb out" e arraste-a para o ambiente de trabalho. Clique duas vezes sobre ela para abrir o ambiente de configuração.

No ambiente de configuração do Influx, clique no "+" o qual irá configurar o nó da seguinte forma:
> Mudar o "Version" para 2.0
>
> Host: localhost
>
> Token: colar o token API gerado no Influx (será descrito logo abaixo)
>
Então clicar em "Adicionar". No menu que aparecerá, insira as seguintes informações:
> Organization: coloque o nome da organização cadastrada no primeiro acesso do Influx.
>
> Bucket: o nome da pasta onde os dados serão armazenados no Influx. Neste exemplo, escolheu-se o nome "sensordata".
>
> Measurement: nome da pasta (neste exemplo é a pasta dht11)
>
Clicar em "Feito" e puxar um fio do palete do MQTT até o palete do Influx e clicar em "Implementar". A organização do fluxo deve ficar semelhante a figura a baixo.

![images/Node Red_scheme.PNG](https://github.com/emiliomercuri/MING-Lactea/blob/00eb69bf35b5b9a532b15ab3e25eab460b39a6af/images/Node%20Red_scheme.PNG "Exemplo de um flow em Node Red")

### Configuração do ambiente Influx
Credenciais de entrada:
> Usuário: Lactea
> 
> Senha: iotempire

Após cadastrar os dados de usuário, vá em "Inicialização rápida" e o Influx pedirá para você nomear um Bucket para receber os dados. Neste exemplo, escolheu-se o nome "sensordata". Feito isso, acessar "API Token", clicar em "Generate API Token", ir na opção "All Access API Token", nomear o Token e então copiar o código exibido. Este código deverá ser inserido no campo "Token" durante a configuração do Influx no ambiente do Node Red.
Dica: Gerar outro Token para o Grafana e utilizar os nomes de cada plataforma para nomear os Tokens para fins de organização.

Após o termino da configuração do Node Red e vinculando-o com o Influx, vá para o Bucket criado para o armazenamento dos dados, neste caso é o Bucket "sensordata". Clique neste Bucket e lá você deverá encontrar a pasta "dht11" com as variáveis de coleta do sensor, neste caso: Temperatura e Umidade. Selecione a variável desejada (podem ser selecionadas ambas as variáveis) e clique em "SUBMIT". Caso a configuração esteja certa, o Influx exibirá um gráfico com a variável selecionada.

Neste mesmo ambiente, para a varável selecionada, clique em "SCRIPT EDITOR". Aparecerá um script logo abaixo do gráfico. Este scrip será utilizado para configurar o Grafana. Guarde-o!

### Configuração do ambiente Grafana
Para acessar o ambiente Grafana, utilize as credenciais:
> Login: admin
>
> senha: admin
> 
> senha redefinida: iotempire
> 
Após entrar, o Grafana exigirá que você escolha uma nova senha. Feito isso, você será redirecionado para a página principal, onde deve selecionar "Data Source" e nas opções exibidas, selecionar "InfluxDB". No formulário que aparecerá, você deve cadastrar os dados sugeridos:
> Query Lenguage: selecionar "Flux"
>
> HTTP -> URL: http://"ipNumber":8086
>
> InfluxDB Details: preencher os ítens "Organization", "Token" e "Default Bucket" com os mesmo dados cadastrado no Influx e deve ser utilizado o Token gerado no API Token para o Grafana.
>
Feito isso, clicar em "Save & test".

Então, vá para "Dashboards", depois clique em "Create dashboard", acesse "Add visualization" e em "Select data source" escolha Influxdb. Caso ocorra tudo certo, no ambiente em que será exibido, haverá um script, logo abaixo do espaço onde será exibido o gráfico. Neste local, cole o script copiado do Influx. A ultima linha do script: "|>.yield(name: "mean")", pode ser excluida.
Após vá para "Query inspector" e clique em "Refresh". No ambiente que aparecerá, você poderá detalhar o nome do gráfico e outras descrições. Feito isso, clique em "Save dashboard", adicione um Título e clique em "Save".

Agora, no menu "Dashboards", no ítem "Add" e depois "Visualization" é possível criar outros gráficos.



