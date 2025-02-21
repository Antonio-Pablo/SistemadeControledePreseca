#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>

#define SS_RFID 10  // Pino CS do módulo RFID
#define RST_RFID 9  // Pino RST do módulo RFID
#define SS_SD 4     // Pino CS do módulo SD
#define led 13
MFRC522 mfrc522(SS_RFID, RST_RFID);
File arquivo;

// Definição da data e hora manual
String data_hora = "21/02/2025 11:05";  // Ajuste conforme necessário

void setup() {
    Serial.begin(9600);
    SPI.begin();

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
        medico_nome = "Dr. Nattan Albuquerque";
    } else if (uid == "4de342196c80") {
        medico_nome = "Dra. Rayssa Nery";
    } else if (uid == "4fa878196c80") {
        medico_nome = "Dr. Digeorgio Martins";
    } else {
        Serial.println("Médico não cadastrado! Registro não será salvo.");
        return; // Sai da função sem salvar no SD
    }

    // Exibe o nome do médico no monitor serial
    Serial.print("Médico: ");
    Serial.println(medico_nome);

    // Salvar os dados no cartão SD
    arquivo = SD.open("presenca.txt", FILE_WRITE);
    if (arquivo) {
        arquivo.print("ID: ");
        arquivo.print(uid);
        arquivo.print(" - Médico: ");
        arquivo.print(medico_nome);
        arquivo.print(" - Data/Hora: ");
        arquivo.println(data_hora); // Usa a data e hora manual
        arquivo.close();
        Serial.println("Presença salva no SD!");
        digitalWrite(led, HIGH);
        
    } else {
        Serial.println("Erro ao salvar no SD!");
    }

    delay(2000); // Pequeno delay para evitar leituras repetidas
}
