Para fazer as configurações iniciais, siga os seguintes passos:

**Boa prática**, realizar as atualizações do Raspberry antes de começar. Utilize o código:
```
sudo apt update && sudo apt upgrade
```
E posteriormente reinicie o Reaspberry:
```
sudo reboot
```

Crie uma pasta para realizar as configurações:

```
mkdir k-30
```
**Boa prática** é criar um diretório para as configurações utilizadas no Python separado do diretório das configurações do Arduino. Por exemplo:
```
mkdir k-30_arduino
mkdir k-30_python
```
Então realize as configurações em cada diretório, respectivamente.

# Instalar o pacote para o Arduino

Para o Raspberry poder identificar o Arduíno, é necessário baixar os pacotes específicos. Para este caso, estamos utilizando o Arduino Nano, logo, utilizaremos os pacotes relacionados à ele.

Primeiro, encontre a porta USB que está sendo utilizada pelo Arduino:
```
ls /dev/ttyUSB*
```
Caso esteja somente o Arduino conectado, deverá aparecer algo como:
```
/dev/ttyUSB0
```

## Instalar o pacote para o Arduino Nano

Utilize o seguinte código:
```
sudo apt install minicom
minicom -b 9600 -o -D /dev/ttyUSB0
```

O seguinte comando mostra os dados recebidos:
```
dmesg | tail
```
Para sair do **minicom**, precione *Crtl + A* e depois *X*.

# Instalar o Python no Raspberry

Agora vamos instalar o Python3 no Raspberry, pois através dele iremos poder ler e salvar os dados do sensor. Para instalar o Python, utilize o seguinte código:
```
sudo apt update
sudo apt install python3-serial
```
Para ter certeza de que houve a instalação, verifique a versão do Python instalada, através do seguinte código:
```
python3 -c "import serial; print(serial.__version__)"
```
Para fazer a leitura dos dados, crie um arquivo de texto *.py*, por exemplo:
```
micro read_serial.py
```
Utilize o seguinte código: https://github.com/emiliomercuri/IoT-Lactea/blob/main/codes/K-30%2BRead_serial.py

E execute com o comando:
```
python3 read_serial.py
```

# Instalar o Arduino CLI no Raspberry

Esta etapa permite carregar códigos no Arduino.
Utilize o seguinte código:
```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

## Colocar o PATH:
```
mkdir -p ~/.local/bin
mv ~/bin/arduino-cli ~/.local/bin/
```

Edite o arquivo *.bashrc*:
```
micro ~/.bashrc
```

Adicione ao final do código:
```
export PATH="$HOME/.local/bin:$PATH"
```

Após salvar e sair, recarregue:
```
source ~/.bashrc
```

Para verificar se a instalação foi bem sucedida, visualize a versão:
```
arduino-cli version
```
Deverá aparecer algo como:
```
arduino-cli Version: 0.35.x
```

## Configurar a placa

Iniciar a congiguração:
```
arduino-cli config init
```

Atualizar os índices:
```
arduino-cli core update-index
arduino-cli core install arduino:avr
```

Detectar a placa:
```
arduino-cli board list
```

Pode ser observada as opções disponíveis também:
```
arduino-cli board listall nano
```

Dentro do diretório das configurações para o Arduino, crie o um arquivo do tipo *.ino* para criar o código que será carregado no Arduino, por exemplo:
```
micro k-30-arduino.ino
```
Então cole o código: [codes/k-30+Arduino.cpp](https://github.com/emiliomercuri/IoT-Lactea/blob/main/codes/k-30%2BArduino.cpp)

Após, compile o código com o comando:
```
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old
```

Faça o upload:
```
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:nano:cpu=atmega328old
```

Teste o Serial para ver se os dados estão chegando:
```
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```

## Fazer o upload utilizando o PlatformIO

Esta etapa terá o mesmo resultado que utilizar o Arduino CLI, porém, será utilizado o PlatformIO. Para tanto, crie um arquivo de texto *.ini*, como já feito em outros tutoriais:
```
micro platformio.ini
```
E cole o seguinte código:
```
[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino
upload_port = /dev/ttyUSB0
monitor_speed = 9600
```

Se for **Old Bootloader**:
```
board = nanoatmega328new
```

Então utilize o mesmo código *.cpp* do Arduino acima: [codes/k-30+Arduino.cpp](https://github.com/emiliomercuri/IoT-Lactea/blob/main/codes/k-30%2BArduino.cpp)

Compile e grave:
```
pio run -t upload
```

Leia os dados:
```
pio device monitor
```
