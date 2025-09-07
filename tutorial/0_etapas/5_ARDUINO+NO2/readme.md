# 1. Materiais necessários

## - Raspberry Pi Model B
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/bcc39b87-5531-4bb5-a937-91739d8e6a6c" />

## - Arduino UNO
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/96cf8247-5d39-4728-839d-c67f0f3651eb" />


## - Sensor Alphasense NO2-B43F
<img width="320" height="260" alt="image" src="https://github.com/user-attachments/assets/ca47cf53-549e-4ac5-b8c0-33c6fd739561" />


## - Conversor Analógico Digital ADS1115
<img width="300" height="300" alt="image" src="https://github.com/user-attachments/assets/243d5884-d71c-4a44-a020-9a1c6448edfb" />


## - Jumpers (4 fêmea-fêmea e 6 fêmea-macho)


# 2. Conexões físicas

<img width="909" height="885" alt="Diagrama sensor NO2_bb" src="https://github.com/user-attachments/assets/643aa7ab-2999-414d-9ff8-d8385056cf58" />

(adicionar explicação detalhada das conexões)


# 3. Software

## 3.1 Configuração do Arduino

Antes de começar, verifique em qual porta serial está conectado o Arduino UNO no seu Raspberry Pi, utilizando o seguinte código na linha de comando:

```bash
ls /dev/ttyUSB*
```

A porta serial que aparecer como resposta vai ser a utilizada durante a configuração. Agora, crie e acesse pasta onde será inserido as instruções e códigos para a leitura do sensor:

```bash
mkdir platformio_tutorial_no2
cd platformio_tutorial_no2
```

Agora adicione o arquivo de configuração do projeto do Arduino:

```bash
micro platformio.ini
```

Dentro do arquivo de texto, cole o seguinte código:

```bash
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 115200
upload_port = /dev/ttyUSB0
lib_deps =
  RobTillaart/ADS1X15@^0.4.0
```

Salve o conteúdo utilizando o atalho "Ctrl + S" e saia com "Crtl + Q". Crie agora a pasta "source", onde ficará o código do Arduino que faz a leitura dos sinais brutos do sensor:

```bash
mkdir src
cd src
```

Adicione na pasta "source" o arquivo ```main.cpp```:

```bash
micro main.cpp
```

Cole no arquivo de texto o seguinte cógido:

```bash
#include <Arduino.h>
//
//    FILE: ADS_read.ino
//  AUTHOR: Rob.Tillaart
// PURPOSE: read analog inputs - straightforward.
//     URL: https://github.com/RobTillaart/ADS1X15

//  test
//  connect 1 potmeter per port.
//
//  GND ---[   x   ]------ 5V
//             |
//
//  measure at x (connect to AIN0).


#include "ADS1X15.h"

ADS1115 ADS(0x48);


void setup() 
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  Wire.begin();
  ADS.begin();
}


void loop() 
{
  ADS.setGain(0);

  //int16_t val_0 = ADS.readADC(0);  
  int16_t val_1 = ADS.readADC(1);  
  //int16_t val_2 = ADS.readADC(2);  
  int16_t val_3 = ADS.readADC(3);  

  float f = ADS.toVoltage(1);  //  voltage factor

  Serial.print("\tAnalogs1+3: "); 
  Serial.print('\t'); 
  Serial.print(val_1); 
  Serial.print('\t'); 
  Serial.print(val_1 * f, 3); 
  Serial.print('\t'); 
  Serial.print(val_3);
  Serial.print('\t');   
  Serial.println(val_3 * f, 3);
  //Serial.println();

  delay(1000);
}


//  -- END OF FILE --
```

Novamente, salve o conteúdo com "Ctrl + S" e saia com "Crtl + Q". Agora, saia da pasta "source" e use o seguinte código na linha de comando para compilar e enviar o projeto para o Arduino:

```bash
pio run --target upload
```

Para verificar se os dados estão sendo enviados para a porta serial do Raspberry Pi, rode:

```bash
pio device monitor
````

Uma mensagem semelhante a essa deverá aparecer:

<img width="868" height="507" alt="image" src="https://github.com/user-attachments/assets/3ba60b96-6b9e-4eda-a795-4a9f94e76b04" />

\n
Pause a operação utilizando o atalho "Ctrl + C". AgoraCrie o script em linguagem Python que fará a leitura e armazenamento dos dados do sensor de NO2:

```bash
micro aquisicao_tutorial_no2.py
```

Cole o seguinte código no arquivo:

```bash
#!/usr/bin/python
# -*- coding: UTF-8 -*-
import serial, time, struct, array
import pandas as pd
import datetime
import os
import sys
import os.path
from os import path

data = datetime.datetime.now().strftime("%Y-%m-%d")
hora = datetime.datetime.now().strftime("%Y-%m-%d_%H")
#hora = datetime.datetime.now().strftime("%Y-%m-%d_%H:%M")
hora_atual = hora

if path.exists("/root/lactea/data/") != True:
    myCmd = 'mkdir /root/lactea/data/'
    os.system(myCmd)        

#########

# Define the serial port and baud rate. Update the port with the correct one for your setup.
# On Windows, the port might look like 'COM3' or 'COM4'.
# On Linux/macOS, it might look like '/dev/ttyUSB0' or '/dev/ttyACM0'.
#serial_port = '/dev/cu.usbserial-2140'  # Replace with your serial port
serial_port = '/dev/ttyUSB0'
baud_rate = 115200
#file_name = 'data_log.csv'

# the following value is column2 from the file in the bag of the sensor
zero_offset_WE = 230.0
zero_offset_aux = 230.0
sensor_sensitivity = 230.0
WE_sensitivity = 165.0


# Open the serial connection
ser = serial.Serial(serial_port, baud_rate)
time.sleep(2)  # Give the connection a second to settle

# Cria um dataframe vazio
df = pd.DataFrame(columns=['NO2(ppb)'])

# Loop da hora
while hora == hora_atual:
    # Read data from the serial port
    line = ser.readline().decode('utf-8').strip()
    #print(line)
    separate = line.split() 
    #print(separate)
    #print(len(separate))

    horario = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    if len(separate) == 5:
        #print(separate[2], separate[4])
        WE_OP1 = float(separate[2])*1000 # transform from volts to mili volts
        Aux_OP2 = float(separate[4])*1000 # transform from volts to mili volts
        col_a = horario # From your data acquisition system
        col_b = ",{0:.3f}".format(WE_OP1) # mV from ISB channel 1
        col_c = ",{0:.3f}".format(WE_OP1-zero_offset_WE) # Column B- Vo 
        col_d = ",{0:.3f}".format(float(Aux_OP2)) # mV from ISB channel 2
        col_e = ",{0:.3f}".format(float(float(Aux_OP2)-zero_offset_aux)) # Column D- Vo
        col_f = "{0:.3f}".format((float(float(WE_OP1)-zero_offset_WE))*1000/WE_sensitivity) # Column C / sensitivity
        #col_g = ",{0:.3f}".format((float(float(WE_OP1)-zero_offset_WE))-(float(float(Aux_OP2)-zero_offset_aux))) # Column C – Column E
        #col_h = ",{0:.3f}".format(((float(float(WE_OP1)-zero_offset_WE))-float(float(Aux_OP2)-zero_offset_aux))/WE_sensitivity)# Column G / sensitivity            
        #f.write(col_a + col_b + col_c + col_d + col_e + col_f + col_g + col_h + '\n')
        #f.write(col_a + col_b + col_c + col_d + col_e + col_f + '\n')
        #f.write(col_a + col_f + '\n')
        dicionario = {col_a: float(col_f)}   
        #new_row = pd.DataFrame([dicionario],columns=columns)

        new_row = pd.DataFrame.from_dict(dicionario, orient='index', columns=['NO2(ppb)'])
        # Renomear o índice usando rename()  
        #new_row = new_row.rename(index={0: 'Time', 1: 'NO2(ppb)'})  
        #print(new_row)
        df = pd.concat([df,new_row],ignore_index=False)
        df.to_csv(f"/root/lactea/data/NO2_IC-Daniel.csv", sep=',', index=True)
        #df[col_a] = col_f           
        #print(col_a + col_f)
        #print(df)

    #hora = datetime.datetime.now().strftime("%Y-%m-%d_%H:%M")    
    hora = datetime.datetime.now().strftime("%Y-%m-%d_%H")    
    data = datetime.datetime.now().strftime("%Y-%m-%d")
    
    if hora != hora_atual:
        df.to_csv(f"/root/lactea/data/NO2_IC-Daniel_{hora_atual}.csv", sep=',', index=True)
        df = pd.DataFrame(columns=['NO2(ppb)'])
        hora_atual = hora

# Close the serial connection when done
ser.close()
```

**Importante:** Tenha certeza que a porta serial incluída no código é a mesma conectada ao Arduino. Ademais, troque os valores de sensibilidade dos eletrodos para que sejam os mesmos enviados pelo fabricante.

Salve as alterações e saia do ambiente de edição. Agora, para tornar o script executável, rode na linha de comando:

```bash
chmod +x aquisicao_tutorial_no2.py
```




