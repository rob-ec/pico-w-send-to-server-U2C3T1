#include <stdio.h>
#include <string.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/async_context.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "lwip/altcp_tls.h"
#include "example_http_client_util.h"

// As credenciais do Wi-Fi foram movidas para secrets.h para segurança
// #include "secrets.h"

#define SERVER_HOST "192.168.0.130"
const uint16_t SERVER_PORT = 3000;

// --- DEFINIÇÃO DOS PINOS ---
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define JOYSTICK_SW_PIN 22
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_X_ADC_CH 1 // GPIO 27 é o ADC 1
#define JOYSTICK_Y_ADC_CH 0 // GPIO 26 é o ADC 0

/**
 * @brief Prepara os dados dos sensores e os envia via HTTP GET.
 *
 * @param context O contexto assíncrono para operações de rede.
 * @param ... (parâmetros dos sensores)
 * @return true se os dados foram enviados com sucesso, false caso contrário.
 */
bool send_data(
    async_context_t *context,
    bool button_a_pressed,
    bool button_b_pressed,
    bool joystick_sw_pressed,
    uint16_t joystick_x,
    uint16_t joystick_y)
{
    // Buffer para armazenar a URL formatada com os parâmetros
    char url_buffer[256];

    // Monta a string da URL com os dados dos sensores como parâmetros de consulta
    snprintf(url_buffer, sizeof(url_buffer),
             "/log-get?button1=%s&button2=%s&joystick_pressed=%s&joystick_x=%d&joystick_y=%d",
             button_a_pressed ? "true" : "false",
             button_b_pressed ? "true" : "false",
             joystick_sw_pressed ? "true" : "false",
             joystick_x,
             joystick_y);

    printf("Enviando GET para: %s%s\n", SERVER_HOST, url_buffer);

    // Configura a requisição HTTP GET
    EXAMPLE_HTTP_REQUEST_T req = {0};
    req.hostname = SERVER_HOST;
    req.port = SERVER_PORT;
    req.url = url_buffer; // A URL agora contém todos os dados

    // Executa a requisição HTTP de forma síncrona e verifica o resultado
    int result = http_client_request_sync(context, &req);
    if (result != 0)
    {
        printf("Falha na requisição HTTP, código de erro: %d\n", result);
        return false;
    }
    else
    {
        printf("Dados enviados com sucesso!\n");
        return true;
    }
}

void connect_to_wifi() {
    printf("Conectando ao Wi-Fi: %s...\n", WIFI_SSID);
    // Tenta conectar repetidamente até conseguir
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar. Tentando novamente em 5 segundos...\n");
        sleep_ms(5000);
    }
    printf("Conectado com sucesso!\n");
}

int main()
{
    stdio_init_all();

    // --- Inicialização da Rede ---
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_WORLDWIDE)) {
        printf("Falha ao inicializar Wi-Fi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();
    connect_to_wifi();

    // --- Inicialização dos Botões e Joystick ---
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_init(JOYSTICK_SW_PIN);
    gpio_set_dir(JOYSTICK_SW_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_SW_PIN);

    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    printf("Iniciando leitura dos botões e joystick...\n");

    // Loop infinito para leitura e envio
    while (true)
    {
        // Verifica o status da conexão Wi-Fi e tenta reconectar se necessário
        int link_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
        if (link_status < 0 || link_status == CYW43_LINK_DOWN) {
            printf("Conexão Wi-Fi perdida. Tentando reconectar...\n");
            connect_to_wifi();
        }

        // --- Leitura dos Sensores ---
        bool button_a_pressed = !gpio_get(BUTTON_A_PIN);
        bool button_b_pressed = !gpio_get(BUTTON_B_PIN);
        bool joystick_sw_pressed = !gpio_get(JOYSTICK_SW_PIN);

        adc_select_input(JOYSTICK_X_ADC_CH); // Eixo X
        uint16_t joystick_x = adc_read();
        adc_select_input(JOYSTICK_Y_ADC_CH); // Eixo Y
        uint16_t joystick_y = adc_read();

        // Chama a função para preparar e enviar os dados
        bool success = send_data(
            cyw43_arch_async_context(),
            button_a_pressed,
            button_b_pressed,
            joystick_sw_pressed,
            joystick_x,
            joystick_y);

        // Aguarda um tempo antes da próxima leitura/envio.
        // Se o envio falhou (ex: servidor offline), espera mais tempo.
        if (success) {
            sleep_ms(1000); // Intervalo normal
        } else {
            printf("Aguardando 5 segundos antes de tentar novamente...\n");
            sleep_ms(5000); // Intervalo maior em caso de falha
        }
    }

    // Desinicializa o módulo Wi-Fi (o código nunca chegará aqui no loop infinito)
    cyw43_arch_deinit();
    return 0;
}