#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>

#define SS_RFID 10  // Pino CS do módulo RFID
#define RST_RFID 9  // Pino RST do módulo RFID
#define SS_SD 4     // Pino CS do módulo SD
#define BUZZER 7    // Pino do buzzer

MFRC522 mfrc522(SS_RFID, RST_RFID);
File arquivo;

// Definição da data e hora manual
String data_hora = "21/02/2025 08:30";  // Ajuste conforme necessário

void setup() {
    Serial.begin(9600);
    SPI.begin();
    pinMode(BUZZER, OUTPUT);

    // Inicializa o módulo RFID
    mfrc522.PCD_Init();
    Serial.println("Aproxime o cartão RFID...");

    // Inicializa o cartão SD
    if (!SD.begin(SS_SD)) {
        Serial.println("Falha ao inicializar o cartão SD!");
        return;
    }
    Serial.println("Cartão SD pronto!");
}

void loop() {
    // Verifica se um cartão foi aproximado
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Captura o UID do cartão RFID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    
    Serial.print("Cartão detectado! UID: ");
    Serial.println(uid);

    // Relacionando UID a um médico (exemplo)
    String medico_nome = "";
    if (uid == "403d7396c80") {
        medico_nome = "Dr. Digeoérgio";
    } else if (uid == "4fa878196c80") {
        medico_nome = "Dra. Elba Laiza";
    }else if (uid == "4de342196c80") {
        medico_nome = "Dr. Antonio Pablo";
    }
    else {
        Serial.println("Médico não cadastrado! Registro não será salvo.");
        tone(BUZZER, 1000, 500); // Som de erro
        return; // Sai da função sem salvar no SD
    }

    // Exibe o nome do médico no monitor serial
    Serial.print("Médico: ");
    Serial.println(medico_nome);

    // Verifica se o médico já registrou entrada no dia
    bool ja_registrado = false;
    String ultima_linha = "";
    arquivo = SD.open("presenca.txt", FILE_READ);
    if (arquivo) {
        while (arquivo.available()) {
            ultima_linha = arquivo.readStringUntil('\n');
        }
        arquivo.close();
        if (ultima_linha.indexOf(uid) != -1 && ultima_linha.indexOf("Entrada") != -1) {
            ja_registrado = true;
        }
    }

    // Define tipo de registro
    String tipo_registro = ja_registrado ? "Saída" : "Entrada";

    // Som de confirmação
    tone(BUZZER, 2000, 300);

    // Salvar os dados no cartão SD
    arquivo = SD.open("presenca.txt", FILE_WRITE);
    if (arquivo) {
        arquivo.print("ID: ");
        arquivo.print(uid);
        arquivo.print(" - Médico: ");
        arquivo.print(medico_nome);
        arquivo.print(" - Tipo: ");
        arquivo.print(tipo_registro);
        arquivo.print(" - Data/Hora: ");
        arquivo.println(data_hora); // Usa a data e hora manual
        arquivo.close();
        Serial.println("Registro de " + tipo_registro + " salvo no SD!");
    } else {
        Serial.println("Erro ao salvar no SD!");
    }

    delay(2000); // Pequeno delay para evitar leituras repetidas
}
