# Acesso Remoto via Raspberry Pi Connect (Modo Headless)

Este guia explica como acessar o terminal do seu Raspberry Pi 3B+ de qualquer lugar utilizando o serviço **Raspberry Pi Connect**, sem a necessidade de monitor, teclado ou configurações de portas no roteador.

## 1. Inicialização do Hardware
1. Insira o cartão MicroSD (gravado com o token do Connect) no seu **Raspberry Pi 3B+**.
>Caso você não saiba como fazer o flashing do cartão SD para instalar o sistema operacional e configurar o Raspberry Pi Connect, acesse o seguinte tutorial: https://github.com/emiliomercuri/IoT-Lactea/blob/main/tutorial/Instala%C3%A7%C3%A3o%20de%20Sistema%20Operacional%20no%20Raspberry%20Pi.md

2. Conecte o **cabo de rede (Ethernet)**.
   > **Dica:** O uso do cabo garante que o serviço de nuvem suba sem instabilidades de sinal de Wi-Fi.
3. Conecte a fonte de alimentação (5V / 2.5A).
4. **Aguarde de 2 a 5 minutos:** No primeiro boot, o sistema expande o sistema de arquivos e inicializa os serviços de rede. 

## 2. Acesso via Portal Web
1. Em qualquer computador com internet, acesse: [connect.raspberrypi.com](https://connect.raspberrypi.com/).
2. Faça login com a conta utilizada para gerar o token no Raspberry Pi Imager.
3. No painel de controle (Dashboard), você verá o dispositivo com o hostname: **`lactea3-daniel`**.
4. Verifique se o status está como **"Online"**.

## 3. Iniciando o Remote Shell
Como instalamos a versão **Lite** (sem interface gráfica), o acesso será feito via terminal:
1. Clique no botão **"Connect"** ao lado do nome do dispositivo.
2. Selecione a opção **"Remote Shell"**.
3. Uma janela de terminal será aberta diretamente no seu navegador.
4. O sistema solicitará as credenciais configuradas anteriormente:
   * **login:** `<nome_de_usuário>`
   * **Password:** `<senha>`
* **Segurança de Rede:** O serviço atravessa firewalls acadêmicos ou residenciais de forma segura, eliminando a necessidade de configurações complexas de IP fixo ou DDNS.

