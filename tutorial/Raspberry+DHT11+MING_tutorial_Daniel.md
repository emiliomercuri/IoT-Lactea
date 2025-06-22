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

## 4. MQTT, Influxdb, Node-Red e Grafana (MING Stack)

* Explicar brevemente o que é o MING Stack e seu princípio de funcionamento

Antes de iniciar a instalação dos softwares, precisamos atualizar todos os pacotes instalados no seu sistema para as versões mais recentes disponíveis nos repositórios. Para isso, execute no terminal:

```bash
sudo apt update && sudo apt upgrade
```

Digite "y" para confirmar a instalação, que poderá demorar alguns instantes.

### Instalação do MQTT:

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

Para salvar as alterações, aperte “Ctrl + S” e para sair do editor de texto, use “Ctrl + Q”. Isso feito, precisamos reiniciar o Mosquitto para garantir que as modificações sejam aplicadas rodando na linha de comando:

```bash
sudo systemctl restart mosquitto
```

### Instalação do Node-RED:

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

### Instalação do InfluxDB:

Para baixar, converter e instalar a chave pública do repositório da InfluxData no seu Raspberry Pi, utilize o seguinte código na linha de comando:

```bash
curl https://repos.influxdata.com/influxdata-archive.key | gpg --dearmor | sudo tee /usr/share/keyrings/influxdb-archive-keyring.gpg >/dev/null
```
Para adicionar o repositório oficial da InfluxData (InfluxDB, Telegraf, etc.) ao APT, permitindo que você instale e atualize esses pacotes diretamente via ```apt```, use o comando:

```bash
echo "deb [signed-by=/usr/share/keyrings/influxdb-archive-keyring.gpg] https://repos.influxdata.com/debian stable main" | sudo tee /etc/apt/sources.list.d/influxdb.list
```
