#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>

#define SS_PIN 10
#define RST_PIN 9
#define SD_CS_PIN 4
#define BUZZER_PIN 7

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Lista de UIDs dos cartões RFID e nomes dos médicos associados
struct Medico {
    String uid;
    String nome;
};

Medico medicos[] = {
    {"d2c74119", "Dr. João Silva"}, // Substitua pelo UID do cartão e nome do médico
    {"04fa8708196c80", "Dra. Maria Souza"}, // Adicione mais médicos conforme necessário
};

// UID do cartão especial para apagar registros
String cartaoAdminUID = "313520a3"; // Substitua pelo UID do cartão especial

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    pinMode(BUZZER_PIN, OUTPUT);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Falha ao inicializar o cartão SD!");
        return;
    }
    Serial.println("Sistema de presença inicializado.");
}

void loop() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        String tagUID = getUID(); // Obtém o UID do cartão
        Serial.println("Cartão detectado: " + tagUID);

        // Verifica se o cartão é o cartão especial de administração
        if (tagUID == cartaoAdminUID) {
            Serial.println("Cartão de administração detectado. Apagando registros...");
            apagarRegistros();
            tone(BUZZER_PIN, 2000, 1000); // Emite um som diferente no buzzer
            return; // Sai da função loop para evitar processamento adicional
        }

        // Verifica se o UID está associado a um médico
        String nomeMedico = verificarMedico(tagUID);
        if (nomeMedico != "") {
            Serial.println("Médico identificado: " + nomeMedico);

            // Verifica se o médico já está presente (entrada registrada sem saída)
            bool entradaRegistrada = verificarEntradaRegistrada(tagUID);

            if (entradaRegistrada) {
                // Se já há uma entrada, registra a saída
                registrarSaida(nomeMedico, tagUID);
                Serial.println("Saída registrada.");
            } else {
                // Se não há entrada, registra a entrada
                registrarEntrada(nomeMedico, tagUID);
                Serial.println("Entrada registrada.");
            }

            tone(BUZZER_PIN, 1000, 500); // Emite um som no buzzer (1kHz por 500ms)
        } else {
            Serial.println("Cartão não reconhecido.");
        }

        mfrc522.PICC_HaltA();
    }
}

// Função para obter o UID do cartão RFID
String getUID() {
    String tagUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        tagUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        tagUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    return tagUID;
}

// Função para verificar se o UID está associado a um médico
String verificarMedico(String uid) {
    for (int i = 0; i < sizeof(medicos) / sizeof(medicos[0]); i++) {
        if (medicos[i].uid == uid) {
            return medicos[i].nome;
        }
    }
    return ""; // Retorna vazio se o UID não for encontrado
}

// Função para verificar se há uma entrada registrada sem saída
bool verificarEntradaRegistrada(String uid) {
    File file = SD.open("presenca.txt");
    bool entradaPendente = false;
    if (file) {
        while (file.available()) {
            String linha = file.readStringUntil('\n');
            if (linha.indexOf("UID: " + uid) >= 0) {
                if (linha.indexOf("Entrada") >= 0) {
                    entradaPendente = true;
                } else if (linha.indexOf("Saída") >= 0) {
                    entradaPendente = false;
                }
            }
        }
        file.close();
    }
    return entradaPendente;
}

// Função para registrar a entrada
void registrarEntrada(String nomeMedico, String uid) {
    File file = SD.open("presenca.txt", FILE_WRITE);
    if (file) {
        file.println("Médico: " + nomeMedico + " | UID: " + uid + " | Entrada | Data/Hora: " + getDateTime());
        file.close();
    } else {
        Serial.println("Erro ao abrir o arquivo.");
    }
}

// Função para registrar a saída
void registrarSaida(String nomeMedico, String uid) {
    File file = SD.open("presenca.txt", FILE_WRITE);
    if (file) {
        file.println("Médico: " + nomeMedico + " | UID: " + uid + " | Saída | Data/Hora: " + getDateTime());
        file.close();
    } else {
        Serial.println("Erro ao abrir o arquivo.");
    }
}

// Função para apagar todos os registros
void apagarRegistros() {
    if (SD.remove("presenca.txt")) {
        Serial.println("Registros apagados com sucesso.");
    } else {
        Serial.println("Erro ao apagar registros.");
    }
}

// Função para obter a data e hora (exemplo fixo)
String getDateTime() {
    // Implemente a lógica para obter a data e hora atual (usando RTC ou outra fonte)
    return "2024-12-02 12:00"; // Exemplo de data/hora fixa
}
