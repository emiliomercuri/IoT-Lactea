# Guia de Instalação: Raspberry Pi OS Lite (64-bit) para Monitoramento Ambiental

Este tutorial detalha o processo de instalação do sistema operacional em um **Raspberry Pi 3B+**, configurado para atuar em uma rede de sensores IoT (monitoramento de NO2, O3 e Material Particulado).

## 1. Instalação do Raspberry Pi Imager
O primeiro passo é baixar a ferramenta oficial de escrita de imagem da Raspberry Pi:
* **Link para Download:** [raspberrypi.com/software](https://www.raspberrypi.com/software/)

## 2. Configuração do Sistema Operativo e Dispositivo
Ao abrir o Imager, siga estas seleções:

1.  **Escolher Dispositivo:** Selecione `Raspberry Pi 3`.
<img width="490" height="350" alt="Captura de tela 2026-03-28 175537" src="https://github.com/user-attachments/assets/4651be7e-975e-42a9-932e-962a5dc6db93" />

2.  **Escolher SO:** Vá em `Raspberry Pi OS (other)` -> `Raspberry Pi OS Lite (64-bit)`.
<img width="490" height="350" alt="Captura de tela 2026-03-28 175609" src="https://github.com/user-attachments/assets/5fb57ab3-13b1-461c-bbd4-b84711250832" />


<img width="490" height="350" alt="Captura de tela 2026-03-28 175642" src="https://github.com/user-attachments/assets/d7e686b5-acc4-40c2-8e89-4a5a4e75e4e3" />

    > **Nota Técnica:** A versão **Lite** não possui interface gráfica. Para um projeto de sensores que rodará scripts Python e MQTT 24/7, isso é ideal, pois economiza memória RAM e processamento, aumentando a estabilidade do sistema. A arquitetura **64 bits** permite o uso de bibliotecas de análise de dados mais modernas.
3.  **Escolher Armazenamento:** Selecione o seu cartão SD (ex: 16GB).
<img width="490" height="350" alt="Captura de tela 2026-03-28 175901" src="https://github.com/user-attachments/assets/98687766-26c3-4c85-a620-4bc14a20339b" />

## 3. Personalização do Sistema (OS Customization)
Após clicar em "Seguinte", selecione **"EDITAR DEFINIÇÕES"**. Esta etapa é crucial para o funcionamento "headless" (sem monitor).

### Hostname
* Aqui, foi definido como: `lactea3-daniel`
* **Por que mudar o Hostname?** Em projetos de IoT com múltiplos dispositivos, o Hostname permite que você encontre o Raspberry na rede digitando `lactea3-daniel.local` em vez de precisar descobrir o endereço IP variável do dispositivo.
<img width="490" height="350" alt="Captura de tela 2026-03-28 180221" src="https://github.com/user-attachments/assets/d34f16df-bcd6-4b23-8908-12c849eb9ee6" />

### Localização e Utilizador
* Em **Localização**, certifique-se de que o fuso horário e o layout do teclado estão corretos (importante para que os logs de dados dos sensores tenham o carimbo de tempo/timestamp correto).
<img width="490" height="350" alt="Captura de tela 2026-03-28 180310" src="https://github.com/user-attachments/assets/e6e135d8-178f-44e4-9061-b3997ea26d2e" />

* Defina um **nome de utilizador** e uma **palavra-passe**.
<img width="490" height="350" alt="Captura de tela 2026-03-28 180435" src="https://github.com/user-attachments/assets/2704b7c5-9ea4-4f71-863f-e335f2b815a1" />

### Serviços e Conectividade
* **Wi-Fi:** Pode ser ignorado se você for utilizar o cabo Ethernet. O cabo oferece uma conexão mais estável para transmissão de dados de sensores.
<img width="490" height="350" alt="Captura de tela 2026-03-28 180519" src="https://github.com/user-attachments/assets/640d944d-5788-484b-948c-ad5d850df173" />

* **SSH:** Ative o SSH e selecione "Utilizar autenticação por palavra-passe". Isso permite que você acesse o terminal do Raspberry de qualquer outro computador na mesma rede.
<img width="490" height="350" alt="Captura de tela 2026-03-28 180744" src="https://github.com/user-attachments/assets/7b38ecaf-f54a-4bcf-aee8-5b0ffb0b348e" />

* **Raspberry Pi Connect:** Ative esta opção para acesso remoto simplificado.
    1. Clique em "Abrir Raspberry Pi Connect".
    2. Você será levado ao navegador para gerar um **Token de autenticação** de maneira automática.
<img width="490" height="350" alt="Captura de tela 2026-03-28 180847" src="https://github.com/user-attachments/assets/b3e8ef47-ab13-47b3-9fc3-dbc8fac37753" />

<img width="490" height="350" alt="Captura de tela 2026-03-28 180933" src="https://github.com/user-attachments/assets/d30f2e39-3883-496b-95d8-1601e7f384a4" />

## 4. Gravação da Imagem
1.  Clique em **"GRAVAR"**.
2.  Um aviso aparecerá informando que **todos os dados existentes no cartão SD serão apagados**.
3.  Confirme em **"SIM"** (ou "EU COMPREENDO, APAGAR E GRAVAR").
4.  Aguarde o processo de gravação e a verificação final.
<img width="490" height="350" alt="Captura de tela 2026-03-28 181020" src="https://github.com/user-attachments/assets/1e33a57e-f9a8-4bde-8e06-fb257e96dfbb" />

