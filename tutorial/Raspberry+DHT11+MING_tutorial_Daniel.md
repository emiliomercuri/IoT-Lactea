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
## 5. Configuração para aquisição de dados

Feita a instalação das quatro ferramentas necessárias para o nosso Dashboard, precisamos agora configurar o Raspberry Pi para realizar a aquisição dos dados de temperatura e umidade do sensor DHT11.

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

Digite "y" para confirmar a instalação. Agora, criamos uma pasta que conterá o código em Python para obtenção de dados usando:

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

Também deve ser instalado mais três bibliotecas importantes para o funcionamento do script em Python, usando o código:

```bash
pip3 install adafruit-circuitpython-minimqtt RPI.GPIO adafruit-blinka
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

Você também pode fazer com que esse scipt em Python rode como um seviço que inicia ao ligar o sistema operacional. Basta seguir o seguinte tutorial: [Service_for_python_aquisition.md](https://github.com/emiliomercuri/MING-Lactea/blob/main/tutorial/Service_for_python_aquisition.md). Vale ressaltar que, antes de iniciar o tutorial, você deve sair do ambiente virtual Python com o comando ```deactivate```.

## 6. Configuração do Node-RED, InfluxDB e Grafana

Agora, precisamos configurar cada uma das quatro ferramentas.

### Acesso ao Node-RED

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

### Acesso ao InfluxDB

Para acessar o InfluxDB através do seu navegador, utilize na barra de busca o seu endereço de IP, acrescido de ":" e o número "8086". Em seguida, clique em "Get started", e preencha as informações:

![21](https://github.com/user-attachments/assets/7f0e6c80-6fc2-4146-98a3-170c284b87b9)

Aqui, você deverá inserir:

> Nome de usuário e senha, para acessar ferramenta;
>
> Nome inicial da organização;
>
> Nome inicial do Bucket, que é onde seus dados serão armazenados. Aqui se escolheu o nome "sensordata" por simplicidade.

Feito isso, clique em "Continue", e uma nova aba deverá aparecer. Certifique-se de copiar e guardar seu "operator  API token", para um possível uso futuro. Em seguida, clique em "Quick start".

### Acesso ao Grafana

O acesso ao Grafana também é feito através do navegador, usando o endereço de IP acrescido de ":" e o número "3000" na barra de busca.

![22](https://github.com/user-attachments/assets/6cae4148-b5d6-4baa-b7bd-4ad5db20663e)


Feito o acesso, você terá que adicionar as seguintes credencias iniciais:

> Usuário: admin
>
> Senha: admin

Após isso, clque em "Log in". Será então pedido para você alterar sua senha. Após a alteração, clique em "Submit". 

### Fazendo o MING Stack

Agora, no Node-RED, usando a barra de busca à esquerda, pesquise por "mqtt" e arraste o nó "mqtt in" para a área quadriculada. Clique duas vezes em cima do nó para iniciar sua configuração. Uma aba como essa deverá se abrir:

![24](https://github.com/user-attachments/assets/ea09910f-f161-4014-8a15-09976b4c6927)

Clique no símbolo de "+" destacado em vermelho na imagem para acessar as propriedades do nó:

![25](https://github.com/user-attachments/assets/0658d97e-37b0-456b-a267-b99b94a45db9)

Agora, no campo "Servidor", preencha com **"localhost"** e depois clique em "Adicionar".

![26](https://github.com/user-attachments/assets/79ac76bb-b2b3-456d-996d-904523d39385)

Preencha o campo "Tópico" com **"dht11/sensordata"** (como no script em Python), e defina o "QoS" como **"1"**. Após isso, clique em "Feito".

Em seguida, podemos usar o nó de Debug para entender o que está acontecendo. Para isso, procure por "Debug" na barra de busca à esquerda e arraste o nó para a área quadriculada. Após isso, conecte os dois nós, formando algo parecido com a imagem abaixo:

![27](https://github.com/user-attachments/assets/5b47af14-fccb-498c-b062-cb6d903dc053)

Clique então no botão "Implementar", destacado em vermelho. Você poderá então ver os dados de umidade e temperatura sendo gerados à direita na aba "mensagens de depuração".

Agora, precisamos instalar o InfluxDB dentro do Node-RED. Para isso, arraste o mouse para o ícone de três barras no canto superior direito e clique em "Gerenciar paleta":

![28](https://github.com/user-attachments/assets/475d6d71-6d2a-4d58-b0c1-b9092726f67b)

Vá então para a aba "Instalar" e procure por "influx". Instale a segunda opção, destacada em vermelho abaixo:

![29](https://github.com/user-attachments/assets/9f6a96fe-64f1-4aa3-a93f-122f04d5e616)

Após instalado, feche a aba e busque pelo nó "influx out". Arraste ele para a área quadriculada e conecte ao nó "dht11/sensordata", como mostrado abaixo:

![30](https://github.com/user-attachments/assets/42b7f07a-4993-4d10-9050-9321d4bcb5e1)

Precisamos agora configurar o nó. Para isso, clique duas vezes sobre ele e depois clique no botão "+" destacado abaixo:

![31](https://github.com/user-attachments/assets/0e16a2ff-ecd9-4e50-aa2f-f66e27d8af24)

Nas propriedades, preencha o campo "Host" com **"localhost"** e defina a versão como **"2.0"**:

![32](https://github.com/user-attachments/assets/da39d9a0-d2e6-42e1-b711-bc5015d01416)

O Token pode ser obtido através do site do InfluxDB. Para gerá-lo, vá até o site e procure por "API Tokens" na barra à esquerda, como mostrado abaixo:

![33](https://github.com/user-attachments/assets/d71d2600-7823-4419-bda7-dfb6a5e14bc8)

Em seguida, clique em "Generate API Token" e depois em "All Acess API Token":

![34](https://github.com/user-attachments/assets/a8a0d1cc-1ac3-4f3a-a227-8f6fb12abb34)

Dê uma descrição para o seu Token, em seguida clique em "Save". Após isso, copie o Token destacado em roxo:

![Captura de tela 2025-06-23 183338](https://github.com/user-attachments/assets/9e9dd4c7-0648-4b4a-8af6-82cde82f2c14)

Esse Token deverá ser copiado na configuração do nó "influxdb out" no Node-RED. Após copiado no campo "Token", clique em "Adicionar". Preencha o campo "Organization" com nome da organização que você colocou no primeiro acesso ao InfluxDB (aqui, o nome dado foi **"lactea"**). Também preencha o nome do Bucket, que no meu caso é **"sensordata"**. No campo "Measurement", pode ser criado um nome qualquer. Aqui foi utilizado **"dht11"** como um exemplo.

![Captura de tela 2025-06-23 184451](https://github.com/user-attachments/assets/2d73874b-e017-49d6-a679-19d44126386c)

 Após essas alterações, clique em "Feito", e depois em "Implementar", no canto superior direito.

 Feito isso, agora você pode acessar os dados do sensor através do InfluxDB. Para isso, vá até o site e acesse a aba "Buckets" através da barra à esquerda:

![Captura de tela 2025-06-23 191010](https://github.com/user-attachments/assets/3c7fda8b-0c5c-4c0a-a66a-bf5039bc92b8)

Em seguida, clique sobre o nome do Bucket que você criou anteriormente:

![Captura de tela 2025-06-23 191248](https://github.com/user-attachments/assets/fdb23f15-29ed-4cd1-92fa-36366ecb4fc8)

Agora, uma aba como a abaixo deverá ter se aberto:

![Captura de tela 2025-06-23 191612](https://github.com/user-attachments/assets/dad3cdd4-567c-4fb6-bd0c-f9cd12ce1ab2)

Para visualizar seus dados, selecione o nome do Bucket e o "Measurement" que foi definido no Node-RED. Em seguida, selecione qual o dado que você quer que apareça (aqui, foi selecionado temperatura e umidade simultâneamente). Na parte superior, você pode escolher como o dado vai ser apresentado. Após ter selecionado as opções desejadas, clique em "Submit" na direita para ver seus dados sendo gerados.

Depois de configurado o InfluxDB, podemos partir para o Grafana. Na pagina inicial, clique em "Data sources":

![Captura de tela 2025-06-23 192629](https://github.com/user-attachments/assets/59636f75-9ee6-4593-9d8c-fea1647ffaa9)

Então clique em "InfluxDB". Uma aba de configuração deverá ter sido aberta:

![Captura de tela 2025-06-23 193338](https://github.com/user-attachments/assets/f1f4b712-5bf6-4e3f-bb85-3b60c364b90b)

No campo "Query Language", selecione **"Flux"** e preencha o URL com **"http://localhost:8086"**. Agora, mais para baixo, temos que preencher os dados do InfluxDB:

![Captura de tela 2025-06-23 193919](https://github.com/user-attachments/assets/a24f5fb8-5bee-457c-81cf-a3a5a52a16ca)

No campo "Organization", preencha com o nome da organização definida anteriormente. Para o campo "Token", um novo API Token deverá ser criado no InfluxDB, usando o mesmo processo anterior. Preencha o campo "Bucket" com o mesmo nome de Bucket configurado nos passos anteriores. Após preenchido esses campos, clique em "Save and Test". Uma mensagem como a abaixo deverá surgir:

![Captura de tela 2025-06-23 194404](https://github.com/user-attachments/assets/6679ebc1-4db0-473f-b9c3-22cb613cb8c2)

Feita a configuração do Grafana, podemos agora criar um Dashbord que mostra os dados obtidos pelo sensor em tempo real. A partir da aba "Dashboard" disponível à esquerda, clique em "Create Dashboard"

![Captura de tela 2025-06-23 194935](https://github.com/user-attachments/assets/e854c805-9a1c-4035-9e75-51b89615c187)

Em seguida, clique em "Add visualization", e selecione a opção "influxdb" como fonte de dados:

![Captura de tela 2025-06-23 195206](https://github.com/user-attachments/assets/c8d3f6cc-584c-4197-a44a-177bccb78dce)

Agora, precisamos de um sample query, que deve ser incluído no campo destacado em vermelho:

![Captura de tela 2025-06-23 195654](https://github.com/user-attachments/assets/b9aea0e4-a6a8-4ec2-a98a-7af6a7ac831a)

Isso pode ser obtido voltando para a interface do InfluxDB:

![Captura de tela 2025-06-23 200119](https://github.com/user-attachments/assets/a28c86d7-4813-4502-8062-c96ce9db7693)

Ao acessar o Bucket em que está sendo armazenado os dados, é possível obter o script que deverá ser colado no Grafana selecionando o dado desejado, e depois clicando em "Script Editor":

![Captura de tela 2025-06-23 200119](https://github.com/user-attachments/assets/57eee58f-be9b-401e-8cee-a99dc4e40379)

Você deverá então copiar todas as linhas do script, **exceto a última**:

![Captura de tela 2025-06-23 200119](https://github.com/user-attachments/assets/4106d6d3-2199-485a-aa44-60cee1637b28)

Agora, volte ao Grafana e cole o script no campo destacado:

![Captura de tela 2025-06-23 201038](https://github.com/user-attachments/assets/7935e596-4603-459a-af64-cd63fd15545d)

Após isso, clique em "Query inspector" e depois em "Refresh". Assim você acaba de criar um painel que mostra os dados obtidos a partir do sensor DHT11 em tempo real:

![Captura de tela 2025-06-23 201618](https://github.com/user-attachments/assets/38fd06d2-6a17-4acd-99dd-c34c7527c14f)

Agora, é possível dar um nome ao seu painel de dados e salvar o seu Dashboard.















