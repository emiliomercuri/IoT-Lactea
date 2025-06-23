# Tutorial para aquição dos dados do sensor DHT11 no Raspberry Pi e criação de um dashboard no Grafana

Nesse tutorial será mostrado como fazer a aquisição dos dados de temperatura do ar e umidade relativa do ar usando o sensor DHT11 conectado no microcomputador Raspberry Pi 3B+.

---

**Autor:** Daniel Antonio Gomes de Souza

**Supervisor:** Emílio Graciliano Ferreira Mercuri

---

As etapas são:

## 1. Conexões Físicas:

Imagem e diagrama do Fritzing

## 2. Instalação do sistema operacional 

Num cartão de memória instalar o sistema operacional Raspberry Pi OS (64-bit) - Debian Linux.

Configurar:
- Usuário: lactea
- Senha: iot******r

## 3. Conexão remota

Raspberry Connect e Netbird

## 4. Instalação do MQTT, Influxdb, Node-Red e Grafana (MING Stack)

* Explicar brevemente o que é o MING Stack e seu princípio de funcionamento

Antes de iniciar a instalação dos softwares, precisamos atualizar todos os pacotes instalados no seu sistema para as versões mais recentes disponíveis nos repositórios. Para isso, execute no terminal:

```bash
sudo apt update && sudo apt upgrade
```

Digite "y" para confirmar a instalação, que poderá demorar alguns instantes.

### Instalação do MQTT

Uma vez que seu sistema foi atualizado, podemos instalar o Mosquitto MQTT Broker, que é um servidor que gerencia mensagens usando o protocolo MQTT (Message Queuing Telemetry Transport), através do seguinte comando:

```bash
sudo apt install -y mosquitto mosquitto-clients
```

Uma vez instalado, precisamos configurar o Mosquitto para iniciar automaticamente toda vez que o sistema for ligado, rodando no Prompt:

```bash
sudo systemctl enable mosquitto.service
```

Agora, para checar o status do Mosquitto, rode:

```bash
mosquitto -v
```

O que deverá retornar o seguinte:

![5](https://github.com/user-attachments/assets/20fb2900-38a6-46a5-9284-cae5df662150)

A mensagem “Error: Address already in use” significa que o software está funcionando. Agora, para configurar a porta de comunicação do Mosquito como sendo a porta 1883, deve ser editado o arquivo "mosquitto.conf", utilizando o código:

```bash
sudo micro /etc/mosquitto/mosquitto.conf
```

Uma tela como essa deverá se abrir:

![6](https://github.com/user-attachments/assets/59ceb53d-1d96-455a-b392-4e5fb5695609)

Agora, adicione no final desse arquivo as segunites linhas:

> listener 1883
> 
> allow_anonymous true

Para salvar as alterações, aperte “Ctrl + S” e, para sair do editor de texto, use “Ctrl + Q”. Isso feito, precisamos reiniciar o Mosquitto para garantir que as modificações sejam aplicadas rodando na linha de comando:

```bash
sudo systemctl restart mosquitto
```

### Instalação do Node-RED

Como uma forma rápida de instalar o Node-RED e o Node.js no Linux, usamos um script hospedado na internet escrevendo o seguinte código na linha de comando:

```bash
bash <(curl -sL https://raw.githubusercontent.com/node-red/linux-installers/master/deb/update-nodejs-and-nodered)
```

Digite "y" para todas as perguntas que aparecerem. Uma tela como essa deverá ser aberta:

![7](https://github.com/user-attachments/assets/6ed31260-14bf-479d-80bd-6a9627e89de0)

Aguarde até que todas as ações sejam realizadas. Apos um momento, você terá que realizar algumas configurações:

![12](https://github.com/user-attachments/assets/c13195f6-93f8-415e-a26b-087863eaee18)

> - Para criar um arquivo de configuração, aperte a tecla "Enter";
> - O compartilhamento de dados de usuário é opcional;
> - Quando perguntado sobre configurar a segurança de usuário, selecione "Yes";
> - Insira um nome de usuário e senha para o Node-RED;
> - Sobre a permissão de usuário, selecione "full acess";
> - Responda "No" para a criação de um outro usuário;
> - Recuse o recurso Projects;
> - A respeito da configuração do Flow File, aperte a tecla "Enter" duas vezes;

![13](https://github.com/user-attachments/assets/3dbb5ded-9240-4096-b1a8-25e6eed12936)

> - Nas configurações de editor, selecione a opção "default" duas vezes;
> - Por fim, selecione "Yes" para permitir que nós de função carreguem módulos externos.

Se todos os passos foram seguidos, a instalação foi concluída. Agora, para evitar que o Node-RED consuma toda a RAM e deixe o sistema lento, limitamos a memória a 256 MB com o seguinte comando:

```bash
node-red-pi --max-old-space-size=256
```
Você deverá obter a seguinte resposta:

![14](https://github.com/user-attachments/assets/3d40eb14-58f0-4a90-88a9-c9f6021fd150)

Aqui, é possível verificar que o Node-RED está rodando sob o host local na porta 1880. Para prosseguir, pare o andamento do software com "Ctrl + C" e, para iniciar o Node-RED como um serviço, use o comando:

```bash
node-red-start
```

Agora, mesmo se você usar "Ctrl + C", o software ainda estará rodando no background.

Finalmente, use o seguinte código para iniciar o Node-RED automaticamente toda vez que o sistema for ligado:

```bash
sudo systemctl enable nodered.service
```

### Instalação do InfluxDB

Para baixar, converter e instalar a chave pública do repositório da InfluxData no seu Raspberry Pi, utilize o seguinte código na linha de comando:

```bash
curl https://repos.influxdata.com/influxdata-archive.key | gpg --dearmor | sudo tee /usr/share/keyrings/influxdb-archive-keyring.gpg >/dev/null
```
Para adicionar o repositório oficial da InfluxData (InfluxDB, Telegraf, etc.) ao APT, permitindo que você instale e atualize esses pacotes diretamente via ```apt```, use o comando:

```bash
echo "deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main" | sudo tee /etc/apt/sources.list.d/influxdb.list
```

Agora, usamos o seguinte comando para encontrar a última versão versão do InfluxDB no repositório:

```bash
sudo apt update
```

Isso feito, podemos instalar o InfluxDB2 com o seguinte código:

```bash
sudo apt install influxdb2
```

Após instalado, para desbloquear o serviço ```influxdb``` e permitir que ele seja iniciado ou habilitado normalmente, rode:

```bash
sudo systemctl unmask influxdb
```

Para configurar o serviço do InfluxDB para iniciar automaticamente toda vez que o sistema for ligado, use o comando:

```bash
sudo systemctl enable influxdb
```

Agora, para iniciar o InfluxDB como um serviço, use:

```bash
sudo systemctl start influxdb
```

### Instalação do Grafana

Para baixar e armazenar com segurança a chave GPG do repositório oficial do Grafana, use o seguinte código da linha de comando:

```bash
curl https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /usr/share/keyrings/grafana-archive-keyrings.gpg >/dev/null
```

Então, para adicionar o repositório oficial do Grafana ao gerenciador de pacotes APT, rode:

```bash
echo "deb [signed-by=/usr/share/keyrings/grafana-archive-keyrings.gpg] https://apt.grafana.com stable main" | sudo tee /etc/apt/sources.list.d/grafana.list
```

Agora, atualizamos novamente o sistema APT usando:

```bash
sudo apt update
```

Podemos agora instalar o Grafana rodando:

```bash
sudo apt install grafana
```

Pressione "y" para confirmar a instalação, que poderá levar um certo tempo. Uma vez instalado, usamos o seguinte comando para que o Grafana seja iniciado junto com o sistema operacional:

```bash
sudo systemctl enable grafana-server
```

Por fim, podemos iniciar o Grafana imediatamente utilizando:

```bash
sudo systemctl start grafana-server
```
## Configuração para aquisição de dados

Feita a instalação das quatro ferramentas necessárias para o nosso Dashboard, precisamos agora configurar o Raspberry Pi para realizar a aquisição dos dados de temperatura e umidade do sensor DHT11.

### Configuração do Python

Para instalar o ambiente completo de Python 3 no seu sistema, execute na linha de comando:

```bash
sudo apt install python3 python3-pip python3-venv
```

Instalamos agora a biblioteca ```libgpiod2```, que permite o acesso e controle das GPIOs (General Purpose Input/Output) em sistemas Linux:

```bash
sudo apt install libgpiod2
```

Também fazemos a instalação da biblioteca ```rpi.gpio```, permitindo que scripts Python controlem os pinos GPIO da placa:

```bash
sudo apt-get install python3-rpi.gpio
```

Digite "y" para confirmar a instalação. 

```bash
sudo apt install python3 python3-pip python3-venv
```

Agora, criamos uma pasta que conterá o código em Python para obtenção de dados usando:

```bash
mkdir ~/dht11
```

Para acessar a pasta, use:

```bash
cd dht11
```

Criamos então um ambiente virtual Python chamado ```env``` dentro desse diretório, utilizando o comando:

```bash
python3 -m venv env
```

Ativamos agora o ambiente virtual, rodando:

```bash
source env/bin/activate
```

Seguindo, rodamos o seguinte código para instalar duas bibliotecas Python necessárias para leitura e envio de dados do sensor, usando o ```pip3```:

```bash
pip3 install adafruit-circuitpython-dht paho.mqtt
```

Podemos agora criar o programa em Python ```mqtt.py``` que conterá o código para a leitura e envio de dados de umidade e temperatura do sensor DHT11, utilizando o editor de texto ```micro```:

```bash
micro mqtt.py
```

Dentro do editor, copie o código disponível na pasta: [codes/mqtt-dht11.py](https://github.com/emiliomercuri/MING-Lactea/blob/main/codes/mqtt-dht11.py). Salve as alterações com “Ctrl + S” e, saia do editor de texto usando “Ctrl + Q”.

Agora, dentro do ambiente virtual, podemos executar o programa em Python com o código:

```bash
python3 mqtt.py
```

## Configuração do Node-RED, InfluxDB e Grafana

Agora, precisamos configurar cada uma das quatro ferramentas.

### Configuração do Node-RED

Para iniciar, precisaremos do endereço de IP "wlan0" (caso sua conexão de internet seja por wi-fi) ou "eth0" (caso sua conexão seja cabeada), que podem ser obtidos usando o código:

```bash
ifconfig
```

A seguinte resposta deverá apacerer. No meu caso, a conexão do Rasbperry Pi está sendo feita por cabo, e o endereço que vou usar está destacado em vermelho:

![18](https://github.com/user-attachments/assets/4fab2932-9a36-450f-a4ca-04cde120636a)

Agora, utilizando um navegador web, é possível acessar o Node-RED utilizando o endereço de IP, acrescentando ":" e o número da porta de acesso "1880" na barra de busca:

> 192.168.1.22:1880

Um site como esse deverá se abrir, e você terá que adicionar suas credenciais configuradas na instalação do Node-RED:

![19](https://github.com/user-attachments/assets/5391a8e1-a646-42c3-a3e2-2e0570020151)

### Configuração do InfluxDB

Para acessar o InfluxDB através do seu navegador, utilize na barra de busca o seu endereço de IP, acrescido de ":" e o número "8086". Em seguida, clique em "Get started", e preencha as informações:

![21](https://github.com/user-attachments/assets/7f0e6c80-6fc2-4146-98a3-170c284b87b9)

Aqui, você deverá inserir:

> Nome de usuário e senha, para acessar ferramenta;
>
> Nome inicial da organização;
>
> Nome inicial do Bucket, que é onde seus dados serão armazenados. Aqui se escolheu o nome "sensordata" por simplicidade.

Feito isso, clique em "Continue", e uma nova aba deverá aparecer. Certifique-se de copiar e guardar seu "operator  API token", para um possível uso futuro. Em seguida, clique em "Quick start".

### Configuração do Grafana

O acesso ao Grafana também é feito através do navegador, usando o endereço de IP acrescido de ":" e o número "3000" na barra de busca. Feito o acesso, você terá que adicionar as seguintes credencias iniciais:

> Usuário: admin
>
> Senha: admin

Após isso, clque em "Log in". Será então pedido para você alterar sua senha. Após a alteração, clique em "Submit" 

